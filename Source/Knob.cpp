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

void Knob::AddConnectedWire(WireSocket* otherSocket) {

}

void Knob::RemoveConnectedWire(WireSocket* otherSocket) {

}

void Knob::RemoveAllConnectedWires() {

}


void Knob::valueChanged() {
    repaint(); // Optional: call in constructor or listener
}

void Knob::paint(juce::Graphics& g)
{
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

