#pragma once
#include <JuceHeader.h>
#include "WireComponent.h"

class Module;
class WireSocket;

struct WireAttachedToSocket {
    WireComponent* wire;
    WireSocket* otherSocket = nullptr;
    ConnectionType connectionType;
};

class WireSocket : public juce::Component
{
    public:

        WireSocket(Module* _module, bool isInput, int dspIndex, ConnectionType _socketType = CONNECT_MONO);
        ~WireSocket() override = default;

        // Callbacks for connection events
        std::function<void(WireSocket*)> onStartConnectionDrag;
        std::function<void(WireSocket*)> onEndConnectionDrag;

        virtual void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        void AddConnectionInProcessing(const WireAttachedToSocket& connection);
        void RemoveConnectionInProcessing(const WireAttachedToSocket& connection);

        void assignWireFromOtherSocket(WireAttachedToSocket connection);
        void computeDspIndexForWire(WireAttachedToSocket& connection);

        void recomputeWireGraphics();

        // Position of socket center, for drawing wires
        juce::Point<int> getConnectionPoint() const;

        // @returns true if this socket has > 0 connections
        bool HasConnection() { return attachedWires.size() > 0; }

        const std::vector<WireAttachedToSocket>& GetAttachedWires() { return attachedWires; }

        Module*         GetModule()         { return block; }
        ConnectionType  GetConnectionType() { return connectionType; }
        bool            GetIsInput()        { return this->isInput; }
        int             GetDSPIndex()       { return dspIndex; }

    private:

        bool isInput;
        int dspIndex;
        Module* block;
        ConnectionType connectionType;

        std::vector<WireAttachedToSocket> attachedWires;
        bool creatingNewWire = false;

        bool movingWire = false;
        std::vector<WireAttachedToSocket> wiresBeingMoved;

        bool isDragging = false;
        juce::Point<int> dragPosition;
};
