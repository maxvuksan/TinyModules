#include "Module_Mixer.h"
#include "Globals.h"
#include "DSPUtility.h"

Module_Mixer::Module_Mixer() : Module::Module(5, "mixer") {

    Component_CreateInputSocket("#in 1", 0, 0);
    Component_CreateInputSocket("#in 2", 0, 1);
    Component_CreateInputSocket("#in 3", 0, 2);
    Component_CreateInputSocket("#in 4", 0, 3);
    Component_CreateInputSocket("#in 5", 0, 4);
    Component_CreateInputSocket("#in 6", 0, 5);
    Component_CreateInputSocket("#in 7", 0, 6);

    KnobConfiguration lvlConfig;
    lvlConfig.defaultValue = 1;
    lvlConfig.increment = 0.01;
    lvlConfig.min = 0;
    lvlConfig.max = 2;

    Component_CreateKnob("#lvl 1", 1, 0, &lvlConfig);
    Component_CreateKnob("#lvl 2", 1, 1, &lvlConfig);
    Component_CreateKnob("#lvl 3", 1, 2, &lvlConfig);
    Component_CreateKnob("#lvl 4", 1, 3, &lvlConfig);
    Component_CreateKnob("#lvl 5", 1, 4, &lvlConfig);
    Component_CreateKnob("#lvl 6", 1, 5, &lvlConfig);
    Component_CreateKnob("#lvl 7", 1, 6, &lvlConfig);

    Component_CreateSwitch("#on 1", 2, 0, "on", true);
    Component_CreateSwitch("#on 2", 2, 1, "on", true);
    Component_CreateSwitch("#on 3", 2, 2, "on", true);
    Component_CreateSwitch("#on 4", 2, 3, "on", true);
    Component_CreateSwitch("#on 5", 2, 4, "on", true);
    Component_CreateSwitch("#on 6", 2, 5, "on", true);
    Component_CreateSwitch("#on 7", 2, 6, "on", true);

    Component_CreateSwitch("#solo 1", 3, 0, "solo", false);
    Component_CreateSwitch("#solo 2", 3, 1, "solo", false);
    Component_CreateSwitch("#solo 3", 3, 2, "solo", false);
    Component_CreateSwitch("#solo 4", 3, 3, "solo", false);
    Component_CreateSwitch("#solo 5", 3, 4, "solo", false);
    Component_CreateSwitch("#solo 6", 3, 5, "solo", false);
    Component_CreateSwitch("#solo 7", 3, 6, "solo", false);

    Component_CreateOutputSocket("out", 4, 0);

    soloChannel = -1;
}

void Module_Mixer::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 7, 7);
}

void Module_Mixer::UnSoloAll() {

    for (int i = 0; i < 7; i++) {
        Switch* _switch = Component_GetSwitch("#solo " + std::to_string(i + 1));
        _switch->SetState(false);
    }
}

void Module_Mixer::Process() {

    float* out = GetOutputWritePtr(0);
    int numSamples = GetOutputBuffer(0).getNumSamples();

    // Clear output buffer
    juce::FloatVectorOperations::clear(out, numSamples);

    // Determine if any channel is soloed
    bool anySolo = false;
    for (int i = 0; i < 7; ++i) {

        Switch* _switch = Component_GetSwitch("#solo " + std::to_string(i + 1));

        if (_switch->GetState()) {
            
            if (_switch->GetStateRecentlyEstablished()) {

                juce::MessageManager::callAsync([this, _switch]() {

                    this->UnSoloAll();
                    _switch->SetState(true);
                    _switch->SetStateRecentlyEstablished(false);
                });

                break;
            }

            anySolo = true;
        }
    }

    // Process each channel
    for (int i = 0; i < 7; ++i)
    {
        const std::string idx = std::to_string(i + 1);

        const bool hasInput = Component_GetSocket("#in " + idx)->HasConnection();
        const bool isSolo = Component_GetSwitch("#solo " + idx)->GetState();
        const bool isOn = Component_GetSwitch("#on " + idx)->GetState();

        const bool shouldPlay = hasInput && (!anySolo || isSolo) && isOn;

        // Target gain: 1 if playing, 0 if muted
        const float targetGain = shouldPlay ? 1.0f : 0.0f;


        // If no input, skip buffer read
        if (!hasInput)
            continue;

        float knobGain = Component_GetKnobValue("#lvl " + idx);
        const float* in = GetInputReadPtr(i);

        for (int s = 0; s < numSamples; ++s) {

            // Smooth gain ramping

            gainRamps[i] = DSP::Lerp(gainRamps[i], targetGain, 0.005f); // lower = slower fade
            out[s] += in[s] * knobGain * gainRamps[i];
        }
    }


}