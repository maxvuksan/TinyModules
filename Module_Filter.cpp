#include "Module_Filter.h"
#include "Globals.h"

Module_Filter::Module_Filter() : Module::Module(1, "LADDER FILTER") {

    KnobConfiguration freqConfig;
    freqConfig.defaultValue = 261.63;
    freqConfig.increment = 0.01;
    freqConfig.min = 10;
    freqConfig.max = 20000; 

    auto& freqKnob = Component_CreateKnob("Frequency", 0, 1, &freqConfig);

    KnobConfiguration resConfig;
    resConfig.min = 0.0f;        
    resConfig.max = 1.0f;       
    resConfig.defaultValue = 1.0f / sqrt(2); 
    resConfig.increment = 0.01f; 

    auto& resKnob = Component_CreateKnob("Resonance", 0, 2, &resConfig);

    Component_CreateInputSocket("In", 0, 3);

    Component_CreateOutputSocket("Out", 0, 4);
}


void Module_Filter::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 1);

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1;

    ladderFilter.prepare(spec);
    ladderFilter.reset();
}

void Module_Filter::Process() {

    float cutoffFreq = Component_GetKnobValue("Frequency");
    float resonance = Component_GetKnobValue("Resonance");

    ladderFilter.setCutoffFrequencyHz(cutoffFreq);
    ladderFilter.setResonance(resonance);
    ladderFilter.setMode(juce::dsp::LadderFilterMode::LPF24);

    const float* in = GetInputReadPtr(0, 0); 
    float * out = GetOutputWritePtr(0, 0);

    // Copy input to output (since LadderFilter processes in-place)
    GetOutputBuffer(0).copyFrom(0, 0, GetInputBuffer(0), 0, 0, GetInputBuffer(0).getNumSamples());

    // Wrap outputBuffer in a JUCE AudioBlock
    juce::dsp::AudioBlock<float> block(GetOutputBuffer(0));
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process the signal
    ladderFilter.process(context);
}
