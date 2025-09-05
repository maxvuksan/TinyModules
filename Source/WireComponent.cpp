#include "WireComponent.h"
#include "Globals.h"
#include "Popup_Settings.h"
#include <juce_graphics/juce_graphics.h>

WireComponent::WireComponent() {

    setInterceptsMouseClicks(false, false);
    endIsKnob = false;
}

void WireComponent::SetStartEnd(juce::Point<float> start, juce::Point<float> end, bool _endIsKnob)
{
    this->endIsKnob = _endIsKnob;
    startPos = start;

    endPos = end;


    // Compute sag based on distance
    float distance = start.getDistanceFrom(end);
    float sagScale = juce::jlimit(10.0f, 80.0f, distance * 0.25f);
    float sag = GLOBAL_WIRE_SAG * (1 - (float)Popup_Settings::GetSettings().wireTension.getValue());
    sag *= sagScale;

    // Midpoint + sag to calculate control point
    float cx = (start.x + end.x) * 0.5f;
    float cy = (start.y + end.y) * 0.5f + sag;

    // Create a Path for calculating bounds
    juce::Path tempPath;
    tempPath.startNewSubPath(start);
    tempPath.quadraticTo({ cx, cy }, end);

    // Keep bounds as Rectangle<float>
    auto boundsFloat = tempPath.getBounds().expanded(10.0f);
    auto newBounds = boundsFloat.getSmallestIntegerContainer();

    setBounds(newBounds);
    // Adjust internal points relative to new bounds
    startPos -= newBounds.getPosition().toFloat();
    endPos -= newBounds.getPosition().toFloat();
    repaint();
}

void WireComponent::SetWireColourIndex(int index) {
    this->colourIndex = index;
    repaint();
}

int WireComponent::GetWireColourIndex() {
    return colourIndex;
}

void WireComponent::SetConnectionType(ConnectionType connectionType) {
    this->connectionType = connectionType;
    repaint();
}

void WireComponent::paint(juce::Graphics& g)
{
    g.setColour(CustomLookAndFeel::GetTheme()->colour_wires[colourIndex]);

    juce::Path path;
    auto p1 = startPos;
    auto p2 = endPos;

    float distance = p1.getDistanceFrom(p2);
    float sagScale = juce::jlimit(10.0f, 80.0f, distance * 0.25f);
    float sag = GLOBAL_WIRE_SAG * (1 - (float)Popup_Settings::GetSettings().wireTension.getValue());
    sag *= sagScale;

    float cx = (p1.x + p2.x) * 0.5f;
    float cy = (p1.y + p2.y) * 0.5f + sag; // center line, sag downward

    path.startNewSubPath(p1);
    path.quadraticTo(cx, cy, p2.x, p2.y);

    float thickness = 2.0f;
    if (connectionType == CONNECT_POLY) {
        thickness = 6.0f;
    }

    g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Rectangle<float> startCircle(
        startPos.x - 7,
        startPos.y - 7,
        14,
        14
    );
    g.fillEllipse(startCircle);


    juce::Rectangle<float> endCircle(
        endPos.x - 6,
        endPos.y - 6,
        12,
        12
    );

    // dont draw end circle if end is knob
    if (endIsKnob) {
        g.fillEllipse(endCircle.reduced(2.0f));

    }
    else {
        g.fillEllipse(endCircle);
    }
}