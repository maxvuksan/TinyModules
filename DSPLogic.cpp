
#include "DSPLogic.h"
#include "Module.h"



void DSPLogic::refresh() {

    bool computedFound = false;
    for (int i = 0; i < outputSocketBuffers.size(); i++) {

        if(outputSocketBuffers[i].computedThisTimestep){
            computedFound = true;
        }
    }

    for (int i = 0; i < outputSocketBuffers.size(); i++) {
        outputSocketBuffers[i].computedThisTimestep = false;
    }

    // if no value was computed in any of the sockets, do not propogate refresh call
    if (!computedFound) {
        return;
    }

    auto& connections = inputSockets[0]->getAttachedWires();

    for (int i = 0; i < connections.size(); i++) {
        connections[i].otherSocket->getBlock()->getDSP()->refresh();
    }
}

void DSPLogic::setFloatParam(const std::string& name, float value) {
    parameters_float[name] = value;
}

float DSPLogic::getFloatParam(const std::string& name) const {
    auto it = parameters_float.find(name);
    return it != parameters_float.end() ? it->second : 0.0f;
}

void DSPLogic::setOutputSocketCount(int size) {
    outputSockets.resize(size);
    outputSocketBuffers.resize(size);
}

void DSPLogic::setOutputSocket(int index, WireSocket* socket) {
    outputSockets[index] = socket;
    socket->setDSPIndex(index);
}

void DSPLogic::setInputSocketCount(int size) {
    inputSockets.resize(size);
}

void DSPLogic::setInputSocket(int index, WireSocket* socket) {
    inputSockets[index] = socket;
    socket->setDSPIndex(index);
}