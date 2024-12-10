#pragma once
#include "DMXController.hpp"

class MIDIHandler {
public:
    MIDIHandler(DMXController& controller);

    void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void handleControlChange(uint8_t channel, uint8_t control, uint8_t value);

private:
    DMXController& controller_;
    static constexpr uint8_t NOTE_COLOR_START = 53;
    static constexpr uint8_t NOTE_SYMMETRICAL_START = 60;
    static constexpr uint8_t NOTE_INDIVIDUAL_START = 65;
};