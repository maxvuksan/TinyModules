#include "Module_ADSR.h"
#include "Globals.h"

Module_ADSR::Module_ADSR() : Module::Module(1, "adsr") {

    KnobConfiguration knobConfig;
    knobConfig.defaultValue = 0.2;
    knobConfig.increment = 0.001;
    knobConfig.min = 0.001;
    knobConfig.max = 8.0;
    knobConfig.skewAroundDefault = true;

    auto& attKob = Component_CreateKnob("attack", 0, 0, &knobConfig);
    auto& decKob = Component_CreateKnob("decay", 0, 1, &knobConfig);
    auto& susKob = Component_CreateKnob("sustain", 0, 2, &knobConfig);
    auto& relKnob = Component_CreateKnob("release", 0, 3, &knobConfig);

    Component_CreateInputSocket("trig", 0, 4);
    Component_CreateOutputSocket("out", 0, 5);
}


void Module_ADSR::Reset() {
    adsr.reset();
}

void Module_ADSR::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 1);


    adsrParams.attack = Component_GetKnobValue("attack");
    adsrParams.decay = Component_GetKnobValue("decay");
    adsrParams.sustain = Component_GetKnobValue("sustain");
    adsrParams.release = Component_GetKnobValue("release");

    adsr.setParameters(adsrParams);
    adsr.setSampleRate(sampleRate);
}

void Module_ADSR::Process() {


    float* writePtr = GetOutputWritePtr(0, 0);
    const float* gateBuffer = GetInputReadPtr(0, 0);

    for (int i = 0; i < GetOutputBuffer(0).getNumSamples(); ++i)
    {
        float gate = gateBuffer[i];
        bool currentGateHigh = gate > 0.5f;

        // Trigger on rising edge
        if (!previousGateHigh && currentGateHigh) {
            adsr.noteOn();
        }
        else if (previousGateHigh && !currentGateHigh) {
            adsr.noteOff();
        }

        previousGateHigh = currentGateHigh;

        writePtr[i] = adsr.getNextSample();
    }

}
