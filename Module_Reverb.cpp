#include "Module_Reverb.h"
#include "DSPUtility.h"

Module_Reverb::Module_Reverb() : Module::Module(1, "reverb") {

    KnobConfiguration knobConfig;
    knobConfig.min = 0;
    knobConfig.max = 1;
    knobConfig.defaultValue = 0.45f;

    Component_CreateKnob("damp", 0, 0, &knobConfig);
    Component_CreateKnob("size", 0, 1, &knobConfig);

    Component_CreateKnob("mix", 0, 3, &knobConfig);

    Component_CreateInputSocket("in", 0, 4);
    Component_CreateOutputSocket("out", 0, 5);
}


void Module_Reverb::Reset() {

    reverb.reset();
}

void Module_Reverb::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 1);
}

void Module_Reverb::Process() {

    const float * in = GetInputReadPtr(0);   // mono input
    float * out = GetOutputWritePtr(0);  // mono output

    // Read knob values
    float wet = Component_GetKnobValue("mix");
    float damping = Component_GetKnobValue("damp");
    float size = Component_GetKnobValue("size");

    // Clamp if needed
    wet = juce::jlimit(0.0f, 1.0f, wet);
    damping = juce::jlimit(0.0f, 1.0f, damping);
    size = juce::jlimit(0.0f, 1.0f, size);

    // Update reverb params
    reverbParams.wetLevel = wet;
    reverbParams.dryLevel = 1.0f - wet;
    reverbParams.roomSize = size;
    reverbParams.damping = damping;
    reverbParams.width = 1.0f;     // full stereo width, can be adjustable too
    reverbParams.freezeMode = 0.0f;

    reverb.setParameters(reverbParams);

    for (int i = 0; i < GetOutputBuffer(0).getNumSamples(); ++i)
    {
        float sample = in[i];
        reverb.processMono(&sample, 1);
        out[i] = sample;
    }

}

