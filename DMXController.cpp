#include "DMXController.hpp"

DMXController::DMXController(HardwareSerial& serial, uint8_t txPin)
    : dmxTx_(serial)
    , txPin_(txPin) {
    pinMode(txPin_, OUTPUT);
    digitalWriteFast(txPin_, HIGH);
    dmxTx_.begin();
}

void DMXController::addFixture(uint8_t startAddress, uint8_t channelsPerFixture) {
    fixtures_.emplace_back(startAddress, channelsPerFixture);
}

void DMXController::setSymmetricalColor(uint8_t leftIndex, const RGB& color) {
    if (leftIndex >= fixtures_.size() / 2) return;

    uint8_t rightIndex = fixtures_.size() - 1 - leftIndex;
    fixtures_[leftIndex].setColor(color);
    fixtures_[rightIndex].setColor(color);
}

void DMXController::setIndividualColor(uint8_t index) {
    if (index >= fixtures_.size()) return;
    fixtures_[index].setColor();
}

void DMXController::setIndividualColor(uint8_t index, const RGB& color) {
    if (index >= fixtures_.size()) return;
    fixtures_[index].setColor(color);
}

void DMXController::setAllColors(const RGB& color) {
    for (auto& fixture : fixtures_) {
        fixture.setColor(color);
    }
}

void DMXController::update() {
    for (auto& fixture : fixtures_) {
        fixture.update();
    }
    updateDMXValues();
}

void DMXController::updateDMXValues() {
    for (size_t i = 0; i < fixtures_.size(); ++i) {
        RGB color = fixtures_[i].getCurrentColor();
        uint8_t startAddr = 1 + (i * fixtures_[i].channelsPerFixture_);
        uint8_t data[3] = {color.r, color.g, color.b};
        dmxTx_.set(startAddr, data, 3);

        // print to serial monitor
        for (const auto &rgbValue : data) {
            int numDigits = String(rgbValue).length();
            for (int j = 0; j < (maxDigits - numDigits); j++) {
                Serial.print("0");
            }
            Serial.print(rgbValue);
            Serial.print(" ");
        }
    }
    Serial.println();
}