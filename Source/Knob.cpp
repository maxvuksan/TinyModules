#include "Knob.h"
#include "CustomLookAndFeel.h"
#include "RackView.h"

Knob::Knob() {
    defaultValue = 0;
    visuallyEnabled = true;
}

void Knob::Configure(KnobConfiguration* configuration) {

    if (configuration == nullptr) {
        return;
    }

    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setVelocityBasedMode(false);
    setRange(configuration->min, configuration->max, configuration->increment);
    setValue(configuration->defaultValue);

    if (configuration->skewAroundDefault) {
        setSkewFactorFromMidPoint(configuration->defaultValue);
    }

    setRotaryParameters(
        juce::MathConstants<float>::pi * 0.75f,  // Start ( 135°)
        juce::MathConstants<float>::pi * 2.25f,  // End   ( 405°)
        true
    );

    minVal = configuration->min;
    maxVal = configuration->max;

    modScalePositive = configuration->max - configuration->defaultValue;
    modScaleNegative = configuration->defaultValue - configuration->min;

    defaultValue = configuration->defaultValue;
    colourType = configuration->colourType;
}


void Knob::SetVisuallyEnabled(bool state) {

    if (state == visuallyEnabled) {
        return;
    }

    this->visuallyEnabled = state;
    repaint();
}

void Knob::SetManualValue(double value) {
    setValue(value);
}

double Knob::GetValue(int sampleIndex) {

    double value = getValue();

    if (connectedWires.size() == 0) {
        return value;
    }



    for (int i = 0; i < connectedWires.size(); i++) {

        // TO DO: Currently we are reading connectedWires from the Audio thread, however the UI thread makes changes to this structure. 
        //          this race condition should be fixed. For a temporary fix a null check is performed.
        if (connectedWires[i].modulationWheel == nullptr) {
            continue;
        }

        float modVal = connectedWires[i].modulationWheel->GetValue();
        float sample = connectedWires[i].otherSocket->GetModule()->GetOutputReadPtr(connectedWires[i].otherSocket->GetDSPIndex(), 0)[sampleIndex];

        if (modVal > 0) {
            value += sample * modVal * modScalePositive;
        }
        else {
            value += sample * modVal * modScaleNegative;
        }

    }

    value = std::clamp(value, minVal, maxVal);

    return value;
}

void Knob::RecomputeWireGraphics() {

    for (int i = 0; i < connectedWires.size(); i++) {

        if (connectedWires[i].otherSocket == nullptr) {
            continue;
        }
        connectedWires[i].otherSocket->RecomputeWireGraphics();
    }
}



void Knob::AddConnectedWire(WireSocket* otherSocket, int wireColourIndex, float modulationValue) {

    // is connection already present...?

    if (otherSocket == nullptr) {
        return;
    }

    for (int i = 0; i < connectedWires.size(); i++) {

        if (connectedWires[i].otherSocket == otherSocket) {
            return;
        }
    }

    ConnectedWire& w = connectedWires.emplace_back();
    w.otherSocket = otherSocket;
    w.modulationWheel = WireManager::instance->CreateModulationWheel();
    w.modulationWheel->SetWireColourIndex(wireColourIndex);
    w.modulationWheel->SetValue(modulationValue);

    PositionModulationWheels();
}


void Knob::RemoveConnectedWire(WireSocket* otherSocket) {
 
    for (int i = 0; i < connectedWires.size(); i++) {

        if (connectedWires[i].otherSocket == otherSocket) {
            
            WireManager::instance->RemoveModulationWheel(connectedWires[i].modulationWheel);
            connectedWires.erase(connectedWires.begin() + i);
            return;
        }
    }

    PositionModulationWheels();

}

void Knob::RemoveAllConnectedWires() {

    for (int i = 0; i < connectedWires.size(); i++) {
        //...
    }

    connectedWires.clear();

    PositionModulationWheels();
}

