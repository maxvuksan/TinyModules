
#include "DSP_Output.h"
#include "Module.h"


void DSP_Output::prepare(double sampleRate, int blockSize) {
    // No state needed for simple white noise, but could add pink/brown/blue filters
    this->sampleRate = sampleRate;
}


void DSP_Output::process(int outputIndex, juce::AudioBuffer<float>& finalBuffer) {

    if (inputSockets.size() == 0) {
        DBG("DSP_Output has no input sockets assigned");
        return;
    }

    auto& connections = inputSockets[0]->getAttachedWires();

    // no input
    for (int i = 0; i < connections.size(); i++) {

        // Allocate temp buffer to store the signal
        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.setSize(finalBuffer.getNumChannels(), finalBuffer.getNumSamples(), false, false, true);
        tempBuffer.clear();

        if (connections[i].otherSocket == nullptr) {
            continue;
        }

        // Ask other DSP to generate signal
        connections[i].otherSocket->getBlock()->getDSP()->process(connections[i].otherSocket->getDSPIndex(), tempBuffer);

        // Mix tempBuffer into main output buffer
        for (int ch = 0; ch < finalBuffer.getNumChannels(); ++ch) {
         
            finalBuffer.addFrom(ch, 0, tempBuffer, ch, 0, tempBuffer.getNumSamples());
        }
    }

    // Get gain from parameter map
    float gain = getFloatParam("Gain"); 

    // Apply gain to entire buffer
    for (int ch = 0; ch < finalBuffer.getNumChannels(); ++ch) {
        finalBuffer.applyGain(ch, 0, finalBuffer.getNumSamples(), gain);
    }
}