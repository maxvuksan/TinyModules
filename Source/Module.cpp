#include "Module.h"
#include "Globals.h"
#include "RackView.h"



Module::Module(int widthUnitsIn, std::string moduleName)
    : widthUnits(widthUnitsIn), moduleName(moduleName) {

    setSize(widthUnits * GLOBAL_BLOCK_WIDTH_UNIT, (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING));

    inputDSPIndexIncrement = 0;
    outputDSPIndexIncrement = 0;

    selected = false;

    // rack is responsible for moving modules, however components should control their own interactions
    setInterceptsMouseClicks(false, true);
}

Module::~Module() {

}

void Module::paint(juce::Graphics& g){

    auto bounds = getLocalBounds().toFloat();

    // Optional: Draw header
    auto headerBounds = bounds;
    headerBounds.setHeight(GLOBAL_BLOCK_HEIGHT_PADDING);
    headerBounds.removeFromTop(2);
    headerBounds.removeFromBottom(4);
    headerBounds.removeFromLeft(2);
    headerBounds.removeFromRight(2);

    g.setFont(juce::Font(CustomLookAndFeel::GetTheme()->font));

    g.setColour(CustomLookAndFeel::GetTheme()->colour_blockHeader);
    g.fillRoundedRectangle(headerBounds, CustomLookAndFeel::GetTheme()->borderRadius);



    // Main body below the header
    bounds.removeFromTop(GLOBAL_BLOCK_HEIGHT_PADDING);

    bounds.removeFromLeft(2);
    bounds.removeFromRight(2);

    // Draw filled rounded rectangle
    g.setColour(CustomLookAndFeel::GetTheme()->colour_blockFill);
    g.fillRoundedRectangle(bounds, CustomLookAndFeel::GetTheme()->borderRadius); // Adjust corner radius as needed
    //g.setColour(CustomLookAndFeel::GetTheme()->colour_blockHeader);
    //g.setColour(CustomLookAndFeel::GetTheme()->colour_blockHeaderText);






    if (RackView::instance->GetLODFactor() != LOD_CLOSE) {
        return;
    }

    // draw module title
    g.setColour(CustomLookAndFeel::GetTheme()->colour_blockHeaderText);
    g.drawText(moduleName,
        headerBounds.getX(), 6,
        headerBounds.getWidth(), 14.0f,
        juce::Justification::centredTop);

    // paint text for components

    const int textOffsetY = -14;
    g.setColour(CustomLookAndFeel::GetTheme()->colour_knobPointer); 
    for (const auto& [label, knob] : componentsKnobs)
    {
        auto bounds = knob.second->getBounds().toFloat();

        g.drawText(knob.first.displayName,
            bounds.getX(), bounds.getBottom() + textOffsetY,
            bounds.getWidth(), 14.0f,
            juce::Justification::centredTop);
    }

    for (const auto& [label, socket] : componentsSockets)
    {
        auto bounds = socket.second->getBounds().toFloat();

        g.drawText(socket.first.displayName,
            bounds.getX(), bounds.getBottom() + textOffsetY,
            bounds.getWidth(), 14.0f,
            juce::Justification::centredTop);
    }


}

void Module::paintOverChildren(juce::Graphics& g) {
    
    if (selected) {
        g.setColour(CustomLookAndFeel::GetTheme()->colour_selectionFill);
        g.fillRect(getLocalBounds().reduced(2.0f));
        g.setColour(CustomLookAndFeel::GetTheme()->colour_selectionOutline);
        g.drawRect(getLocalBounds().reduced(2.0f), 2);
    }

    // each knob should be drawn with a selection
    if (WireManager::instance->GetCanConnectToKnob()) {

        for (auto& knob : componentsKnobs) {

            g.setColour(CustomLookAndFeel::GetTheme()->colour_selectionFill);
            g.fillEllipse(
                knob.second.second->getPosition().x + 2,
                knob.second.second->getPosition().y + 7,
                knob.second.second->getWidth() - 4, 
                knob.second.second->getWidth() - 4);

            g.setColour(CustomLookAndFeel::GetTheme()->colour_selectionOutline);

        }

    }
}



