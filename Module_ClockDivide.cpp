#include "Module_ClockDivide.h"
#include "Globals.h"
#include "DSPUtility.h"

Module_ClockDivide::Module_ClockDivide() : Module::Module(2, "clock divide") {

    Component_CreateInputSocket("in", 0, 0);

    KnobConfiguration freqConfig;
    freqConfig.defaultValue = 120;
    freqConfig.increment = 1;
    freqConfig.min = 1;
    freqConfig.max = 999;

    auto& freqKnob = Component_CreateKnob("bpm", 1, 0, &freqConfig);

    Component_CreateOutputSocket("out", 0, 1);

    Component_CreateOutputSocket("1/2", 0, 2);
    Component_CreateOutputSocket("1/4", 0, 3);
    Component_CreateOutputSocket("1/8", 0, 4);
    Component_CreateOutputSocket("1/16", 0, 5);
    Component_CreateOutputSocket("1/32", 0, 6);

    Component_CreateOutputSocket("1/3", 1, 2);
    Component_CreateOutputSocket("1/6", 1, 3);
    Component_CreateOutputSocket("1/12", 1, 4);
    Component_CreateOutputSocket("1/24", 1, 5);
    Component_CreateOutputSocket("1/48", 1, 6);

}


void Module_ClockDivide::Reset() {
    phase = 0;
    clockCounter = 0;
    lastInputSample = 0;
    hasClockInput = false;
}

void Module_ClockDivide::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 11);
}

void Module_ClockDivide::Process() {

    const int numSamples = GetOutputBuffer(0).getNumSamples();
    const float* inPtr = GetInputReadPtr(0, 0);

    // determine if we need to use BPM knob or input signal as clock
    bool newHSI = Component_GetSocket("in")->HasConnection();

    if (newHSI != hasClockInput) {
    
        if (newHSI) {

            juce::MessageManager::callAsync([this] {
                Component_GetKnob("bpm")->SetVisuallyEnabled(false);
            });
        }
        else {
            juce::MessageManager::callAsync([this] {
                Component_GetKnob("bpm")->SetVisuallyEnabled(true);
            });
        }

        hasClockInput = newHSI;
    }


    outSocketWritePtrs.resize(divisionValues.size());
    for (int d = 0; d < divisionValues.size(); d++) {
        outSocketWritePtrs[d] = GetOutputWritePtr(d, 0); 
    }

    bool established = false;
    bool risingEdge;

    for (int i = 0; i < numSamples; i++)
    {
        // generate clock signal

        float sample;

        if (!hasClockInput) {
            
            phaseIncrement = DSP::GetPhaseIncrement(DSP::BpmToFrequency(Component_GetKnobValue("bpm")), sampleRate);
            sample = DSP::SampleWaveform(DSP::WaveType::Square, phase);

            DSP::IncrementPhase(phase, phaseIncrement);
        }
        else {
            sample = inPtr[i];
        }

        // Rising edge detection and outputting to sockets

        if (!established) {
            risingEdge = (lastInputSample <= 0.0f && sample > 0.0f);
            lastInputSample = sample;

            if (risingEdge)
            {
                ++clockCounter;
            }

            established = true;
        }

        // Output pulses on appropriate divisions
        for (int d = 0; d < divisionValues.size(); ++d)
        {
            int div = divisionValues[d];

            if (risingEdge && (clockCounter % div == 0)) {
                outSocketWritePtrs[d][i] = 1.0f;
            }
            else {
                outSocketWritePtrs[d][i] = -1.0f;
            }
        }


    }
}


