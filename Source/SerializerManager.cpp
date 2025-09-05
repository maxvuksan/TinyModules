#include "SerializerManager.h"
#include <fstream>

juce::var SerializerManager::SerializePatch(const RackView& rack) {

	juce::DynamicObject::Ptr root = new juce::DynamicObject();

    // save modules ------------------------------------------------------------------

    const std::vector<Module*>& allModules = RackView::processingManager.GetAllModules();
    juce::Array<juce::var> modulesArray;
    std::unordered_map<Module*, int> moduleToIndex;     // map the module addresses to indicies (so connections know what modules are routed to what)
    
    int i = 0;
    for (auto* module : allModules) { // modules are serialized in the order they are in 

        modulesArray.add(module->Serialize());
        moduleToIndex[module] = i;
        i++;
    }


    root->setProperty("modules", modulesArray);

    // save connections --------------------------------------------------------------

    juce::Array<juce::var> connectionsArray;

    
    for (auto& connection : RackView::processingManager.GetAllConnections()) {

        juce::DynamicObject::Ptr obj = new juce::DynamicObject();

        // Find and store module indices
        obj->setProperty("outModuleIndex", moduleToIndex[connection.outModule]);
        obj->setProperty("outSocketIndex", connection.outSocketIndex);
        obj->setProperty("inModuleIndex", moduleToIndex[connection.inModule]);
        obj->setProperty("inSocketIndex", connection.inSocketIndex);
        obj->setProperty("colI", connection.wireColourIndex);

        if (connection.connectedToKnob) {
            obj->setProperty("knobName", juce::String(connection.knobName));

            WireSocket* outSocket = allModules[moduleToIndex[connection.outModule]]->GetSocketFromDspIndex(connection.outSocketIndex, false);

            // this indirect operation is not ideal, but given the save method is rarley called, the performance is not crucial.
            obj->setProperty("knobMod", 
                allModules[moduleToIndex[connection.inModule]]->
                Component_GetKnob(connection.knobName)->
                GetModulationWheel(outSocket)->
                GetValue());
        }

        connectionsArray.add(obj.get());
    }

    root->setProperty("connections", connectionsArray);


    return root.get();
}
void SerializerManager::DeserializePatch(const juce::var& patchData, RackView& rack) {

    if (!patchData.isObject()) {
        return;
    }

    auto* obj = patchData.getDynamicObject();

    if (!obj) {
        return;
    }


    // load modules ----------------------------------------------------------------

    auto modulesVar = obj->getProperty("modules");
    if (!modulesVar.isArray()) return;

    juce::Array<juce::var> modulesArray = *modulesVar.getArray();

    std::unordered_map<int, Module*> indexToModule; // inverse of moduleToIndex when saving

    int i = 0;
    for (const auto& moduleVar : modulesArray) {

        if (!moduleVar.isObject()) {
            continue;
        }

        auto* moduleObj = moduleVar.getDynamicObject();

        if (!moduleObj) {
            continue;
        }


        // Get module type
        auto typeVar = moduleObj->getProperty("type");
    
        if (!typeVar.isInt()) {
            continue;
        }
 
        int x = static_cast<int>(moduleObj->getProperty("x"));
        int y = static_cast<int>(moduleObj->getProperty("y"));

        ModuleType type = static_cast<ModuleType>(static_cast<int>(typeVar));

        // Create module
        Module* module = rack.CreateModule(x, y, type);

        indexToModule[i] = module;

        if (module == nullptr) {
            continue;
        }

        // Deserialize state into the created module
        module->Deserialize(moduleVar);

        i++;
    }



    // load connections ------------------------------------------------------------

    auto connectionsVar = obj->getProperty("connections");
    if (!connectionsVar.isArray()) return;

    juce::Array<juce::var> connectionsArray = *connectionsVar.getArray();

    for (const auto& connectionVar : connectionsArray) {

        if (!connectionVar.isObject()) {
            continue;
        }

        auto* connectionObj = connectionVar.getDynamicObject();


        int outModuleIndex = static_cast<int>(connectionObj->getProperty("outModuleIndex"));
        int outSocketIndex = static_cast<int>(connectionObj->getProperty("outSocketIndex"));
        int inModuleIndex = static_cast<int>(connectionObj->getProperty("inModuleIndex"));
        int wireColourIndex = static_cast<int>(connectionObj->getProperty("colI"));

        int inSocketIndex = -1;
        std::string knobName = "";
        float knobModValue = 0;

        // since these parameters are not always present, do an additional check

        if (connectionObj->hasProperty("inSocketIndex")) {
            inSocketIndex = static_cast<int>(connectionObj->getProperty("inSocketIndex"));
        }
        // if there is a knob, there must be a knobMod property
        if (connectionObj->hasProperty("knobName")) {

            knobName = connectionObj->getProperty("knobName").toString().toStdString();
            knobModValue = static_cast<float>(connectionObj->getProperty("knobMod"));
        }

        Connection connection;
        connection.inModule = indexToModule[inModuleIndex];
        connection.outModule = indexToModule[outModuleIndex];
        connection.inSocketIndex = inSocketIndex;
        connection.outSocketIndex = outSocketIndex;
        connection.knobName = knobName;
        connection.wireColourIndex = wireColourIndex;
         
        WireManager::instance->LoadConnectionFromSavedData(connection);
    }

}

void SerializerManager::SaveToFile(const RackView& state, const juce::File& file) {

    auto data = SerializePatch(state);

    juce::String json = juce::JSON::toString(data, true); // pretty print = true

    if (!file.existsAsFile())
        file.create();

    file.replaceWithText(json); // replace content with new JSON string
}


void SerializerManager::LoadFromFile(const juce::File& file, RackView& rack) {

                                // validate file extension
    if (!file.existsAsFile() || file.getFileExtension() != GLOBAL_FILE_EXTENSION)
        return;

    juce::String json = file.loadFileAsString();

    juce::var parsed = juce::JSON::parse(json);

    if (parsed.isVoid() || !parsed.isObject()) {
        DBG("Failed to parse file: " + file.getFullPathName());
        return;
    }


    RackView::processingManager.SetProcessingEnabled(false); // PAUSE... 

    rack.Reset();
    DeserializePatch(parsed, rack);

    RackView::processingManager.SetProcessingEnabled(true); // PLAY... 

}


void SerializerManager::SaveToPluginState(const RackView& state, juce::MemoryBlock& dest) {

}
void SerializerManager::LoadFromPluginState(const void* data, int size, RackView& rack) {

}