#pragma once
#include "DSPLogic.h"

class DSP_Filter : public DSPLogic {

	public:

		void prepare(double sampleRate, int blockSize) override;
		void process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) override;

	private:
		juce::dsp::StateVariableTPTFilter<float> lowpassFilter;

};