#pragma once
#include "Module.h"

class Module_LFO : public Module {

	public:
		Module_LFO();

		void Reset() override;
		void Prepare(double sampleRate, int blockSize) override;
		void Process() override;

	private:

		float phase;
		float phaseIncrement;
};