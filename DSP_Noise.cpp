
#include "DSP_Noise.h"


void DSP_Noise::prepare(double sampleRate, int blockSize) {
    // No state needed for simple white noise, but could add pink/brown/blue filters
    this->sampleRate = sampleRate;
}


void DSP_Noise::process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) {

    if (outputSocketBuffers[outputIndex].computedThisTimestep) {

        //bufferToWriteTo.makeCopyOf(outputSocketBuffers[outputIndex].audioBuffer, true);
        //return;
    }

    bufferToWriteTo.setSize(1, bufferToWriteTo.getNumSamples());

    auto* data = bufferToWriteTo.getWritePointer(0);

    for (int i = 0; i < bufferToWriteTo.getNumSamples(); ++i) {

        switch (outputIndex) {

            case 0: data[i] = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; break; // White
            case 1: data[i] = generatePinkNoise(); break;
            case 2: data[i] = generateBrownNoise(); break;
            case 3: data[i] = generateBlueNoise(); break;
        }
    }


    outputSocketBuffers[outputIndex].audioBuffer.makeCopyOf(bufferToWriteTo, true);
    outputSocketBuffers[outputIndex].computedThisTimestep = true;
}

float DSP_Noise::generatePinkNoise() {
    
    float white = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    pink_b0 = 0.99886f * pink_b0 + white * 0.0555179f;
    pink_b1 = 0.99332f * pink_b1 + white * 0.0750759f;
    pink_b2 = 0.96900f * pink_b2 + white * 0.1538520f;
    pink_b3 = 0.86650f * pink_b3 + white * 0.3104856f;
    pink_b4 = 0.55000f * pink_b4 + white * 0.5329522f;
    pink_b5 = -0.7616f * pink_b5 - white * 0.0168980f;
    
    float pink = pink_b0 + pink_b1 + pink_b2 + pink_b3 + pink_b4 + pink_b5 + pink_b6 + white * 0.5362f;
    pink_b6 = white * 0.115926f;

    return pink * 0.1f;
}

float DSP_Noise::generateBrownNoise() {
    float white = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    brown_lastOutput += white * 0.02f;
    brown_lastOutput = juce::jlimit(-1.0f, 1.0f, brown_lastOutput);

    return brown_lastOutput;
}

float DSP_Noise::generateBlueNoise() {
    float white = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    float blue = white - blue_lastWhite;
    blue_lastWhite = white;

    return blue * 0.5f;
}