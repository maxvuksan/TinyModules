#pragma once
#include <JuceHeader.h>
#include "WireComponent.h"

class Module;
class WireSocket;

struct WireAttachedToSocket {
    WireComponent* wire;
    WireSocket* otherSocket = nullptr;
    ConnectionType connectionType;

    Knob* otherKnob = nullptr; // only valid if this wire is connecting to a knob NOT an input socket
};

class WireSocket : public juce::Component
{
    public:

        WireSocket(Module* _module, bool isInput, int dspIndex, ConnectionType _socketType = CONNECT_MONO);
        ~WireSocket() override = default;

        virtual void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        void AddConnectionInProcessing(const WireAttachedToSocket& connection);
        void RemoveConnectionInProcessing(const WireAttachedToSocket& connection);
        void RemoveAllConnectionsInProcessing();

        void AssignWireFromOtherSocket(WireAttachedToSocket connection);
        void RecomputeWireGraphics();

        // @returns true if this socket has > 0 connections 
        bool HasConnection() { return attachedWires.size() > 0; }

        const std::vector<WireAttachedToSocket>& GetAttachedWires() { return attachedWires; }

        /*
            Number of voices represents how many channels (voices) are being used in a polyphonic wire, for monophonic we always assume 1 channel is active
        */
        void SetNumActiveVoices(int numActiveVoices);

        Module*         GetModule()             { return block; }
        ConnectionType  GetConnectionType()     { return connectionType; }
        bool            GetIsInput()            { return isInput; }
        int             GetDSPIndex()           { return dspIndex; }
        int             GetNumActiveVoices()    { return numActiveVoices; }

    private:

        bool isInput;
        int dspIndex;
        int numActiveVoices;

        Module* block;
        ConnectionType connectionType;

        std::vector<WireAttachedToSocket> attachedWires;
        bool creatingNewWire = false;

        bool movingWire = false;
        std::vector<WireAttachedToSocket> wiresBeingMoved;

        bool isDragging = false;
        juce::Point<int> dragPosition;
};
