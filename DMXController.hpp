#pragma once
#include <vector>
#include "DMXFixture.hpp"
#include <TeensyDMX.h>

class DMXController {
public:
    DMXController(HardwareSerial& serial, uint8_t txPin);

    std::vector<DMXFixture> fixtures_;
    void addFixture(uint8_t startAddress, uint8_t channelsPerFixture = 7);
    void setSymmetricalColor(uint8_t leftIndex, const RGB& color);
    void setIndividualColor(uint8_t index, const RGB& color);
    void setIndividualColor(uint8_t index);
    void setAllColors(const RGB& color);
    void update(); // Called every frame

private:
    qindesign::teensydmx::Sender dmxTx_;
    uint8_t txPin_;

    void updateDMXValues();
};