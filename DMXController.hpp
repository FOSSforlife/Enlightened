#pragma once
#include <vector>
#include "DMXFixture.hpp"
#include <TeensyDMX.h>

enum HueSchemaType {
  Static, // A = B = C
  Complementary, // A = C
  Analogous,
  Triadic,
  SplitComplementary
};

class DMXController {
public:
    DMXController(HardwareSerial& serial, uint8_t txPin);

    std::vector<DMXFixture> fixtures_;
    void addFixture(uint8_t startAddress, uint8_t channelsPerFixture = 7);
    void setSymmetricalNoteHit(size_t leftIndex);
    void setSymmetricalNoteHit(size_t leftIndex, const RGB& color);
    void setIndividualNoteHit(size_t index, const RGB& color);
    void setIndividualNoteHit(size_t index);
    void setColorScheme(const RGB& color, HueSchemaType schemaType);
    void update(); // Called every frame

private:
    qindesign::teensydmx::Sender dmxTx_;
    uint8_t txPin_;

    void updateDMXValues();
};