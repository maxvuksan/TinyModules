#include "WaveformVisual.h"
#include "CustomLookAndFeel.h"

WaveformVisual::WaveformVisual() {
    currentType = DSP::WaveType::Saw;
}

void WaveformVisual::SetWaveType(DSP::WaveType waveType) {

    if (currentType == waveType) {
        return;
    }

    currentType = waveType;
    rebuildPath();
    repaint();
}

DSP::WaveType WaveformVisual::GetWaveType() {
    return currentType;
}

void WaveformVisual::rebuildPath() {
    waveformPath.clear();

    // Match this to the stroke you use in paint()
    const float strokePx = 2.0f;
    const float aa = 0.5f; // tiny cushion for antialiasing

    // Area you fill/clip in paint()
    auto panel = getLocalBounds().toFloat().reduced(1.0f);

    // Inset path by half the stroke so it renders fully inside the clip
    auto r = panel.reduced(strokePx * 0.5f + aa);
    if (r.getWidth() <= 1.0f || r.getHeight() <= 1.0f)
        return;

    const float midY = r.getCentreY();
    float ampPx = 0.5f * r.getHeight() * 0.6f;                       

    const int   N = juce::jmax(2, (int)std::round(r.getWidth())); // ~1 sample per pixel

    const float x0 = r.getX();
    const float w = r.getWidth();

    for (int i = 0; i < N; ++i) {
        const float t = (N == 1) ? 0.0f : (float)i / (float)(N - 1);          // 0..1 inclusive
        const float phase = t * juce::MathConstants<float>::twoPi;                // radians 0..2pi
        const float x = x0 + t * w;
        const float y = midY - DSP::SampleWaveform(currentType, phase) * ampPx;

        if (i == 0) waveformPath.startNewSubPath(x, y);
        else        waveformPath.lineTo(x, y);
    }
}

void WaveformVisual::paint(juce::Graphics& g) {

    const auto* theme = CustomLookAndFeel::GetTheme();

    auto panel = getLocalBounds().toFloat().reduced(1);
    g.setColour(theme->colour_displayPanel);
    g.fillRect(panel);
    g.setColour(theme->colour_switchOff);
    g.fillRect(panel.reduced(8));

    // draw the waveform as a solid-colour path
    g.setColour(theme->colour_switchOn); // pick something visible vs panel
    g.strokePath(waveformPath, juce::PathStrokeType(2.0f));

}

void WaveformVisual::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() || e.mods.isLeftButtonDown())
    {
        juce::PopupMenu menu;

        menu.addItem("Reset", [this]()
            {
                SetWaveType(DSP::WaveType::Sine);
            });

        menu.addSeparator();

        menu.addItem("Sine", [this]()
            {
                SetWaveType(DSP::WaveType::Sine);
            });
        menu.addItem("Saw", [this]()
            {
                SetWaveType(DSP::WaveType::Saw);
            });
        menu.addItem("Square", [this]()
            {
                SetWaveType(DSP::WaveType::Square);
            });
        menu.addItem("Triangle", [this]()
            {
                SetWaveType(DSP::WaveType::Triangle);
            });
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
    }
}


void WaveformVisual::resized() {
    rebuildPath();
}
