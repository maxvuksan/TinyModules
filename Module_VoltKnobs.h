
#pragma once
#include "Module.h"

class Module_VoltKnobs : public Module {

	public:
		Module_VoltKnobs();

		void Prepare(double sampleRate, int blockSize);
		void Process();

	private:

};