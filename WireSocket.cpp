#include "WireSocket.h"
#include "Globals.h"
#include "WireManager.h"
#include "RackView.h"
#include "CustomLookAndFeel.h"

WireSocket::WireSocket(Module* _module, bool _isInput, int _dspIndex, ConnectionType _connectionType):
    block(_module), isInput(_isInput), connectionType(_connectionType), dspIndex(_dspIndex)
{
    creatingNewWire = false;
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

    if (isInput) {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_knobOutline);
        g.drawRoundedRectangle(squareBaseRect, 2.0f, CustomLookAndFeel::GetTheme()->borderRadius);
    }
    else {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
        g.fillRoundedRectangle(squareBaseRect, CustomLookAndFeel::GetTheme()->borderRadius);
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

    // creating wire when ctrl down

    if (e.mods.isCtrlDown() || attachedWires.size() == 0) {

        WireComponent* wire = WireManager::instance->createWire();
        wire->SetConnectionType(connectionType);
        attachedWires.push_back(WireAttachedToSocket({ wire, nullptr, connectionType }));
        creatingNewWire = true;
    }
    else {
        movingWire = true;

        wiresBeingMoved.resize(attachedWires.size());

        for (int i = 0; i < attachedWires.size(); i++) {
            wiresBeingMoved[i] = attachedWires[i];

            bool removed = true;
            while(removed){

                // remove wire from other socket
                removed = false;
                for (int x = 0; x < attachedWires[i].otherSocket->attachedWires.size(); x++) {

                    if (attachedWires[i].otherSocket->attachedWires[x].otherSocket = this) {

                        attachedWires[i].otherSocket->attachedWires.erase(attachedWires[i].otherSocket->attachedWires.begin() + x);
                        removed = true;
                        break;
                    }
                }
            }

            
            RemoveConnectionInProcessing(attachedWires[i]);
        }

        attachedWires.clear();
    }
}



