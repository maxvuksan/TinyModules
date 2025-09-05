#include "WireManager.h"
#include "Globals.h"
#include "RackView.h"

int WireManager::currentWireColourIndex = 0;

WireManager* WireManager::instance = nullptr;

WireManager::WireManager() {

    setInterceptsMouseClicks(false, true);
    instance = this;
}

void WireManager::Reset() {

    // Deactivate all wires visually/logically first
    for (auto& wire : wires) {
        removeChildComponent(wire.get());
        wire->SetStartEnd({ 0,0 }, { 0,0 });
        wire->setVisible(false);
    }

    // Move all unique_ptrs from wires -> vacantWires
    vacantWires.reserve(vacantWires.size() + wires.size());
    std::move(wires.begin(), wires.end(), std::back_inserter(vacantWires));
    wires.clear();
}

void WireManager::StartWireFrom(WireSocket* fromSocket) {
    
    RackView::instance->ClearSelections();

    // is this an output socket?
    if (!fromSocket->GetIsInput()) {
        interactState.canConnectToKnobs = true;
    }
    else {
        interactState.canConnectToKnobs = false;
    }

    RackView::instance->ReactToCanConnectToKnobs(interactState.canConnectToKnobs);


    interactState.currentWire = CreateWire();
    interactState.currentWire->SetConnectionType(fromSocket->GetConnectionType());
    interactState.sourceSocket = fromSocket;
    interactState.mode = WireInteractionState::Creating;

    juce::Point<float> socketCentre = RackView::instance->getLocalPoint(interactState.sourceSocket, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
    interactState.currentWire->SetStartEnd(socketCentre, socketCentre);
}
 
void WireManager::MoveWireFrom(WireSocket* fromSocket) {

    RackView::instance->ClearSelections();

    // was the connected socket an input socket? (should be inverse check of StartWireFrom)
    if (fromSocket->GetIsInput()) {
        interactState.canConnectToKnobs = true;
    }
    else {
        interactState.canConnectToKnobs = false;
    }

    RackView::instance->ReactToCanConnectToKnobs(interactState.canConnectToKnobs);

    interactState.mode = WireInteractionState::Moving;
    interactState.sourceSocket = fromSocket;
    interactState.movedWires = fromSocket->GetAttachedWires(); // when moving, we reference the movedWires structure instead of current wire since we (could) have multiple wires

    interactState.sourceSocket->RemoveAllConnectionsInProcessing();
}

void WireManager::UpdateDragPosition(juce::Point<float> end) {

    if (interactState.mode == WireInteractionState::None) {
        return;
    }

    if (interactState.mode == WireInteractionState::Creating) {
     
        juce::Point<float> socketCentre = RackView::instance->getLocalPoint(interactState.sourceSocket, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
        interactState.currentWire->SetStartEnd(socketCentre, end);
    }

    else if (interactState.mode == WireInteractionState::Moving) {

        // update each wire being moved...  
        for (int i = 0; i < interactState.movedWires.size(); i++) {

            juce::Point<float> socketCentre;
            bool endIsKnob = false;

            if (interactState.movedWires[i].otherSocket != nullptr) {
                socketCentre = RackView::instance->getLocalPoint(interactState.movedWires[i].otherSocket, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
            }
            else if (interactState.movedWires[i].otherKnob != nullptr) {
                socketCentre = RackView::instance->getLocalPoint(interactState.movedWires[i].otherKnob, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
                endIsKnob = true;
            }

            interactState.movedWires[i].wire->SetStartEnd(end, socketCentre, endIsKnob);
        }
    }
}

void WireManager::FinishDragAt(Knob* targetKnob) {

    interactState.canConnectToKnobs = false;
    RackView::instance->ReactToCanConnectToKnobs(interactState.canConnectToKnobs);

    if (interactState.mode == WireInteractionState::None) {
        return;
    }

    if (interactState.mode == WireInteractionState::Creating) {

        interactState.mode = WireInteractionState::None;

        // connection to knobs is only allowed for outgoing signals
        if (interactState.sourceSocket->GetIsInput() || targetKnob == nullptr || interactState.sourceSocket == nullptr) {

            RemoveWire(interactState.currentWire);
            return;
        }

        WireAttachedToSocket connection;
        connection.otherSocket = nullptr;
        connection.wire = interactState.currentWire;
        connection.connectionType = interactState.sourceSocket->GetConnectionType();
        connection.otherKnob = targetKnob;

        connection.otherKnob->AddConnectedWire(interactState.sourceSocket, interactState.currentWire->GetWireColourIndex());
        interactState.sourceSocket->AddConnectionInProcessing(connection);
    }
    if (interactState.mode == WireInteractionState::Moving) {

        interactState.mode = WireInteractionState::None;

        // remove wires which cannot be attached to a knob
        for (int i = 0; i < interactState.movedWires.size(); i++) {

            // we connection must meet these conditions
            if (interactState.movedWires[i].otherKnob == nullptr && 
            (interactState.movedWires[i].otherSocket != nullptr && 
            interactState.movedWires[i].otherSocket->GetIsInput())) {
                

                WireAttachedToSocket connection;
                connection.otherSocket = nullptr;
                connection.wire = interactState.currentWire;
                connection.connectionType = interactState.sourceSocket->GetConnectionType();
                connection.otherKnob = targetKnob;

                connection.otherKnob->AddConnectedWire(interactState.movedWires[i].otherSocket, interactState.currentWire->GetWireColourIndex());
                interactState.movedWires[i].otherSocket->AddConnectionInProcessing(connection);

            }
            else { // we remove the wire

                WireAttachedToSocket connection;
                connection.otherSocket = interactState.movedWires[i].otherSocket;

                interactState.sourceSocket->RemoveConnectionInProcessing(connection);

                connection.otherSocket = interactState.sourceSocket;

                if (interactState.movedWires[i].otherSocket != nullptr) {
                    interactState.movedWires[i].otherSocket->RemoveConnectionInProcessing(connection);
                }
                else if (interactState.movedWires[i].otherKnob != nullptr) {
                    interactState.movedWires[i].otherKnob->RemoveConnectedWire(interactState.sourceSocket);
                }

                RemoveWire(interactState.movedWires[i].wire);
            }
        }

    }

}

void WireManager::FinishDragAt(WireSocket* targetSocket) {

    /*
        TO DO:      When any connection is made or moved (to either a knob or socket) 
                    we should ensure that the connection does not already exist (we do not want stacked wires / duplicate connections)
                    if we find a duplicate, this should be removed
    */



    interactState.canConnectToKnobs = false;
    RackView::instance->ReactToCanConnectToKnobs(interactState.canConnectToKnobs);

    if (interactState.mode == WireInteractionState::None) {
        return;
    }

    if (interactState.mode == WireInteractionState::Creating) {

        interactState.mode = WireInteractionState::None;

                                        // socket In/Out must differ
        if (targetSocket == nullptr || targetSocket->GetIsInput() == interactState.sourceSocket->GetIsInput()) {

            RemoveWire(interactState.currentWire);
            return;
        }


        WireAttachedToSocket connection;
        connection.otherSocket = targetSocket;
        connection.wire = interactState.currentWire;
        connection.connectionType = interactState.sourceSocket->GetConnectionType();
        

        interactState.sourceSocket->AddConnectionInProcessing(connection);

        // flip for targetSocket
        connection.otherSocket = interactState.sourceSocket;

        targetSocket->AssignWireFromOtherSocket(connection);
    }

    else if (interactState.mode == WireInteractionState::Moving) {

        interactState.mode = WireInteractionState::None;

        if (targetSocket == nullptr || targetSocket->GetIsInput() != interactState.sourceSocket->GetIsInput()) {

            for (int i = 0; i < interactState.movedWires.size(); i++) {
            
                WireAttachedToSocket connection;
                connection.otherSocket = interactState.movedWires[i].otherSocket;

                interactState.sourceSocket->RemoveConnectionInProcessing(connection);

                connection.otherSocket = interactState.sourceSocket;

                if (interactState.movedWires[i].otherSocket != nullptr) {
                    interactState.movedWires[i].otherSocket->RemoveConnectionInProcessing(connection);
                }
                else if (interactState.movedWires[i].otherKnob != nullptr) {
                    interactState.movedWires[i].otherKnob->RemoveConnectedWire(interactState.sourceSocket);
                }


                RemoveWire(interactState.movedWires[i].wire);
            }

            return;
        }

        // for each, remove attached wire, THEN re add with the updated connection
        for (int i = 0; i < interactState.movedWires.size(); i++) {

            WireAttachedToSocket connection;
            connection.otherSocket = interactState.sourceSocket;

            if (interactState.movedWires[i].otherSocket != nullptr) {
                interactState.movedWires[i].otherSocket->RemoveConnectionInProcessing(connection);
            }
            else if(interactState.movedWires[i].otherKnob != nullptr){
                interactState.movedWires[i].otherKnob->RemoveConnectedWire(interactState.sourceSocket);
            }


            if (interactState.movedWires[i].otherSocket != nullptr) {
                connection.otherSocket = targetSocket;
                connection.wire = interactState.movedWires[i].wire;
                connection.connectionType = interactState.movedWires[i].otherSocket->GetConnectionType();

                interactState.movedWires[i].otherSocket->AddConnectionInProcessing(connection);

                // flip for targetSocket
                connection.otherSocket = interactState.movedWires[i].otherSocket;
                targetSocket->AssignWireFromOtherSocket(connection);
            }
            else if (interactState.movedWires[i].otherKnob != nullptr) {

                connection.otherKnob = interactState.movedWires[i].otherKnob;
                connection.otherSocket = nullptr;
                connection.wire = interactState.movedWires[i].wire;
                connection.connectionType = targetSocket->GetConnectionType();

                targetSocket->AddConnectionInProcessing(connection);
                interactState.movedWires[i].otherKnob->AddConnectedWire(targetSocket, interactState.movedWires[i].wire->GetWireColourIndex());
            }

        }


    }

}

bool WireManager::GetCanConnectToKnob() {

    // we are not drawing a wire
    if (interactState.mode == WireInteractionState::None) {
        return false;
    }

    return interactState.canConnectToKnobs;
}

void WireManager::LoadConnectionFromSavedData(const Connection& connection) {

    // we are establishing the connection both visually and in processing...

    WireSocket* inSock = connection.inModule->GetSocketFromDspIndex(connection.inSocketIndex, true);
    WireSocket* outSock = connection.outModule->GetSocketFromDspIndex(connection.outSocketIndex, false);

    // ensure poly considers are restored
    ConnectionType connectionType = CONNECT_MONO;
    if (outSock->GetConnectionType() == CONNECT_POLY || (inSock != nullptr && inSock->GetConnectionType() == CONNECT_POLY)) {
        connectionType = CONNECT_POLY;
    }

    // we can make the assumption out socket will always be valid.

    WireAttachedToSocket visualConnect;
    visualConnect.connectionType = connectionType;
    visualConnect.wire = CreateWire();
    visualConnect.wire->SetWireColourIndex(connection.wireColourIndex);
    visualConnect.wire->SetConnectionType(connectionType);
    visualConnect.otherSocket = inSock;

    // connect to knob if possible
    Knob* knob = connection.inModule->Component_GetKnob(connection.knobName);

    if (knob != nullptr) {
        visualConnect.otherKnob = knob;

        knob->AddConnectedWire(outSock, connection.wireColourIndex, connection.knobModValue);
        visualConnect.otherKnob = knob;
        visualConnect.otherSocket = nullptr;
    }


    outSock->AddConnectionInProcessing(visualConnect);

    if (inSock != nullptr) {
    
        visualConnect.otherSocket = outSock;
        inSock->AssignWireFromOtherSocket(visualConnect);
    }


}



WireComponent* WireManager::CreateWire()
{
    std::unique_ptr<WireComponent> wire;

    // Reuse a wire from the vacant pool if available
    if (!vacantWires.empty()) {

        wire = std::move(vacantWires.back());
        vacantWires.pop_back();
    }
    else {
        wire = std::make_unique<WireComponent>();
    }

    wire->SetWireColourIndex(currentWireColourIndex);

    WireComponent* rawPtr = wire.get(); // Save raw pointer before transferring ownership

    wires.push_back(std::move(wire));
    // add behind other children (modulation wheels)
    addAndMakeVisible(rawPtr, 0);

    currentWireColourIndex++;
    currentWireColourIndex %= CustomLookAndFeel::GetTheme()->colour_wires.size();

    return rawPtr;
}

void WireManager::RemoveWire(WireComponent* wire) {

    for (int i = wires.size() - 1; i >= 0; --i) {

        if (wires[i].get() == wire) {

            // deactivate wire visually/logically
            removeChildComponent(wire);
            wire->SetStartEnd({ 0,0 }, { 0,0 });
            wire->setVisible(false);

            // move to vacant pool
            vacantWires.push_back(std::move(wires[i]));

            // remove from active list
            wires.erase(wires.begin() + i);
            return;
        }
    }
}

KnobModulationWheel* WireManager::CreateModulationWheel() {

    std::unique_ptr<KnobModulationWheel> wheel;

    // Reuse a wire from the vacant pool if available
    if (!vacantModulationWheels.empty()) {

        wheel = std::move(vacantModulationWheels.back());
        vacantModulationWheels.pop_back();
    }
    else {
        wheel = std::make_unique<KnobModulationWheel>();
    }


    KnobModulationWheel* rawPtr = wheel.get(); // Save raw pointer before transferring ownership
    rawPtr->SetValue(0);

    modulationWheels.push_back(std::move(wheel));

    // add infront of other children (wires)
    addAndMakeVisible(rawPtr, -1);

    return rawPtr;
}

void WireManager::RemoveModulationWheel(KnobModulationWheel* wheel) {

    for (int i = (int)modulationWheels.size() - 1; i >= 0; --i) {

        if (modulationWheels[i].get() == wheel) {

            removeChildComponent(wheel);
            wheel->setVisible(false);
            vacantModulationWheels.push_back(std::move(modulationWheels[i]));
            modulationWheels.erase(modulationWheels.begin() + i);
            return;
        }
    }
}