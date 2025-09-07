#include "Module_Oscillator.h"
#include "Globals.h"
#include "DSPUtility.h"

Module_Oscillator::Module_Oscillator() : Module::Module(5, "oscillator") {

    KnobConfiguration freqConfig;
    freqConfig.defaultValue = 261.63;
    freqConfig.increment = 0.01;
    freqConfig.min = 20;
    freqConfig.max = 16000;
    freqConfig.skewAroundDefault = true;

    for (int osc = 0; osc < 2; osc++) {
        addAndMakeVisible(oscillators[osc].waveformVisual);
        SetComponentBounds(oscillators[osc].waveformVisual, 1, 1 + osc * 3, 3, 2);
        oscillators[osc].waveformVisual.SetWaveType(DSP::WaveType::Sine);
    }

    auto& freqKnob = Component_CreateKnob("freq#0", 3, 0, &freqConfig);
    auto& freqKnob2 = Component_CreateKnob("freq#1", 3, 3, &freqConfig);
    Component_CreateInputSocket("v/oct", 0, 0);

    KnobConfiguration gainConfig;
    freqConfig.defaultValue = 1;
    freqConfig.increment = 0.01;
    freqConfig.min = 0;
    freqConfig.max = 2;
    auto& gainKnob = Component_CreateKnob("gain#0", 1, 0, &gainConfig);
    auto& gainKnob2 = Component_CreateKnob("gain#1", 1, 3, &gainConfig);

    KnobConfiguration octConfig;
    octConfig.defaultValue = 0;
    octConfig.min = -4;
    octConfig.max = 4;
    octConfig.increment = 1;
    auto& octKnob = Component_CreateKnob("oct#0", 2, 0, &octConfig);
    auto& octKnob2 = Component_CreateKnob("oct#1", 2, 3, &octConfig);

    Component_CreateOutputSocket("out", 4, 0);
}

void Module_Oscillator::Reset() {
    // set each channel to 0
    for (int osc = 0; osc < 2; osc++) {
        for (int i = 0; i < 16; i++) {
            oscillators[osc].phase[i] = 0;
        }
    }
}

void Module_Oscillator::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 1);
}

void Module_Oscillator::Process() {

    const int numSamples = GetOutputBuffer(0).getNumSamples();

    float* outWritPtr      = GetOutputWritePtr(0);

    // TODO: num of active voices need to be assigned to the input socket from out socket 
    /*
        when we have multiple cables, we take the maximum number of channels, and sum the contents of each channel
    */
    int numActiveVoices = Component_GetSocket("v/oct")->GetNumActiveVoices();

    for (int osc = 0; osc < 2; osc++) {


        for (int voice = 0; voice < numActiveVoices; voice++) {

            float vOct = 0;
            if (Component_GetSocket("v/oct")->HasConnection()) {

                const float* vOctBuffer = GetInputReadPtr(0, voice); // V/OCT input
                vOct = vOctBuffer[0];
            }

            for (int i = 0; i < numSamples; ++i)
            {
                float knobFreq = Component_GetKnobValue("freq#" + std::to_string(osc), i); // Hz
                float knobGain = Component_GetKnobValue("gain#" + std::to_string(osc), i); // 0-2
                float knobOct = Component_GetKnobValue("oct#" + std::to_string(osc), i);

                float knobVolts = DSP::FrequencyToVoltage(knobFreq) + knobOct;

                float totalVolts = knobVolts + vOct;
                float freq = DSP::VoltageToFrequency(totalVolts);
                phaseIncrement = DSP::GetPhaseIncrement(freq, sampleRate);



                // Generate samples
                float sample = DSP::SampleWaveform(oscillators[osc].waveformVisual.GetWaveType(), oscillators[osc].phase[voice]);

                // Write to output buffer 
                outWritPtr[i] += sample * knobGain;

                DSP::IncrementPhase(oscillators[osc].phase[voice], phaseIncrement);
            }
        }
    }

}


juce::var Module_Oscillator::SerializeCustom() {

    juce::DynamicObject* rootObj = new juce::DynamicObject();

    rootObj->setProperty("osc_waveform1", oscillators[0].waveformVisual.GetWaveType());
    rootObj->setProperty("osc_waveform2", oscillators[1].waveformVisual.GetWaveType());

    return rootObj;
}

void Module_Oscillator::DeserializeCustom(const juce::var& data) {

    if (auto* obj = data.getDynamicObject()) {

        DSP::WaveType wave1 = (DSP::WaveType)static_cast<int>(obj->getProperty("osc_waveform1"));
        DSP::WaveType wave2 = (DSP::WaveType)static_cast<int>(obj->getProperty("osc_waveform2"));

        oscillators[0].waveformVisual.SetWaveType(wave1);
        oscillators[1].waveformVisual.SetWaveType(wave2);
    }
}
