#include "Module_LFO.h"
#include "DSP_Oscillator.h"
#include "Globals.h"

Module_LFO::Module_LFO() : Module::Module(2, "lfo") {

    KnobConfiguration freqConfig;
    freqConfig.defaultValue = 261.63;
    freqConfig.increment = 0.01;
    freqConfig.min = 0.1;
    freqConfig.max = 15;
    freqConfig.defaultValue = 1;
    freqConfig.skewAroundDefault = true;

    Component_CreateKnob("freq", 0, 0, &freqConfig);

    KnobConfiguration fmConfig;
    freqConfig.defaultValue = 0;
    freqConfig.increment = 0.001;
    freqConfig.min = -1;
    freqConfig.max = 1;

    Component_CreateKnob("fm", 0, 3, &fmConfig);

    Component_CreateInputSocket("fm", 0, 4);

    Component_CreateOutputSocket("sin", 1, 1);
    Component_CreateOutputSocket("squ", 1, 2);
    Component_CreateOutputSocket("saw", 1, 3);
    Component_CreateOutputSocket("tri", 1, 4);
}

void Module_LFO::Reset() {
    phase = 0;
}

void Module_LFO::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 4);
    phaseIncrement = DSP::GetPhaseIncrement(Component_GetKnobValue("freq"), sampleRate);
}

void Module_LFO::Process() {

    const int numSamples = GetOutputBuffer(0).getNumSamples();

    float* sineOut = GetOutputWritePtr(0, 0);
    float* squareOut = GetOutputWritePtr(1, 0);
    float* sawOut = GetOutputWritePtr(2, 0);
    float* triangleOut = GetOutputWritePtr(3, 0);

    for (int i = 0; i < numSamples; ++i)
    {
        float fm = GetInputReadPtr(0)[i];

        // Generate samples
        float sineSample = DSP::SampleWaveform(DSP::WaveType::Sine, phase);
        float squareSample = DSP::SampleWaveform(DSP::WaveType::Square, phase);
        float sawSample = DSP::SampleWaveform(DSP::WaveType::Saw, phase);
        float triangleSample = DSP::SampleWaveform(DSP::WaveType::Triangle, phase);

        // Write to output buffer
        sineOut[i] = sineSample;
        squareOut[i] = squareSample;
        sawOut[i] = sawSample;
        triangleOut[i] = triangleSample;

        DSP::IncrementPhase(phase, phaseIncrement);
    }
}