void Module::Prepare(double _sampleRate, int blockSize, int numInputSockets, int numOutputSockets) {

    /*
        Currently each sockets audio buffer is allocated 16 channels 
        (to simplify the interaction between mono and polyphonic connections)

        TODO: if memory performance issues present themselves, dynamically changing
                the number of channels (set to 0 when mono, 16 when poly) could be good.
    */

    inputBuffers.resize(numInputSockets);
    
    for (int i = 0; i < numInputSockets; i++) {
        inputBuffers[i].setSize(16, blockSize);
        inputBuffers[i].clear();
    }

    outputBuffers.resize(numOutputSockets);

    for (int i = 0; i < numOutputSockets; i++) {
        outputBuffers[i].setSize(16, blockSize);
        outputBuffers[i].clear();

    }
    this->sampleRate = _sampleRate;
}

juce::var Module::Serialize() {
    
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();

    // meta data about module
    obj->setProperty("type", (int)moduleType);   
    obj->setProperty("x", rackXPosition);
    obj->setProperty("y", rackYPosition);

    juce::DynamicObject::Ptr knobData = new juce::DynamicObject();

    for (const auto& [label, pair] : componentsKnobs) {

        knobData->setProperty(juce::String(label), pair.second->getValue());
    }
    obj->setProperty("knobs", juce::var(knobData.get()));


    // Call module-specific custom serialization hook
    juce::var custom = SerializeCustom();

    if (custom.isObject()) {
        obj->setProperty("custom", custom);
    }

    return obj.get();
}

void Module::Deserialize(const juce::var& data) {

    // if not valid, cancel
    if (!data.isObject()) {
        return;
    }
    auto* obj = data.getDynamicObject();
    if (!obj) {
        return;
    }

    // Restore knob values
    auto knobVar = obj->getProperty("knobs");

    if (knobVar.isObject()) {

        auto* knobObj = knobVar.getDynamicObject();
        for (const auto& [label, pair] : componentsKnobs) {
            
            if (knobObj->hasProperty(juce::String(label))) {
                pair.second->setValue(knobObj->getProperty(juce::String(label)));
            }
        }
    }

    // Call custom deserialization hook
    auto customVar = obj->getProperty("custom");
    if (customVar.isObject()) {
        DeserializeCustom(customVar);
    }

}

void Module::SetSelected(bool selected) {
    this->selected = selected;
    repaint();
}
void Module::SetGridOffsetOnSelection(int x, int y) {
    this->gridOffsetOnSelectionX = x;
    this->gridOffsetOnSelectionY = y;
}

juce::Point<int> Module::GetGridOffsetOnSelection() {
    return { gridOffsetOnSelectionX, gridOffsetOnSelectionY };
}

WireSocket* Module::GetSocketFromDspIndex(int dspIndex, bool isInput) {

    for (auto& [name, pair] : componentsSockets)
    {
        auto& socket = pair.second;

        if (socket && socket->GetDSPIndex() == dspIndex && socket->GetIsInput() == isInput)
        {
            return socket.get();
        }
    }

    return nullptr; // Not found
}


void Module::mouseDown(const juce::MouseEvent& e)
{
    return;
    // Only start dragging if we didn't click on an excluded child
    if (e.eventComponent && e.eventComponent != this) {
        return;
    }

    if (!e.mods.isMiddleButtonDown()) {
        return;
    }


    mouseMovingBlockStartPos = e.getPosition();

}

void Module::mouseDrag(const juce::MouseEvent& e)
{
    return;
    if (!e.mods.isMiddleButtonDown()) {
        return;
    }


    // If we didn't click on a slider/socket, perform drag
    if (!e.eventComponent || e.eventComponent == this)
    {
        auto screenPos = e.getScreenPosition();
        auto parentPos = getParentComponent()->getLocalPoint(nullptr, screenPos);
        // Offset by the point we first grabbed the block
        auto newTopLeft = parentPos - mouseMovingBlockStartPos;

           
        newRackPosition.x = round(newTopLeft.x / (float)GLOBAL_BLOCK_WIDTH_UNIT);
        newRackPosition.y = round(newTopLeft.y / (float)(GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING));

        setTopLeftPosition(newTopLeft);

        RackView::instance->SetFloatingBlockFootprintShow(true);
        RackView::instance->SetFloatingBlockFootprint(newRackPosition, widthUnits);
        
        // since we are moving the module we should also trigger wire redraws
        for (auto& socket : componentsSockets) {
            socket.second.second->RecomputeWireGraphics();
        }
    }
}

