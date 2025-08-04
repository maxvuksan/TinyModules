#pragma once
#include "Module.h"
#include "PianoSwitch.h"

class Module_VoltQuantizer : public Module {

	public:
		Module_VoltQuantizer();

		juce::var SerializeCustom() override;
		void DeserializeCustom(const juce::var&) override;

		void Reset();
		void Prepare(double sampleRate, int blockSize);
		void Process();

	private:

		std::unique_ptr<PianoSwitch> pianoSwitch;
		int previousBestMatch;
};