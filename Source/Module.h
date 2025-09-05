#pragma once
#include "DSPLogic.h"
#include <JuceHeader.h>
#include "ModuleType.h"
#include "ComponentDefinitions.h"
#include "SwitchGrid.h"
#include "Knob.h"
#include "Switch.h"

/*
    Module acts as the base class for rack modules,

    ____________________
    |    moduleName     |
    |-------------------|
    |                   |
    |                   |
    |                   |
    |                   |
    |-------------------|

          widthUnits
    <------------------->

*/
class Module : public juce::Component
{
    public:


        /*
            @param widthUnits   how wide is the module (acts as a multiplier, widthUnits * GLOBAL_BLOCK_WIDTH_UNIT)
            @param moduleName   the display name of the module
        */
        Module(int widthUnits, std::string moduleName);
        ~Module(); 

        /*
            Can be used to reset the internal state of a module,
            e.g. clear running state of delay, reverb or envelope 
        */
        virtual void Reset(){}

        /*
            is called before processing for a DSP cycle begins

            internally should call Module::Prepare(sampleRate, blockSize, numInputSocket, numOutputSockets)

            @param  sampleRate  - the number of audio samples per second
            @param  blockSize   - the number of samples processed each DSP cycle
        */
        virtual void Prepare(double sampleRate, int blockSize){}
        void Prepare(double sampleRate, int blockSize, int numInputSocket, int numOutputSockets);

        /*
            processes the module for this DSP cycle
        */
        virtual void Process() {}

        /*
            is called when the module needs to be serialize/deserialize custom controls or other.
        */
        virtual juce::var SerializeCustom() { return {}; }
        virtual void DeserializeCustom(const juce::var& customData){}

        /*
            is called to serialize/deserialize knobs and other built in components
        */
        juce::var Serialize();
        void Deserialize(const juce::var& data);


        /*
            determines if we should paint this module to look selected
        */
        void SetSelected(bool state);
        /*
            the offset to apply moving moving a large selection of blocks (if this block is the top left block in the selection it would have 0,0)
        */
        void SetGridOffsetOnSelection(int x, int y);
        juce::Point<int> GetGridOffsetOnSelection();

        /*
            creates a new knob component on the block 

            @param label        - name of the control
            @param x            - x coordinate of the control (on the module grid)
            @param y            - y coordinate of the control (on the module grid)
            @param configuration- specific configuration for the control, the configuration data is copied, not kept in pointer form, otherwise nullptr
            @param width        - number of grid cells the component takes up horizontally 
            @param height       - number of grid cells the component takes up vertically 
        */
        Knob& Component_CreateKnob(const std::string& label, int x, int y, KnobConfiguration* configuration);
        /*
            @returns dspIndex   - the index to reference this socket by
        */
        int Component_CreateInputSocket(const std::string& label, int x, int y, ConnectionType socketType = CONNECT_MONO);
        int Component_CreateOutputSocket(const std::string& label, int x, int y, ConnectionType socketType = CONNECT_MONO);
        int Component_CreateSocket(const std::string& label, int x, int y, bool isInput, ConnectionType socketType = CONNECT_MONO);

        /*
            @param innerLabel       - the small text placed within the switch
            @parma defaultState     - the inital state of the switch boolean 
        */
        Switch& Component_CreateSwitch(const std::string& label, int x, int y, const std::string& innerLable, bool defaultState = false);
        ComponentScope& Component_CreateScope(const std::string& label, int x, int y);

        /*
            sets the bounds of a component on the modules component grid (position and size)

            @param  component   -   the component we are manipulating 
            @param  x           -   the x coordinate on the module grid
            @param  y           -   the y coordinate on the module grid
            @param width        -   number of grid cells the component takes up horizontally
            @param height       -   number of grid cells the component takes up vertically
        */
        void SetComponentBounds(juce::Component& component, int x, int y, int width, int height);

