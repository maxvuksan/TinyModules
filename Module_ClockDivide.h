#pragma once
#include "Module.h"

class Module_ClockDivide : public Module {

	public:

		Module_ClockDivide();

		void Reset() override;
		void Prepare(double sampleRate, int blockSize) override;
		void Process() override;

	private:

		bool hasClockInput = false;

		float phaseIncrement = 0;
		float phase = 0;

		int clockCounter = 0;
		float lastInputSample = 0.0f;

		std::vector<float*> outSocketWritePtrs = { };
		std::vector<int> divisionValues = { 1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48 };
};