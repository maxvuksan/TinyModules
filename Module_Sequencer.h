#pragma once
#include "Module.h"
#include "SwitchGrid.h"

class Module_Sequencer : public Module {

	public:
		Module_Sequencer();

		void Reset();
		void Prepare(double sampleRate, int blockSize);
		void Process();

		void Step();
		void StepReset();
	

	private:

		bool previousClockHigh = false;
		bool previousStepResetHigh = false;

		std::unique_ptr<SwitchGrid> switchGrid[4];


		int selectedX = 0;
		int selectedY = 0;
		int previousSelectedY = 0;

		float vOctValues[8][4];

		int stepIndex[4];
};