#pragma once
#include "Module.h"
#include "EnvelopeVisual.h"
#include <JuceHeader.h>

class Module_ADSR : public Module {

	public:

		Module_ADSR();

		void Reset() override;
		void Prepare(double sampleRate, int blockSize) override;
		void Process();

	private:

		juce::ADSR adsr; // envelope
		juce::ADSR::Parameters adsrParams;

		EnvelopeVisual visual;

		bool previousGateHigh = false;
};