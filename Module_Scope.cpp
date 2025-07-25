#include "Module_Scope.h"

Module_Scope::Module_Scope() : Module::Module(2, "scope")
{
    Component_CreateScope("Scope", 0, 0);
    Component_CreateInputSocket("In", 0, 4);

    KnobConfiguration scaleConfig;
    scaleConfig.defaultValue = 1.0f;
    scaleConfig.increment = 0.01f;
    scaleConfig.min = 0.1f;
    scaleConfig.max = 10.0f;

    Component_CreateKnob("scale", 1, 3, &scaleConfig);
}

void Module_Scope::Prepare(double sampleRate, int blockSize)
{
    Module::Prepare(sampleRate, blockSize, 1, 0);
}

void Module_Scope::Process()
{
    //const float* in = inputBuffer.getReadPointer(0);
    //const int numSamples = inputBuffer.getNumSamples();

    //juce::AudioBuffer<float> tempBuffer;
    //tempBuffer.setSize(1, numSamples, false, false, true);
    //tempBuffer.copyFrom(0, 0, in, numSamples);

    // Apply vertical gain (Height knob)
    //tempBuffer.applyGain(Component_GetKnobValue("scale"));

    // Send buffer to attached scopes
    //for (auto& [label, scopePair] : GetScopes())
    //{
    //    if (scopePair.second)
    //        scopePair.second->visualiser.pushBuffer(tempBuffer);
    //}
}