void WireSocket::mouseDrag(const juce::MouseEvent& e) {

    if (creatingNewWire) {

        juce::Point<float> socketCentreInRackSpace =
            RackView::instance->getLocalPoint(this, getLocalBounds().toFloat().getCentre());

        juce::Point<float> mousePositionInRackSpace =
            RackView::instance->getLocalPoint(this, e.getPosition().toFloat());


        attachedWires[attachedWires.size() - 1].wire->SetStartEnd(socketCentreInRackSpace, mousePositionInRackSpace);
    }
    if (movingWire) {

        juce::Point<float> mousePositionInRackSpace =
            RackView::instance->getLocalPoint(this, e.getPosition().toFloat());

        for (int i = 0; i < wiresBeingMoved.size(); i++) {

            juce::Point<float> otherSocketCentreInRackSpace =
                RackView::instance->getLocalPoint(wiresBeingMoved[i].otherSocket, getLocalBounds().toFloat().getCentre());
            
            wiresBeingMoved[i].wire->SetStartEnd(mousePositionInRackSpace, otherSocketCentreInRackSpace);
        }
    }
}
void WireSocket::mouseUp(const juce::MouseEvent& e) {

    auto mousePosInRack = RackView::instance->getLocalPoint(this, e.getPosition().toFloat());
    auto* comp = RackView::instance->getComponentAt(mousePosInRack.roundToInt());

    auto* hoveredSocket = dynamic_cast<WireSocket*>(comp);

    if (creatingNewWire) {
                                                                // cannot assign input to input or output to output
        if (hoveredSocket && hoveredSocket != this && hoveredSocket->isInput != this->isInput) {

            juce::Point<float> socketCentreInRackSpace =
                RackView::instance->getLocalPoint(this, getLocalBounds().toFloat().getCentre());

            juce::Point<float> hoveredSocketInRackSpace =
                RackView::instance->getLocalPoint(hoveredSocket, hoveredSocket->getLocalBounds().toFloat().getCentre());


            attachedWires[attachedWires.size() - 1].otherSocket = hoveredSocket;
            attachedWires[attachedWires.size() - 1].wire->SetStartEnd(socketCentreInRackSpace, hoveredSocketInRackSpace);

            AddConnectionInProcessing(attachedWires[attachedWires.size() - 1]);

            // finally tell the other socket about the connection

            WireAttachedToSocket connection;
            connection.wire = attachedWires[attachedWires.size() - 1].wire;
            connection.otherSocket = this;

            hoveredSocket->assignWireFromOtherSocket(connection);
        }
        else {
            // remove incomplete wire
            WireManager::instance->removeWire(attachedWires[attachedWires.size() - 1].wire);
            RemoveConnectionInProcessing(attachedWires[attachedWires.size() - 1]);

            attachedWires.pop_back();
        

        }

        creatingNewWire = false;
    }
    
    if (movingWire) {

                            // the socket wires are moved to should be of the same type, e.g. input moves to another input socket
        if (hoveredSocket && hoveredSocket->isInput == this->isInput) {

            juce::Point<float> thisSocketCentreInRackSpace =
                RackView::instance->getLocalPoint(hoveredSocket, hoveredSocket->getLocalBounds().toFloat().getCentre());

            for (int i = 0; i < wiresBeingMoved.size(); i++) {

                juce::Point<float> otherSocketCentreInRackSpace =
                    RackView::instance->getLocalPoint(wiresBeingMoved[i].otherSocket, getLocalBounds().toFloat().getCentre());

                wiresBeingMoved[i].wire->SetStartEnd(thisSocketCentreInRackSpace, otherSocketCentreInRackSpace);

                hoveredSocket->attachedWires.push_back(wiresBeingMoved[i]);
                
                // reconfigure connection at otherSocket
                for (int x = 0; x < wiresBeingMoved[i].otherSocket->attachedWires.size(); x++) {

                    // tell otherSocket the end point has changed
                    if (wiresBeingMoved[i].otherSocket->attachedWires[x].wire == wiresBeingMoved[i].wire) {
                        wiresBeingMoved[i].otherSocket->attachedWires[x].otherSocket = hoveredSocket;
                        break;
                    }
                }

                AddConnectionInProcessing(wiresBeingMoved[i]);
            }
        }
        else {
            for (int i = 0; i < wiresBeingMoved.size(); i++) {
            
                // reconfigure connection at otherSocket
                for (int x = 0; x < wiresBeingMoved[i].otherSocket->attachedWires.size(); x++) {

                    // tell otherSocket the end point the wire has been destroyed
                    if (wiresBeingMoved[i].otherSocket->attachedWires[x].wire == wiresBeingMoved[i].wire) {
                        wiresBeingMoved[i].otherSocket->attachedWires.erase(wiresBeingMoved[i].otherSocket->attachedWires.begin() + x);
                        break;
                    }
                }

                WireManager::instance->removeWire(wiresBeingMoved[i].wire);
            }
        }

        wiresBeingMoved.clear();
        movingWire = false;
    }
}

void WireSocket::AddConnectionInProcessing(const WireAttachedToSocket& connection) {

    if (connection.otherSocket == nullptr) {
        return;
    }

    if (isInput) {
        RackView::processingManager.AddConnection(
            connection.connectionType,
            connection.otherSocket->GetModule(),
            connection.otherSocket->GetDSPIndex(),
            GetModule(),
            dspIndex);
    }
    else {
        RackView::processingManager.AddConnection(
            connection.connectionType,
            GetModule(),
            dspIndex,
            connection.otherSocket->GetModule(),
            connection.otherSocket->GetDSPIndex());
    }
}

void WireSocket::RemoveConnectionInProcessing(const WireAttachedToSocket& connection) {

    if (connection.otherSocket == nullptr) {
        return;
    }

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

void WireSocket::assignWireFromOtherSocket(WireAttachedToSocket connection) {
    
    attachedWires.push_back(connection);
}

void WireSocket::recomputeWireGraphics() {

    for (int i = 0; i < attachedWires.size(); i++) {

        juce::Point<float> socketCentreInRackSpace =
            RackView::instance->getLocalPoint(this, getLocalBounds().toFloat().getCentre());

        juce::Point<float> otherPositionInRackSpace =
            RackView::instance->getLocalPoint(attachedWires[i].otherSocket, attachedWires[i].otherSocket->getLocalBounds().toFloat().getCentre());


        attachedWires[i].wire->SetStartEnd(socketCentreInRackSpace, otherPositionInRackSpace);
    }
}


// Position of socket center, for drawing wires
juce::Point<int> getConnectionPoint() {
    return { 0,0 };
}

