
#include "DSP_Filter.h"
#include "Module.h"
#include "DSPUtility.h"

void DSP_Filter::prepare(double sampleRate, int blockSize) {
    // No state needed for simple white noise, but could add pink/brown/blue filters
    this->sampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 1; // or 2 for stereo

    lowpassFilter.prepare(spec);
    lowpassFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}


void DSP_Filter::process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) {

    if (inputSockets.size() <= 0 || !inputSockets[0]) {
        return;
    }

    auto& connections = inputSockets[0]->getAttachedWires();
    juce::AudioBuffer<float> summedBuffer;

    DSPUtility::SumInputBuffers(connections, bufferToWriteTo.getNumSamples(), summedBuffer);

    juce::dsp::AudioBlock<float> block(summedBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    lowpassFilter.setCutoffFrequency(getFloatParam("Frequency")); // Hz
    lowpassFilter.setResonance(0.707f);        // 1/sqrt(2) = standard 12dB/oct filter
    lowpassFilter.process(context);

    bufferToWriteTo.makeCopyOf(summedBuffer, true);
}