        /*
            @returns a reference to a component with the matching label

            @param sampleIndex  used to apply sample based modulation (if a modulation wire was connected)
        */
        double      Component_GetKnobValue(const std::string& label, int sampleIndex = 0);
        Knob*       Component_GetKnob(const std::string& label);
        WireSocket* Component_GetSocket(const std::string& label);
        Switch*     Component_GetSwitch(const std::string& label);

        /*
            @param  dspIndex the index to refer to the buffer (input and output indicies may overlap)

            @returns a reference to the audio buffer allocated for the input/output with dspIndex
        */
        juce::AudioBuffer<float>& GetOutputBuffer(int dspIndex);
        juce::AudioBuffer<float>& GetInputBuffer(int dspIndex);

        std::vector <juce::AudioBuffer<float>>& GetOutputBuffers();
        std::vector <juce::AudioBuffer<float>>& GetInputBuffers();

        /*
            @returns a pointer to a specific channel of a sockets audio buffer
        
            @param  dspIndex    the socket we are targeting 
            @param  channel     the channel of said connection (0 if mono, otherwise 0-15 for poly)
        */
        const float*    GetInputReadPtr(int dspIndex, int channel = 0);
        float*          GetInputWritePtr(int dspIndex, int channel = 0);
        const float*    GetOutputReadPtr(int dspIndex, int channel = 0);
        float*          GetOutputWritePtr(int dspIndex, int channel = 0);

        std::unordered_map<std::string, std::pair<ComponentConfig, std::unique_ptr<WireSocket>>>& GetSockets() { return componentsSockets; }
        std::unordered_map<std::string, std::pair<ComponentConfig, std::unique_ptr<Knob>>>& GetKnobs() { return componentsKnobs; }

        /*
            @param  dspIndex     index of socket (input and output indicies are independent)
            @param  isInput     are we looking for an input our output socket?
        */
        WireSocket* GetSocketFromDspIndex(int dspIndex, bool isInput);

        /*
            JUCE function overrides
        */
        void mouseDown  (const juce::MouseEvent& e) override;
        void mouseDrag  (const juce::MouseEvent& e) override;
        void mouseUp    (const juce::MouseEvent& e) override;
        void paint      (juce::Graphics& g) override;
        void paintOverChildren(juce::Graphics& g) override;
        void resized    () override;

        int GetWidthUnits() const;

        void SetRackPosition(int x, int y);
        juce::Point<int> GetRackPosition();

        void SetModuleType(ModuleType type);
        const ModuleType GetModuleType();

        std::string GetModuleName();

        /*
            given a string, removes all the content after the first '#' character. e.g.
            "gain#4" would become "gain" when displayed. This is good to prevent naming conflicts 
        */
        std::string SanitizeLabel(const std::string& label);

    protected:

        bool selected;
        int gridOffsetOnSelectionX;
        int gridOffsetOnSelectionY;

        double sampleRate;

        /*
            each socket on the module gets a buffer, 
            where if the connected wire is mono the buffer has 1 channel,
            otherwise if polyphonic 16 channels are allocated
        */
        std::vector<juce::AudioBuffer<float>> outputBuffers;
        std::vector<juce::AudioBuffer<float>> inputBuffers;

        /*
            used to incrementally assign dsp indicies to sockets as they are created
        */
        int inputDSPIndexIncrement;
        int outputDSPIndexIncrement;

        std::unordered_map<std::string, std::pair<ComponentConfig, std::unique_ptr<Knob>>> componentsKnobs;
        std::unordered_map<std::string, std::pair<ComponentConfig, std::unique_ptr<WireSocket>>> componentsSockets;
        std::unordered_map<std::string, std::pair<ComponentConfig, std::unique_ptr<Switch>>> componentsSwitches;
        std::unordered_map<std::string, std::pair<ComponentConfig, std::unique_ptr<ComponentScope>>> componentsScopes;

    private:

        std::string moduleName;
        ModuleType moduleType;

        int rackXPosition;
        int rackYPosition;
        int widthUnits;

        juce::Point<int> newRackPosition;
        juce::Point<int> mouseMovingBlockStartPos;
};
