#pragma once
#include "Module.h"
#include "WaveformVisual.h"

struct OscillatorStructure {

	float phase[16];
	WaveformVisual waveformVisual;

};

class Module_Oscillator : public Module {

	public:

		Module_Oscillator();

		void Reset() override;
		void Prepare(double sampleRate, int blockSize) override;
		void Process();

		juce::var SerializeCustom() override;
		void DeserializeCustom(const juce::var& customData) override; 

	private:

		OscillatorStructure oscillators[2];
		float phaseIncrement = 0;

};