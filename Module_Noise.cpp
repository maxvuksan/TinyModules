#include "Module_Noise.h"
#include "DSP_Noise.h"
#include "DSPUtility.h"

Module_Noise::Module_Noise() : Module::Module(1, "noise") {

    Component_CreateOutputSocket("whit", 0, 1);
    Component_CreateOutputSocket("pink", 0, 2);
    Component_CreateOutputSocket("brow", 0, 3);
    Component_CreateOutputSocket("blue", 0, 4);
}


void Module_Noise::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 0, 4);
}

void Module_Noise::Process() {

    float* whiteOut = GetOutputWritePtr(0);
    float* pinkOut = GetOutputWritePtr(1);
    float* brownOut = GetOutputWritePtr(2);
    float* blueOut = GetOutputWritePtr(3);

    for (int i = 0; i < GetOutputBuffer(0).getNumSamples(); ++i)
    {
        // Generate samples
        float white = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; 
        float pink = GeneratePinkNoise();
        float brown = GenerateBrownNoise();
        float blue = GenerateBlueNoise();

        // Write to output buffer
        whiteOut[i] = white;
        pinkOut[i] = pink;
        brownOut[i] = brown;
        blueOut[i] = blue;
    }

}

float Module_Noise::GeneratePinkNoise() {

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

float Module_Noise::GenerateBrownNoise() {
    float white = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    brown_lastOutput += white * 0.02f;
    brown_lastOutput = juce::jlimit(-1.0f, 1.0f, brown_lastOutput);

    return brown_lastOutput;
}

float Module_Noise::GenerateBlueNoise() {
    float white = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    float blue = white - blue_lastWhite;
    blue_lastWhite = white;

    return blue * 0.5f;
}