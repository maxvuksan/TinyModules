#pragma once
#include "Module.h"

class Module_Output : public Module {

	public:
		Module_Output();

		void Prepare(double sampleRate, int blockSize);
		void Process();

	private:
};