KnobModulationWheel* Knob::GetModulationWheel(WireSocket* otherOutSocket) {

    for (int i = 0; i < connectedWires.size(); i++) {
        
        if (connectedWires[i].otherSocket == otherOutSocket) {
            return connectedWires[i].modulationWheel;
        }
    }

    return nullptr;
}

void Knob::PositionModulationWheels() {

    // position modulation wheels in a grid above the knob

    int margin = 1;
    int itemsPerRow = 3;

    const auto knobInRack = RackView::instance->getLocalArea(this, getLocalBounds());

    for (int i = 0; i < connectedWires.size(); i++) {

        connectedWires[i].modulationWheel->setCentrePosition(
            knobInRack.getPosition()
            + juce::Point<int>{i * (margin + connectedWires[i].modulationWheel->getWidth()), 0
        });
 
        connectedWires[i].modulationWheel->repaint();
    }

}


void Knob::valueChanged() {
    repaint(); 
}

void Knob::moved() {
    PositionModulationWheels();
}
void Knob::resized() {
    PositionModulationWheels();
}

void Knob::paint(juce::Graphics& g)
{
    PositionModulationWheels();
    auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    const float centerX = bounds.getCentreX();
    const float centerY = bounds.getCentreY();

    const float radius = juce::jmin(width, height) / 2.0f - 10.0f;

    const auto& params = getRotaryParameters();
    const float sliderPosProportional = valueToProportionOfLength(getValue());
    const float angle = params.startAngleRadians + sliderPosProportional * (params.endAngleRadians - params.startAngleRadians);

    // --- Arc Drawing ---
    const float arcThickness = 4.0f;
    const float arcPadding = 2.5f;
    const float arcRadius = radius + arcPadding;

    const float arcStart = params.startAngleRadians + juce::MathConstants<float>::pi * 0.5f;
    const float arcEnd = angle + juce::MathConstants<float>::pi * 0.5f;

    // Background arc (full range)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f,
        arcStart,
        params.endAngleRadians + juce::MathConstants<float>::pi * 0.5f,
        true);
    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
    g.strokePath(backgroundArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Foreground arc (current value)
    juce::Path arcPath;
    arcPath.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f,
        arcStart, arcEnd, true);


    if (this->visuallyEnabled) {

        if (colourType == KNOB_COL_DEFAULT) {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_switchOn);
        }
        else {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_switchOutline);
        }
    }
    else {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
    }
    
    
    g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // --- Knob face ---
    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
    g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);

    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobOutline);
    g.drawEllipse(centerX - radius - 1, centerY - radius - 1, (radius + 1) * 2, (radius + 1) * 2, 2.0f);


    if (RackView::instance->GetLODFactor() != LOD_CLOSE) {
        return;
    }

    if (this->visuallyEnabled) {

        if (colourType == KNOB_COL_DEFAULT) {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_switchOn);
        }
        else {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_switchOutline);
        }
    }
    else {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_knobFill);
    }


    // --- Pointer ---
    const float pointerStepoff = radius * 0.6;
    const float pointerLength = radius * 0.64f;
    const float pointerThickness = 1.5f;

    juce::Path pointer;

    float pointX = centerX + pointerStepoff * std::cos(angle);
    float pointY = centerY + pointerStepoff * std::sin(angle);

    pointer.startNewSubPath(pointX, pointY);
    pointer.lineTo(pointX + pointerLength * std::cos(angle),
        pointY + pointerLength * std::sin(angle));

    g.strokePath(pointer, juce::PathStrokeType(pointerThickness));
}


void Knob::SetLabel(const std::string& _label) {
    this->label = _label;
}

void Knob::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        juce::PopupMenu menu;

        menu.addItem("Set", [this]()
            {
                
            });

        menu.addSeparator();

        menu.addItem("Reset", [this]()
            {
                setValue(defaultValue);
            });

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
    }
    else // interact with slider per usual
    {
        // This is required for Slider to handle dragging
        Slider::mouseDown(e);
    }
}

