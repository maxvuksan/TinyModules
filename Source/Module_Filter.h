#pragma once
#include "Module.h"
#include "EnvelopeVisual.h"
#include <juce_dsp/juce_dsp.h>

class Module_Filter : public Module {

	public:

		Module_Filter();
		
		void Prepare(double sampleRate, int blockSize) override;
		void Process() override;

	private:

		EnvelopeVisual visual;

		juce::dsp::LadderFilter<float> ladderFilter;
		juce::dsp::ProcessSpec spec;
};