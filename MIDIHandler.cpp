#include "MIDIHandler.hpp"
MIDIHandler::MIDIHandler(DMXController& controller)
    : controller_(controller) {}

void MIDIHandler::handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (note >= INPUT_NOTE_FIXTURE_HUES_START && note <= INPUT_NOTE_COLOR_SCHEME_START) {

    }
    if (note >= INPUT_NOTE_COLOR_SCHEME_START && note <= INPUT_NOTE_SYMMETRICAL_START) {
        // Color selection notes
        // TODO: Color schemes
        controller_.setAllColors(noteToColor(note));
    }
    else if (note >= NOTE_SYMMETRICAL_START && note <= NOTE_SYMMETRICAL_START + 4) {
        // Symmetrical control notes
        uint8_t leftFixture = (note - NOTE_SYMMETRICAL_START) / 2;
        if (velocity == 127) {
            controller_.setSymmetricalColor(leftFixture);
        }
        else {
            RGB color = velocityToColor(velocity);
            controller_.setSymmetricalColor(leftFixture, color);
        }
    }
    else if (note >= NOTE_INDIVIDUAL_START && note <= NOTE_INDIVIDUAL_START + 7) {
        // Individual fixture control
        uint8_t fixture = note - NOTE_INDIVIDUAL_START;
        if (velocity == 127) {
            controller_.setIndividualColor(fixture);
        }
        else {
            RGB color = velocityToColor(velocity);
            controller_.setIndividualColor(fixture, color);
        }

    }
}

void MIDIHandler::handleControlChange(uint8_t channel, uint8_t control, uint8_t value) {
    // Implement CC handling (e.g., for brightness, attack, release)
    float normalized = value / 127.0f;

    // Example CC mappings:
    switch (control) {
        case 1: // Modulation wheel - global brightness
            for (size_t i = 0; i < 6; ++i) {
                controller_.fixtures_[i].setBrightness(normalized);
            }
            break;
        case 73: // Attack time
            for (size_t i = 0; i < 6; ++i) {
                controller_.fixtures_[i].setAttack(static_cast<int>(normalized * 60));
            }
            break;
        case 72: // Release time
            for (size_t i = 0; i < 6; ++i) {
                controller_.fixtures_[i].setRelease(static_cast<int>(normalized * 60));
            }
            break;
    }
}

RGB MIDIHandler::velocityToColor(uint8_t velocity) const {
    // Convert note to color using HSV->RGB conversion
    uint8_t hue = ((velocity) * 2) % 256;

    // Simple HSV to RGB conversion for full saturation and value
    uint8_t region = hue / 43;
    uint8_t remainder = (hue - (region * 43)) * 6;

    uint8_t p = 0;
    uint8_t q = 0;
    uint8_t t = 0;

    switch (region) {
        case 0: return {255, remainder, p};
        case 1: return {255 - remainder, 255, p};
        case 2: return {p, 255, remainder};
        case 3: return {p, 255 - remainder, 255};
        case 4: return {remainder, p, 255};
        case 5: default: return {255, p, 255 - remainder};
    }
}

RGB MIDIHandler::noteToColor(uint8_t note) const {
    // Convert note to color using HSV->RGB conversion
    uint8_t hue = ((note - NOTE_COLOR_START) * 32) % 256;

    // Simple HSV to RGB conversion for full saturation and value
    uint8_t region = hue / 43;
    uint8_t remainder = (hue - (region * 43)) * 6;

    uint8_t p = 0;
    uint8_t q = 0;
    uint8_t t = 0;

    switch (region) {
        case 0: return {255, remainder, p};
        case 1: return {255 - remainder, 255, p};
        case 2: return {p, 255, remainder};
        case 3: return {p, 255 - remainder, 255};
        case 4: return {remainder, p, 255};
        case 5: default: return {255, p, 255 - remainder};
    }
}