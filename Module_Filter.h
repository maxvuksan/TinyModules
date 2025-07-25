#pragma once
#include "Module.h"
#include <juce_dsp/juce_dsp.h>

class Module_Filter : public Module {

	public:

		Module_Filter();
		
		void Prepare(double sampleRate, int blockSize) override;
		void Process() override;


	private:

		juce::dsp::LadderFilter<float> ladderFilter;
		juce::dsp::ProcessSpec spec;
};