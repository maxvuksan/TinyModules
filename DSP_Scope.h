#pragma once
#include "DSPLogic.h"

class DSP_Scope : public DSPLogic {

public:

	void prepare(double sampleRate, int blockSize) override;
	void process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) override;

};