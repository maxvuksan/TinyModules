
#include "DSP_Scope.h"
#include "Module.h"

void DSP_Scope::prepare(double sampleRate, int blockSize) {
    // No state needed for simple white noise, but could add pink/brown/blue filters
    this->sampleRate = sampleRate;
}


void DSP_Scope::process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) {

    if (inputSockets.empty() || inputSockets[0] == nullptr)
        return;

    
    auto& connections = inputSockets[0]->getAttachedWires();

    if (connections.empty())
        return;


    const int numSamples = 128;
    
    juce::AudioBuffer<float> summedBuffer;
    
    summedBuffer.setSize(1, numSamples);
    summedBuffer.clear();

    for (auto& conn : connections) {
        if (!conn.otherSocket)
            continue;

        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.setSize(1, numSamples);
        tempBuffer.clear();

        conn.otherSocket->getBlock()->getDSP()->process(conn.otherSocket->getDSPIndex(), tempBuffer);
        tempBuffer.applyGain(getFloatParam("YScale"));

        // Sum into main buffer
        summedBuffer.addFrom(0, 0, tempBuffer, 0, 0, numSamples);
    }

    // Push summed result to all scopes on the block

    for (auto& [label, scopePair] : block->getScopes()) {
        scopePair.second->visualiser.pushBuffer(summedBuffer);
    }
}
