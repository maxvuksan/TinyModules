 #include "ProcessingManager.h"
#include "DSPUtility.h"

void ProcessingManager::AddModule(Module* module) {

    allModules.push_back(module);

    SortModules(); // re-sort when graph changes
}

void ProcessingManager::RemoveModule(Module* module) {

	allModules.erase(std::remove(allModules.begin(), allModules.end(), module), allModules.end());

	// Also remove any connections to/from this module
	connections.erase(std::remove_if(connections.begin(), connections.end(),
		[module](const Connection& c) {
			return c.inModule == module || c.outModule == module;
		}),
		connections.end());

	SortModules(); // re-sort when graph changes
}

void ProcessingManager::Reset() {

    allModules.clear();
    sortedModules.clear();
    connections.clear();
}

const std::vector<Module*>& ProcessingManager::GetAllModules() {
    return allModules;
}
const std::vector<Connection>& ProcessingManager::GetAllConnections() {
    return connections;
}


void ProcessingManager::AddConnection(ConnectionType connectionType, Module* outModule, int outSocketIndex, Module* inModule, int inSocketIndex) {

    Connection toAdd{ connectionType, outModule, outSocketIndex, inModule, inSocketIndex };

    for (int i = 0; i < connections.size(); i++) {
        
        // connection already exists
        if (connections[i] == toAdd) {
            return;
        }
    }

    connections.push_back(toAdd);

    /*
    Connection& newConnection = connections.back();

    if (connectionType == CONNECT_MONO) {
        newConnection.delayBuffer.setSize(2, 1);  // 1 channels, 1 sample
    }
    else { // polyphonic
        newConnection.delayBuffer.setSize(16, 1);  // 16 channels, 1 sample
    }

    newConnection.delayBuffer.clear();
    */
    SortModules();
}

void ProcessingManager::RemoveConnection(ConnectionType connectionType, Module* outModule, int outSocketIndex, Module* inModule, int inSocketIndex) {

	Connection toRemove{ connectionType, outModule, outSocketIndex, inModule, inSocketIndex };

	connections.erase(std::remove(connections.begin(), connections.end(), toRemove), connections.end());

	SortModules();
}

void ProcessingManager::SortModules() {

    sortedModules.clear();

    std::unordered_set<Module*> modulesInGraph;
    for (const auto& connection : connections) {
        modulesInGraph.insert(connection.outModule);
        modulesInGraph.insert(connection.inModule);
    }

    // Step 2: Build in-degree map and adjacency list
    std::unordered_map<Module*, int> inDegree;
    std::unordered_map<Module*, std::vector<Module*>> adjacency;

    // Initialize in-degree to 0
    for (auto* module : modulesInGraph)
        inDegree[module] = 0;

    // Fill in-degree and adjacency list from connections
    for (const auto& connection : connections) {
        adjacency[connection.outModule].push_back(connection.inModule);
        inDegree[connection.inModule]++;
    }

    // Step 3: Find modules with no incoming edges (no dependencies)
    std::queue<Module*> ready;
    for (auto* module : modulesInGraph) {
        if (inDegree[module] == 0) {
            ready.push(module);
        }
    }

    // Step 4: Perform topological sort
    while (!ready.empty()) {
        Module* current = ready.front();
        ready.pop();

        sortedModules.push_back(current);

        for (Module* dependent : adjacency[current]) {
            inDegree[dependent]--;
            if (inDegree[dependent] == 0)
                ready.push(dependent);
        }
    }

    // Step 5: Check for cycles (feedback loops)
    if (sortedModules.size() != modulesInGraph.size()) {
        jassertfalse; // Detected a cycle; cannot determine safe processing order
        sortedModules.clear();
    }
}


void ProcessingManager::PrintOrdering() {

    DBG("[");

    for (int i = 0; i < sortedModules.size(); i++) {

        DBG(sortedModules[i]->GetModuleName());
    }

    DBG("]\n");
}

void ProcessingManager::PrintConnections() {

    DBG("Connections:");

    for (const auto& connection : connections) {
        juce::String fromName = connection.outModule->GetModuleName();
        juce::String toName = connection.inModule->GetModuleName();

        DBG(fromName + " -> " + toName);
    }
}

void ProcessingManager::PrepareAll(double sampleRate, int blockSize) {

    for (int i = 0; i < allModules.size(); i++) {

        allModules[i]->Prepare(sampleRate, blockSize);
    }
}

void ProcessingManager::ProcessAll() {

    for (int i = 0; i < sortedModules.size(); i++) {

        // clear both input and output buffers

        for (auto& buf : sortedModules[i]->GetInputBuffers()) {
            buf.clear();
        }
        for (auto& buf : sortedModules[i]->GetOutputBuffers()) {
            buf.clear();
        }

    }


    for (int i = 0; i < sortedModules.size(); i++) {

        ProcessModule(sortedModules[i]);

    }
}

void ProcessingManager::ProcessModule(Module* _module) {

    // before processing, populate inputBuffers

    for (auto& socketEntry : _module->GetSockets()) {

        WireSocket* socket = socketEntry.second.second.get();

        // socket is always INPUT, otherSocket is OUTPUT
        if (!socket->GetIsInput()) {
            continue;
        }

        int dspIndex = socket->GetDSPIndex();
        auto& inBuffer = _module->GetInputBuffer(dspIndex);
        

        const std::vector<WireAttachedToSocket>& attachedWires = socket->GetAttachedWires();
        int maxNumActiveVoices = 1;

        // determine the max number of active voices we need
        for (const auto& connection : attachedWires) {

            WireSocket* otherSocket = connection.otherSocket;

            if (otherSocket == nullptr) {
                continue;
            }

            // determine the greatest number of voices required
            maxNumActiveVoices = std::max(maxNumActiveVoices, otherSocket->GetNumActiveVoices());
        }

        socket->SetNumActiveVoices(maxNumActiveVoices);

        /*
            iterate over all active channels, mono should be assigned one active channel so only iterate once
        */
        for (int con = 0; con < maxNumActiveVoices; con++) {



            float* writePtr = inBuffer.getWritePointer(con);


            // iterate and sum incoming wires, store summed buffer in the input buffer of the module 

            for (const auto& connection : attachedWires) {

                WireSocket* otherSocket = connection.otherSocket;

                if (otherSocket == nullptr) {
                    continue;
                }

                int otherDspIndex = otherSocket->GetDSPIndex();
                auto& otherBuffer = otherSocket->GetModule()->GetOutputBuffer(otherDspIndex);

                const float* readPtr = otherBuffer.getReadPointer(con);

                inBuffer.addFrom(con, 0, otherBuffer, con, 0, inBuffer.getNumSamples());
            }
        }
    }

    // finally, with the input buffers populated, perform processing

    _module->Process();
}


void ProcessingManager::SetProcessingEnabled(bool enabled) {
    processingEnabled.store(enabled, std::memory_order_release);
}

bool ProcessingManager::IsProcessingEnabled() const {
    return processingEnabled.load(std::memory_order_acquire);
}