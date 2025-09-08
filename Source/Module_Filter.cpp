#include "Module_Filter.h"
#include "Globals.h"

Module_Filter::Module_Filter() : Module::Module(3, "filter") {

    KnobConfiguration freqConfig;
    freqConfig.defaultValue = 261.63;
    freqConfig.increment = 0.01;
    freqConfig.min = 10;
    freqConfig.max = 20000; 
    freqConfig.skewAroundDefault = true;

    addAndMakeVisible(visual);
    SetComponentBounds(visual, 0, 1, 3, 2);

    auto& freqKnob = Component_CreateKnob("freq", 0, 3, &freqConfig);

    KnobConfiguration resConfig;
    resConfig.min = 0.0f;        
    resConfig.max = 1.0f;       
    resConfig.defaultValue = 0.0f; 
    resConfig.increment = 0.01f; 

    auto& resKnob = Component_CreateKnob("res", 1, 3, &resConfig);

    Component_CreateInputSocket("in", 0, 0);

    Component_CreateOutputSocket("out", 2, 0);
}


void Module_Filter::Prepare(double sampleRate, int blockSize) {
    Module::Prepare(sampleRate, blockSize, 1, 1);

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1;

    juce::MessageManager::callAsync([this] {
        visual.SetFilterControls(Component_GetKnobValue("freq"), Component_GetKnobValue("res"), false);
     });

    ladderFilter.prepare(spec);
    ladderFilter.reset();
}

void Module_Filter::Process() {

    /*
        TO DO: The sampled freq and res values should be called in a loop over each sample, 
                this is because we need to provide the sample index to the knob (to get the correct knob modulation value)

                because of this we may need to roll our own filter solution
    */

    float cutoffFreq = Component_GetKnobValue("freq");
    float resonance = Component_GetKnobValue("res");

    ladderFilter.setCutoffFrequencyHz(cutoffFreq);
    ladderFilter.setResonance(resonance);
    ladderFilter.setMode(juce::dsp::LadderFilterMode::LPF24);

    const float* in = GetInputReadPtr(0, 0); 
    float * out = GetOutputWritePtr(0, 0);

    // Copy input to output (since LadderFilter processes in-place)
    GetOutputBuffer(0).copyFrom(0, 0, GetInputBuffer(0), 0, 0, GetInputBuffer(0).getNumSamples());

    // Wrap outputBuffer in a JUCE AudioBlock
    juce::dsp::AudioBlock<float> block(GetOutputBuffer(0));
    juce::dsp::AudioBlock<float> monoBlock = block.getSubsetChannelBlock(0, 1);
    juce::dsp::ProcessContextReplacing<float> context(monoBlock);

    // Process the signal
    ladderFilter.process(context);
}
