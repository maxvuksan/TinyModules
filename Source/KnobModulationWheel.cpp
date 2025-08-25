#include "KnobModulationWheel.h"
#include "CustomLookAndFeel.h"
#include "RackView.h"

KnobModulationWheel::KnobModulationWheel()
{
    setSize(16, 16);
    setRepaintsOnMouseActivity(true);
}

void KnobModulationWheel::setValue(float v, juce::NotificationType nt)
{
    v = juce::jlimit(-1.0f, 1.0f, v);
    if (value == v) return;

    value = v;
    repaint();

    if (nt == juce::sendNotification && onChange)
        onChange(value);
}


void KnobModulationWheel::paint(juce::Graphics& g)
{



    auto ring = getLocalBounds().toFloat().reduced(1.0f);
    
    if (RackView::instance->GetLODFactor() == LOD_CLOSE) {

        auto centre = ring.getCentre();
        float r = juce::jmin(ring.getWidth(), ring.getHeight()) * 0.5f;

        g.setColour(CustomLookAndFeel::GetTheme()->colour_wiresOff[wireColourIndex]);
        g.fillEllipse(ring);

        // Map value [-1..1] -> angle [-pi..+pi]
        const float angle = value * juce::MathConstants<float>::pi;

        const float angleOffset = 0.0f;

        if (value != 0.0f)
        {
            float a0 = angleOffset;
            float a1 = angleOffset + angle;
            if (a1 < a0) std::swap(a0, a1);            // ensure start < end for the pie segment

            juce::Path pie;
            pie.addPieSegment(ring, a0, a1, 0.0f);     // innerRadiusRatio=0 => from centre
            g.setColour(CustomLookAndFeel::GetTheme()->colour_wires[wireColourIndex]);
            g.fillPath(pie);
        }
        g.setColour(CustomLookAndFeel::GetTheme()->colour_wires[wireColourIndex]);
        g.drawEllipse(ring, 0.5f);
    }
    else {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_wires[wireColourIndex]);
        g.fillEllipse(ring);
    }

}

void KnobModulationWheel::mouseDown(const juce::MouseEvent& e)
{
    dragStartPos = e.getPosition();
    startDragValue = value;
}

void KnobModulationWheel::mouseDrag(const juce::MouseEvent& e)
{
    // rotary vertical drag: moving up -> increase, down -> decrease
    const int dy = e.getPosition().y - dragStartPos.y;

    // sensitivity: tweak as needed (smaller = slower)
    constexpr float perPixel = 0.01f;

    setValue(startDragValue - dy * perPixel);
}

void KnobModulationWheel::mouseDoubleClick(const juce::MouseEvent&)
{
    setValue(0.0f);
}
