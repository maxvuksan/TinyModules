#pragma once
#include "Module.h"

class Module_Oscillator : public Module {

	public:

		Module_Oscillator();

		void Reset() override;
		void Prepare(double sampleRate, int blockSize) override;
		void Process();

	private:

		float phaseIncrement = 0;
		float phase = 0;

};