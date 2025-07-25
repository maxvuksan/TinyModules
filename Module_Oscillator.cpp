#include "Module_Oscillator.h"
#include "Globals.h"
#include "DSPUtility.h"

Module_Oscillator::Module_Oscillator() : Module::Module(2, "oscillator") {

    KnobConfiguration freqConfig;
    freqConfig.defaultValue = 261.63;
    freqConfig.increment = 0.01;
    freqConfig.min = 20;
    freqConfig.max = 16000;
    freqConfig.skewAroundDefault = true;

    auto& freqKnob = Component_CreateKnob("freq", 0, 0, &freqConfig);

    KnobConfiguration fmConfig;
    freqConfig.defaultValue = 0;
    freqConfig.increment = 0.001;
    freqConfig.min = -1;
    freqConfig.max = 1;

    auto& fmDepthKnob = Component_CreateKnob("fm", 0, 3, &fmConfig);


    Component_CreateInputSocket("fm", 0, 4);
    Component_CreateInputSocket("v/oct", 1, 0);

    Component_CreateOutputSocket("sin", 1, 1);
    Component_CreateOutputSocket("squ", 1, 2);
    Component_CreateOutputSocket("saw", 1, 3);
    Component_CreateOutputSocket("tri", 1, 4);

}

void Module_Oscillator::Reset() {
    phase = 0;
}

void Module_Oscillator::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 2, 4);
}

void Module_Oscillator::Process() {

    const int numSamples = GetOutputBuffer(0).getNumSamples();

    float* sineOut      = GetOutputWritePtr(0);
    float* squareOut    = GetOutputWritePtr(1);
    float* sawOut       = GetOutputWritePtr(2);
    float* triangleOut  = GetOutputWritePtr(3);

    float vOct = 0;
    if (Component_GetSocket("v/oct")->HasConnection()) {

        const float* vOctBuffer = GetInputReadPtr(1); // V/OCT input
        vOct = vOctBuffer[0];
    }

    float knobFreq = Component_GetKnobValue("freq"); // Hz
    float knobVolts = DSP::FrequencyToVoltage(knobFreq);    

    float totalVolts = knobVolts + vOct;

    for (int i = 0; i < numSamples; ++i)
    {
        float fm = GetInputReadPtr(0)[i];

        float freq = DSP::VoltageToFrequency(totalVolts) + (fm * Component_GetKnobValue("FM") * 15.0f);

        phaseIncrement = DSP::GetPhaseIncrement(freq, sampleRate);

        // Generate samples
        float sineSample        = DSP::SampleWaveform(DSP::WaveType::Sine, phase);
        float squareSample      = DSP::SampleWaveform(DSP::WaveType::Square, phase);
        float sawSample         = DSP::SampleWaveform(DSP::WaveType::Saw, phase);
        float triangleSample    = DSP::SampleWaveform(DSP::WaveType::Triangle, phase);

        // Write to output buffer
        sineOut[i]      = sineSample;
        squareOut[i]    = squareSample;
        sawOut[i]       = sawSample;
        triangleOut[i]  = triangleSample;

        DSP::IncrementPhase(phase, phaseIncrement);
    }
}


