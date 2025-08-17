#pragma once
#include <JuceHeader.h>
#include "WireComponent.h"
#include "WireSocket.h"

/*
    Is the parent to all wire visuals, additionally manages the logic for drawing out and connecting wires
*/
class WireManager : public juce::Component
{

    struct WireInteractionState {

        enum Mode { None, Creating, Moving } mode = None;

        WireSocket* sourceSocket = nullptr;
        WireComponent* currentWire = nullptr;
        std::vector<WireAttachedToSocket> movedWires;
    };

    public:

        WireManager();

        void Reset();

        /* called from the socket drawing the wire */
        void StartWireFrom(WireSocket* fromSocket);
        void MoveWireFrom(WireSocket* fromSocket);

        /*
            @param mousePos     the end position of the wire
        */
        void UpdateDragPosition(juce::Point<float> mousePos);

        /*  Note: Wires can be patched to either other sockets or knobs  */
        void FinishDragAt(WireSocket* targetSocket);
        //void FinishDragAt(Knob* knob); 

        /*
            mimics the act of patching a wire, is generally called when loading a save
        */
        void LoadConnectionFromSavedData(const Connection& connection);

        WireComponent* CreateWire();
        void RemoveWire(WireComponent* wire);

        static WireManager* instance;

    private:

        WireInteractionState interactState;

        static int currentWireColourIndex;

        std::vector<std::unique_ptr<WireComponent>> wires;
        std::vector<std::unique_ptr<WireComponent>> vacantWires;
};
