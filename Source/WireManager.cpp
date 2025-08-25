#include "WireManager.h"
#include "Globals.h"
#include "RackView.h"

int WireManager::currentWireColourIndex = 0;

WireManager* WireManager::instance = nullptr;

WireManager::WireManager() {

    setInterceptsMouseClicks(false, false);
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

            juce::Point<float> socketCentre = RackView::instance->getLocalPoint(interactState.movedWires[i].otherSocket, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
            interactState.movedWires[i].wire->SetStartEnd(socketCentre, end);
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
        if (interactState.sourceSocket->GetIsInput()) {

            RemoveWire(interactState.currentWire);
            return;
        }


        WireAttachedToSocket connection;
        connection.otherSocket = nullptr;
        connection.wire = interactState.currentWire;
        connection.connectionType = interactState.sourceSocket->GetConnectionType();

        juce::Point<float> startSocketCentre = RackView::instance->getLocalPoint(interactState.sourceSocket, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
        juce::Point<float> endSocketCentre = RackView::instance->getLocalPoint(targetSocket, targetSocket->getLocalBounds().toFloat().getCentre());
        interactState.currentWire->SetStartEnd(startSocketCentre, endSocketCentre);

        interactState.sourceSocket->AddConnectionInProcessing(connection);

        // flip for targetSocket
        connection.otherSocket = interactState.sourceSocket;

        targetSocket->AssignWireFromOtherSocket(connection);
    }

}

void WireManager::FinishDragAt(WireSocket* targetSocket) {

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
        
        juce::Point<float> startSocketCentre = RackView::instance->getLocalPoint(interactState.sourceSocket, interactState.sourceSocket->getLocalBounds().toFloat().getCentre());
        juce::Point<float> endSocketCentre = RackView::instance->getLocalPoint(targetSocket, targetSocket->getLocalBounds().toFloat().getCentre());
        interactState.currentWire->SetStartEnd(startSocketCentre, endSocketCentre);

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

                interactState.movedWires[i].otherSocket->RemoveConnectionInProcessing(connection);

                RemoveWire(interactState.movedWires[i].wire);
            }

            return;
        }

        // for each, remove attached wire, THEN re add with the updated connection
        for (int i = 0; i < interactState.movedWires.size(); i++) {

            WireAttachedToSocket connection;
            connection.otherSocket = interactState.sourceSocket;

            interactState.movedWires[i].otherSocket->RemoveConnectionInProcessing(connection);

            connection.otherSocket = targetSocket;
            connection.wire = interactState.movedWires[i].wire;
            connection.connectionType = interactState.movedWires[i].otherSocket->GetConnectionType();

            juce::Point<float> startSocketCentre = RackView::instance->getLocalPoint(interactState.movedWires[i].otherSocket, interactState.movedWires[i].otherSocket->getLocalBounds().toFloat().getCentre());
            juce::Point<float> endSocketCentre = RackView::instance->getLocalPoint(targetSocket, targetSocket->getLocalBounds().toFloat().getCentre());

            interactState.movedWires[i].wire->SetStartEnd(startSocketCentre, endSocketCentre);

            interactState.movedWires[i].otherSocket->AddConnectionInProcessing(connection);

            // flip for targetSocket
            connection.otherSocket = interactState.movedWires[i].otherSocket;

            targetSocket->AssignWireFromOtherSocket(connection);
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
    if (inSock->GetConnectionType() == CONNECT_POLY || outSock->GetConnectionType() == CONNECT_POLY) {
        connectionType = CONNECT_POLY;
    }

    WireAttachedToSocket visualConnect;
    visualConnect.connectionType = connectionType;
    visualConnect.wire = CreateWire();
    visualConnect.wire->SetConnectionType(connectionType);
    visualConnect.otherSocket = outSock;

    inSock->AddConnectionInProcessing(visualConnect);

    visualConnect.otherSocket = inSock;
    outSock->AssignWireFromOtherSocket(visualConnect);
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
    wire->setVisible(true); // Ensure the wire is visible again

    WireComponent* rawPtr = wire.get(); // Save raw pointer before transferring ownership

    wires.push_back(std::move(wire));
    addAndMakeVisible(rawPtr);

    currentWireColourIndex++;
    currentWireColourIndex %= GLOBAL_WIRE_COLOUR_POOL.size();

    return rawPtr;
}

void WireManager::RemoveWire(WireComponent* wire) {

    for (int i = wires.size() - 1; i >= 0; --i) {

        if (wires[i].get() == wire) {

            // 1. Deactivate wire visually/logically
            removeChildComponent(wire);
            wire->SetStartEnd({ 0,0 }, { 0,0 });
            wire->setVisible(false);

            // 2. Move to vacant pool
            vacantWires.push_back(std::move(wires[i]));

            // 3. Remove from active list
            wires.erase(wires.begin() + i);
            return;
        }
    }
}