void Module::mouseUp(const juce::MouseEvent& e) {

    return;
    if (!e.mods.isMiddleButtonDown()) {
        return;
    }


    RackView::instance->MoveModule(rackXPosition, rackYPosition, newRackPosition.x, newRackPosition.y);
    RackView::instance->SetFloatingBlockFootprintShow(false);
    
    setTopLeftPosition({
        newRackPosition.x * GLOBAL_BLOCK_WIDTH_UNIT, 
        newRackPosition.y * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)
    });

    // since we are moving the module we should also trigger wire redraws
    for (auto& socket : componentsSockets) {
        socket.second.second->RecomputeWireGraphics();
    }
}




Knob& Module::Component_CreateKnob(const std::string& label, int x, int y, KnobConfiguration* configuration)
{
    auto [it, inserted] = componentsKnobs.try_emplace(label, ComponentConfig{}, std::make_unique<Knob>());

    KnobConfiguration finalConfig;

    if (configuration != nullptr) {
        finalConfig = *configuration;
    }

    it->second.first.displayName = SanitizeLabel(label);
    it->second.first.coordinate = { x, y };
    it->second.first.size = { 1, 1 };

    auto& slider = it->second.second;
    slider->Configure(&finalConfig);

    for (int i = 0; i < rand() % 7; i++) {
        slider->AddConnectedWire(nullptr);
    }


    addAndMakeVisible(*slider);
    const auto& config = it->second.first;
    SetComponentBounds(*slider, config.coordinate.x, config.coordinate.y, config.size.x, config.size.y);

    return *it->second.second;
}

int Module::Component_CreateInputSocket(const std::string& label, int x, int y, ConnectionType socketType) {
    return Component_CreateSocket(label, x, y, true, socketType);
}
int Module::Component_CreateOutputSocket(const std::string& label, int x, int y, ConnectionType socketType) {
    return Component_CreateSocket(label, x, y, false, socketType);
}
int Module::Component_CreateSocket(const std::string& label, int x, int y, bool isInput, ConnectionType socketType) {
    
    // labels which start with # are not displayed

    int dspIndex;
    if (isInput) {
        dspIndex = inputDSPIndexIncrement;
        inputDSPIndexIncrement++;
    }
    else {
        dspIndex = outputDSPIndexIncrement;
        outputDSPIndexIncrement++;
    }


    auto [it, inserted] = componentsSockets.try_emplace(label, ComponentConfig{}, std::make_unique<WireSocket>(this, isInput, dspIndex, socketType));
    auto& socket = *it->second.second;

    it->second.first.displayName = SanitizeLabel(label);
    it->second.first.coordinate = { x, y };
    it->second.first.size = { 1, 1 };

    addAndMakeVisible(socket);
    const auto& config = it->second.first;
    SetComponentBounds(socket, config.coordinate.x, config.coordinate.y, config.size.x, config.size.y);

    return dspIndex;

}

Switch& Module::Component_CreateSwitch(const std::string& label, int x, int y, const std::string& innerLabel, bool defaultState) {

    auto [it, inserted] = componentsSwitches.try_emplace(label, ComponentConfig{}, std::make_unique<Switch>());
    auto& _switch = *it->second.second;
    
    it->second.first.displayName = SanitizeLabel(label);
    it->second.first.coordinate = { x, y };
    it->second.first.size = { 1, 1 };

    _switch.SetState(defaultState);
    _switch.SetInnerLabel(innerLabel);

    addAndMakeVisible(_switch);
    const auto& config = it->second.first;
    SetComponentBounds(_switch, config.coordinate.x, config.coordinate.y, config.size.x, config.size.y);

    return _switch;
}

