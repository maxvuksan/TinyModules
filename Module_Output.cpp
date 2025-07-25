#include "Module_Output.h"

Module_Output::Module_Output() : Module::Module(1, "output") {

    KnobConfiguration gainConfig;
    gainConfig.min = 0;
    gainConfig.max = 1;
    gainConfig.defaultValue = 0.5f;

    auto& gainSlider = Component_CreateKnob("gain", 0, 0, &gainConfig);
    
    Component_CreateInputSocket("in", 0, 1);
}

void Module_Output::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 1);
}

void Module_Output::Process() {

    const float* in = GetInputReadPtr(0);
    float* out = GetOutputWritePtr(0);

    float gain = Component_GetKnobValue("gain");

    for (int i = 0; i < GetInputBuffer(0).getNumSamples(); ++i) {
        out[i] = in[i] * gain;
    }

}