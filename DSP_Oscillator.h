// DSP_Oscillator.h
#pragma once
#include <JuceHeader.h>
#include "DSPLogic.h"
#include "DSPUtility.h"

class DSP_Oscillator : public DSPLogic {

    public:

        void prepare(double sampleRate, int blockSize) override;
        void process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) override;

    private:

        float phase = 0.0f;
        float phaseIncrement = 0.0f;

        float generateSample(DSP::WaveType waveType);

        void updateIncrement(float frequency);
};