ComponentScope& Module::Component_CreateScope(const std::string& label, int x, int y) {

    auto [it, inserted] = componentsScopes.try_emplace(label, ComponentConfig{}, std::make_unique<ComponentScope>());
    auto& cs = *it->second.second;
   
    it->second.first.displayName = SanitizeLabel(label);
    it->second.first.coordinate = { x, y };
    it->second.first.size = { 2, 2 };

    addAndMakeVisible(cs.visualiser);
    cs.visualiser.setInterceptsMouseClicks(false, false); // clicks pass through to block

    const auto& config = it->second.first;
    SetComponentBounds(it->second.second->visualiser, config.coordinate.x, config.coordinate.y, config.size.x, config.size.y);

    return cs;
}

void Module::SetComponentBounds(juce::Component& component, int x, int y, int width, int height) {

    component.setBounds(
        x * GLOBAL_COMPONENT_GRID_POS_X,
        y * GLOBAL_COMPONENT_GRID_POS_Y,
        width * GLOBAL_COMPONENT_WIDTH,
        height * GLOBAL_COMPONENT_HEIGHT
    );
}

double Module::Component_GetKnobValue(const std::string& label) {

    if (componentsKnobs.find(label) != componentsKnobs.end()) {
        return componentsKnobs[label].second->getValue();
    }
    return 0;
}

Knob* Module::Component_GetKnob(const std::string& label) {
    if (componentsKnobs.find(label) != componentsKnobs.end()) {
        return componentsKnobs[label].second.get();
    }
    return nullptr;
}

WireSocket* Module::Component_GetSocket(const std::string& label) {
    if (componentsSockets.find(label) != componentsSockets.end()) {
        return componentsSockets[label].second.get();
    }
    return nullptr;
}

Switch* Module::Component_GetSwitch(const std::string& label) {
    if (componentsSwitches.find(label) != componentsSwitches.end()) {
        return componentsSwitches[label].second.get();
    }
    return nullptr;
}


juce::AudioBuffer<float>& Module::GetOutputBuffer(int dspIndex) { 
    return outputBuffers[dspIndex]; 
}

juce::AudioBuffer<float>& Module::GetInputBuffer(int dspIndex) { 
    return inputBuffers[dspIndex]; 
}

std::vector <juce::AudioBuffer<float>>& Module::GetOutputBuffers() {
    return outputBuffers;
}
std::vector <juce::AudioBuffer<float>>& Module::GetInputBuffers() {
    return inputBuffers;
}

/*
    @returns a pointer to a specific channel of a sockets audio buffer
*/
const float* Module::GetInputReadPtr(int dspIndex, int channel) {
    return inputBuffers[dspIndex].getReadPointer(channel);
}
float* Module::GetInputWritePtr(int dspIndex, int channel) {
    return inputBuffers[dspIndex].getWritePointer(channel);
}
const float* Module::GetOutputReadPtr(int dspIndex, int channel) {
    return outputBuffers[dspIndex].getReadPointer(channel);
}
float* Module::GetOutputWritePtr(int dspIndex, int channel) {
    return outputBuffers[dspIndex].getWritePointer(channel);
}


void Module::resized()
{
    setTopLeftPosition(rackXPosition * GLOBAL_BLOCK_WIDTH_UNIT, rackYPosition * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING));  // position to rack rail
}


int Module::GetWidthUnits() const {
    return widthUnits;
}

void Module::SetRackPosition(int x, int y) { 
    this->rackXPosition = x; this->rackYPosition = y;  
}

juce::Point<int> Module::GetRackPosition() { 
    return { rackXPosition, rackYPosition }; 
}

void Module::SetModuleType(ModuleType type) { 
    this->moduleType = type; 
}

const ModuleType Module::GetModuleType() { 
    return this->moduleType; 
}

std::string Module::GetModuleName() { 
    return this->moduleName; 
}

std::string Module::SanitizeLabel(const std::string& label) {
    
    auto pos = label.find('#');
    
    if (pos != std::string::npos) {
        return label.substr(0, pos);
    }

    return label;
}
