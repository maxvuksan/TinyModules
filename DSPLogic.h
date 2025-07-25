#pragma once
#include <JuceHeader.h>
#include "WireSocket.h"


struct DSPOutputSocketBuffer {

    bool computedThisTimestep; // each timestep, audio buffers are cleared and recomputed
    juce::AudioBuffer<float> audioBuffer;

};

class Module;

class DSPLogic {

    public:

        virtual void prepare(double sampleRate, int blockSize) = 0;
        
        // called before each time stepb 
        void refresh();
        virtual void process(int outputIndex, juce::AudioBuffer<float>& bufferToWriteTo) = 0;

        virtual ~DSPLogic() = default;

        void setFloatParam(const std::string& name, float value);
        float getFloatParam(const std::string& name) const;

        void setOutputSocketCount(int size);
        void setOutputSocket(int index, WireSocket*);
        void setInputSocketCount(int size);
        void setInputSocket(int index, WireSocket*);

        void setBlock(Module* block) { this->block = block; }

    protected:
        float sampleRate = 44100.0f;

        std::vector<WireSocket*> inputSockets;
        std::vector<WireSocket*> outputSockets; 
        std::vector<DSPOutputSocketBuffer> outputSocketBuffers;

        Module* block;

    private:
        std::unordered_map<std::string, float> parameters_float;
};
