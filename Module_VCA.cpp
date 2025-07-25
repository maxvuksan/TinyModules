#include "Module_VCA.h"
#include "Globals.h"

Module_VCA::Module_VCA() : Module::Module(1, "vca") {

    Component_CreateInputSocket("cv", 0, 1);
    Component_CreateInputSocket("in", 0, 2);

    Component_CreateOutputSocket("out", 0, 3);

}

void Module_VCA::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 2, 1);
}

void Module_VCA::Process(){

    const float* inCV = GetInputReadPtr(0); // CV input
    const float* inSig = GetInputReadPtr(1); // Signal input
    
    float* out = GetOutputWritePtr(0); // Output

    int numSamples = GetOutputBuffer(0).getNumSamples();

    for (int i = 0; i < numSamples; ++i) {
        out[i] = inSig[i] * inCV[i];
    }
}