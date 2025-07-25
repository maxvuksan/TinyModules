/*
  ==============================================================================

    WireManager.cpp
    Created: 24 Jun 2025 9:37:14pm
    Author:  maxvu

  ==============================================================================
*/

#include "WireManager.h"
#include "Globals.h"

int WireManager::currentWireColourIndex = 0;

WireManager* WireManager::instance = nullptr;

WireManager::WireManager() {

    setInterceptsMouseClicks(false, false);
    instance = this;
}


WireComponent* WireManager::createWire()
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

void WireManager::removeWire(WireComponent* wire) {

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
