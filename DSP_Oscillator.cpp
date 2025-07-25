#include "DSP_Oscillator.h"
#include <cmath>
#include "Module.h"


void DSP_Oscillator::prepare(double newSampleRate, int blockSize) {
    sampleRate = newSampleRate;
}


void DSP_Oscillator::updateIncrement(float frequency) {
    phaseIncrement = (2.0f * juce::MathConstants<float>::pi * frequency) / (float)sampleRate;
}


float DSP_Oscillator::generateWaveform(WaveType waveType, float phase) {

    float sample = 0.0f;
    
    switch (waveType) {

        case Sine:
            sample = std::sin(phase);
            break;
        case Square:
            sample = std::sin(phase) >= 0.0f ? 1.0f : -1.0f;
            break;
        case Saw:
            sample = (2.0f / juce::MathConstants<float>::pi) * (phase - juce::MathConstants<float>::pi);
            break;
        case Triangle:
            sample = std::asin(std::sin(phase)) * (2.0f / juce::MathConstants<float>::pi);
            break;
    
    }

    phase += phaseIncrement;

    if (phase >= juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    return sample;
}

void DSP_Oscillator::process(int outputIndex, juce::AudioBuffer<float>& buffer) {


    float fmValue = 0.0f;

    if (inputSockets.size() <= 0 || !inputSockets[0]) {
        return;
    }

    auto& connections = inputSockets[0]->getAttachedWires();

    for(int i = 0; i < connections.size(); i++){

        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.setSize(1, buffer.getNumSamples(), false, false, true);
        tempBuffer.clear();

        connections[i].otherSocket->getBlock()->getDSP()->process(connections[i].otherSocket->getDSPIndex(), tempBuffer);

        float total = 0;
        // Use average
        for (int i = 0; i < tempBuffer.getNumSamples(); ++i) {
            total += tempBuffer.getSample(0, i);
        }
        total /= tempBuffer.getNumSamples();
        fmValue += total;
    }

    updateIncrement(getFloatParam("Frequency") + (fmValue * getFloatParam("FM") * getFloatParam("Frequency")));

    buffer.setSize(1, buffer.getNumSamples());
    auto* data = buffer.getWritePointer(0);

    WaveType type = (WaveType)outputIndex;

    for (int i = 0; i < buffer.getNumSamples(); ++i) {

        data[i] = generateSample(type);
    }

    outputSocketBuffers[outputIndex].audioBuffer.makeCopyOf(buffer, true);
    outputSocketBuffers[outputIndex].computedThisTimestep = true;
}
