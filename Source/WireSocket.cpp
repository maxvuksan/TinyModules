#include "WireSocket.h"
#include "Globals.h"
#include "WireManager.h"
#include "RackView.h"
#include "CustomLookAndFeel.h"
#include "WireManager.h"

WireSocket::WireSocket(Module* _module, bool _isInput, int _dspIndex, ConnectionType _connectionType):
    block(_module), isInput(_isInput), connectionType(_connectionType), dspIndex(_dspIndex)
{
    creatingNewWire = false;
  
    SetNumActiveVoices(1);
}



void WireSocket::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();

    float squareBaseRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.275f;
    juce::Rectangle<float> squareBaseRect(
        centre.x - squareBaseRadius,
        centre.y - squareBaseRadius,
        squareBaseRadius * 2.0f,
        squareBaseRadius * 2.0f
    );

    if (RackView::instance->GetLODFactor() != LOD_CLOSE) {

        if (isInput) {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_knobPointer);
            g.drawRect(squareBaseRect, 2.0f);
        }
        else {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
            g.fillRect(squareBaseRect);
        }

        return;
    }

    if (isInput) {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_knobOutline);
        g.drawRect(squareBaseRect, 2.0f);
    }
    else {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
        g.fillRect(squareBaseRect);
    }
    
    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobPointer);

    // Outer ring
    float outerRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.15f;
    juce::Rectangle<float> outerCircle(
        centre.x - outerRadius,
        centre.y - outerRadius,
        outerRadius * 2.0f,
        outerRadius * 2.0f
    );

    g.drawEllipse(outerCircle, 2.0f); // second arg is stroke thickness

    // Inner filled circle
    float innerRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.07f;
    juce::Rectangle<float> innerCircle(
        centre.x - innerRadius,
        centre.y - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f
    );

    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobPointer);
    g.fillEllipse(innerCircle);
}


void WireSocket::mouseDown(const juce::MouseEvent& e) {
    
    if (!HasConnection() || e.mods.isCtrlDown()) {
        WireManager::instance->StartWireFrom(this);
    }
    else {
        WireManager::instance->MoveWireFrom(this);
    }
}

void WireSocket::mouseDrag(const juce::MouseEvent& e) {

    juce::Point<float> posInRack = RackView::instance->getLocalPoint(this, e.getPosition().toFloat());

    WireManager::instance->UpdateDragPosition(posInRack);
}

void WireSocket::mouseUp(const juce::MouseEvent& e) {

    auto mousePosInRack = RackView::instance->getLocalPoint(this, e.getPosition().toFloat());

    auto* comp = RackView::instance->getComponentAt(mousePosInRack.roundToInt());
     
    WireSocket* hoveredSocket = dynamic_cast<WireSocket*>(comp);
    Knob*       hoveredKnob = dynamic_cast<Knob*>(comp);

    // place wire in socket or knob, if both are null the wire is removed
    if (hoveredKnob != nullptr) {
        
        WireManager::instance->FinishDragAt(hoveredKnob);
    }
    else {
        WireManager::instance->FinishDragAt(hoveredSocket);
    }
}

void WireSocket::AddConnectionInProcessing(const WireAttachedToSocket& connection) {

    if (connection.otherSocket != nullptr) {

        attachedWires.push_back(connection);

        if (isInput) {
            RackView::processingManager.AddConnection(
                connection.connectionType,
                connection.otherSocket->GetModule(),
                connection.otherSocket->GetDSPIndex(),
                GetModule(),
                dspIndex,
                connection.wire->GetWireColourIndex());
        }
        else {
            RackView::processingManager.AddConnection(
                connection.connectionType,
                GetModule(),
                dspIndex,
                connection.otherSocket->GetModule(),
                connection.otherSocket->GetDSPIndex(),
                connection.wire->GetWireColourIndex());
        }
    }
    else if (connection.otherKnob != nullptr) {
        attachedWires.push_back(connection);

        RackView::processingManager.AddConnection(
            connection.connectionType,
            GetModule(),
            connection.otherKnob->GetModule(),
            dspIndex,
            connection.otherKnob->GetLabel(),
            connection.wire->GetWireColourIndex()
        );
    }

    RecomputeWireGraphics();
}

void WireSocket::RemoveAllConnectionsInProcessing() {

    for (int i = 0; i < attachedWires.size(); i++) {
        RemoveConnectionInProcessing(attachedWires[i]);
    }

    attachedWires.clear();
}

void WireSocket::RemoveConnectionInProcessing(WireAttachedToSocket connection) {

    for (int i = 0; i < attachedWires.size(); i++) {

        if (connection.otherSocket == attachedWires[i].otherSocket || connection.otherKnob == attachedWires[i].otherKnob) {

            attachedWires.erase(attachedWires.begin() + i);
            break;
        }

    }

    if (connection.otherSocket != nullptr) {

        if (isInput) {
            RackView::processingManager.RemoveConnection(
                connection.connectionType,
                connection.otherSocket->GetModule(),
                connection.otherSocket->GetDSPIndex(),
                GetModule(),
                dspIndex);
        }
        else {
            RackView::processingManager.RemoveConnection(
                connection.connectionType,
                GetModule(),
                dspIndex,
                connection.otherSocket->GetModule(),
                connection.otherSocket->GetDSPIndex());
        }
    }

    else if (connection.otherKnob != nullptr) {

        RackView::processingManager.RemoveConnection(
            connection.connectionType,
            GetModule(),
            connection.otherKnob->GetModule(),
            connection.otherKnob->GetLabel()
        );
    }

    RecomputeWireGraphics();
}

void WireSocket::AssignWireFromOtherSocket(WireAttachedToSocket connection) {
    
    attachedWires.push_back(connection);
}

void WireSocket::RecomputeWireGraphics() {

    for (int i = 0; i < attachedWires.size(); i++) {

        juce::Point<float> socketCentreInRackSpace = RackView::instance->getLocalPoint(this, getLocalBounds().toFloat().getCentre());
        juce::Point<float> otherPositionInRackSpace; 
        
        bool endIsKnob = false;
        // connecting to socket
        if (attachedWires[i].otherSocket != nullptr) {
            otherPositionInRackSpace = RackView::instance->getLocalPoint(attachedWires[i].otherSocket, attachedWires[i].otherSocket->getLocalBounds().toFloat().getCentre());
        }
        // connecting to knob
        else if(attachedWires[i].otherKnob != nullptr) {
            otherPositionInRackSpace = RackView::instance->getLocalPoint(attachedWires[i].otherKnob, attachedWires[i].otherKnob->getLocalBounds().toFloat().getCentre());
            endIsKnob = true;
        }
        attachedWires[i].wire->SetStartEnd(socketCentreInRackSpace, otherPositionInRackSpace, endIsKnob);
    }
}

void WireSocket::SetNumActiveVoices(int _numActiveVoices) {
    this->numActiveVoices = _numActiveVoices;
}