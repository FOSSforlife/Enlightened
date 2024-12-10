#include "DMXController.hpp"
#include "MIDIHandler.hpp"
#include <TeensyDMX.h>
#include <TimerOne.h>
// #include "util.hpp"
#include <array>
#include <algorithm>

DMXController controller(Serial1, 17);
MIDIHandler midiHandler(controller);
constexpr uint8_t FRAME_RATE = 60;

void setup() {
    Serial.begin(250000);

    // Initialize controller with 6 fixtures
    for (int i = 0; i < 6; i++) {
        controller.addFixture(1 + (i * 7));
    }

    // Set up MIDI callbacks
    usbMIDI.setHandleControlChange([](uint8_t channel, uint8_t control, uint8_t value) {
        midiHandler.handleControlChange(channel, control, value);
    });

    usbMIDI.setHandleNoteOn([](uint8_t channel, uint8_t note, uint8_t velocity) {
        midiHandler.handleNoteOn(channel, note, velocity);
    });

    // Set up frame timer
    Timer1.initialize(1000000 / FRAME_RATE);
    Timer1.attachInterrupt([]() {
        controller.update();
    });

    Serial.println("Starting program...");
    Serial.printf("Frame rate: %u\n", FRAME_RATE);
}

void loop() {
    usbMIDI.read();
    delay(5);
}