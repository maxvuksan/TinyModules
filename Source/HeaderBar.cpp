
#include "HeaderBar.h"
#include "SerializerManager.h"
#include "Popups.h"

HeaderBar::HeaderBar() {

    fileChooser = std::make_unique<juce::FileChooser>("Save Tiny Modules Project",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        std::string("*") + GLOBAL_FILE_EXTENSION);

    menuButton.setTriggeredOnMouseDown(true); // optional, faster menu
    menuButton.onClick = [this] { ShowMenuCallback(); };
    addAndMakeVisible(menuButton);

    // Title label setup
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
}

HeaderBar::~HeaderBar() {

}

void HeaderBar::paint(juce::Graphics& g) {

    g.fillAll(CustomLookAndFeel::GetTheme()->colour_headerBar);
}

void HeaderBar::resized() {

    auto area = getLocalBounds().reduced(10);
    menuButton.setBounds(area.removeFromLeft(40));
    titleLabel.setBounds(area.removeFromLeft(200)); 
}

void HeaderBar::ShowSaveDialog()
{
    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [](const juce::FileChooser& fc)
        {
            juce::File file = fc.getResult();
            if (file != juce::File{})
            {
                file = file.withFileExtension(GLOBAL_FILE_EXTENSION);
                // Save to file...

                SerializerManager::SaveToFile(*RackView::instance, file);
            }
        });
}

void HeaderBar::ShowLoadDialog() {

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [](const juce::FileChooser& fc)
        {
            auto selectedFile = fc.getResult();

            SerializerManager::LoadFromFile(selectedFile, *RackView::instance);
        });
}

void HeaderBar::ShowMenuCallback() {

    juce::PopupMenu menu;

    menu.addItem("New Patch...", []() { juce::Logger::outputDebugString("New Project clicked"); });
    menu.addItem("Load Patch...", [this]() { this->ShowLoadDialog(); });
    menu.addItem("Save Patch...", [this]() { this->ShowSaveDialog(); });

    menu.addSeparator();

    menu.addItem("Settings", [this]() { Popups::OpenPopup(POPUP_SETTINGS); });

    menu.addSeparator();

    menu.addItem("Quit", [this]() {});

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&menuButton));
}   

