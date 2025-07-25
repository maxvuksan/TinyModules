#pragma once
#include "Module.h"

class Module_Reverb : public Module {

	public:
		Module_Reverb();

		void Reset() override;
		void Prepare(double sampleRate, int blockSize) override;
		void Process() override;

	private:
		juce::Reverb reverb;
		juce::Reverb::Parameters reverbParams;
};