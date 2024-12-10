#include "DMXController.hpp"

RGB hueToColor1(uint8_t hue) {
    // Convert note to color using HSV->RGB conversion

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

// https://claude.ai/chat/56b9cd17-c5b2-4a80-aeed-9ccac40b37d3
uint8_t colorToHue(const RGB& color) {
    // Convert uint8_t to float for calculations
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float delta = max - min;

    // If delta is 0, the color is grayscale (no hue)
    if (delta == 0) {
        return 0;
    }

    float hue = 0.0f;

    // Calculate hue based on which color channel is max
    if (max == r) {
        hue = 60.0f * fmodf(((g - b) / delta), 6.0f);
    } else if (max == g) {
        hue = 60.0f * (((b - r) / delta) + 2.0f);
    } else { // max == b
        hue = 60.0f * (((r - g) / delta) + 4.0f);
    }

    // Ensure hue is positive
    if (hue < 0) {
        hue += 360.0f;
    }

    // Scale hue from 0-360 to 0-255
    return static_cast<uint8_t>(hue * 255.0f / 360.0f);
}

constexpr uint8_t maxHue = 255;
auto wrapHue = [](int16_t hue) -> uint8_t {
    return static_cast<uint8_t>((hue + maxHue) % maxHue);
};

uint8_t complementaryHue(uint8_t hue) {
  return wrapHue(hue + 128);
}

uint8_t analogousHueLeft(uint8_t hue) {
  return wrapHue(hue - 30);
}

uint8_t analogousHueRight(uint8_t hue) {
  return wrapHue(hue + 30);
}

uint8_t triadicHueLeft(uint8_t hue) {
  return wrapHue(hue - 85);
}

uint8_t triadicHueRight(uint8_t hue) {
  return wrapHue(hue + 85);
}

uint8_t splitComplementaryHueLeft(uint8_t hue) {
  return wrapHue(hue - 106);
}

uint8_t splitComplementaryHueRight(uint8_t hue) {
  return wrapHue(hue + 106);
}

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

void DMXController::setSymmetricalNoteHit(size_t leftIndex) {
    if (leftIndex >= fixtures_.size() / 2) {
        Serial.println("Invalid fixture index");
        return;
    };

    uint8_t rightIndex = fixtures_.size() - 1 - leftIndex;
    fixtures_[leftIndex].noteHit();
    fixtures_[rightIndex].noteHit();
}

void DMXController::setSymmetricalNoteHit(size_t leftIndex, const RGB& color) {
    if (leftIndex >= fixtures_.size() / 2) {
        Serial.println("Invalid fixture index");
        return;
    };

    uint8_t rightIndex = fixtures_.size() - 1 - leftIndex;
    fixtures_[leftIndex].noteHit(color);
    fixtures_[rightIndex].noteHit(color);
}

void DMXController::setIndividualNoteHit(size_t index) {
    if (index >= fixtures_.size()) return;
    fixtures_[index].noteHit();
}

void DMXController::setIndividualNoteHit(size_t index, const RGB& color) {
    if (index >= fixtures_.size()) return;
    fixtures_[index].noteHit(color);
}

void DMXController::setColorScheme(const RGB& color, HueSchemaType schemaType) {
    RGB fixtureA = color;
    RGB fixtureB, fixtureC;

    switch(schemaType) {
        case Static:
            fixtureB = color;
            fixtureC = color;
            break;
        case Complementary:
            fixtureB = hueToColor1(complementaryHue(colorToHue(color)));
            fixtureC = color;
            break;
        case Analogous:
            fixtureB = hueToColor1(analogousHueLeft(colorToHue(color)));
            fixtureC = hueToColor1(analogousHueRight(colorToHue(color)));
            break;
        case Triadic:
            fixtureB = hueToColor1(triadicHueLeft(colorToHue(color)));
            fixtureC = hueToColor1(triadicHueRight(colorToHue(color)));
            break;
        case SplitComplementary:
            fixtureB = hueToColor1(splitComplementaryHueLeft(colorToHue(color)));
            fixtureC = hueToColor1(splitComplementaryHueRight(colorToHue(color)));
            break;
    }

    int halfwayPoint = fixtures_.size() / 2;

    for (size_t i = 0; i < halfwayPoint; i++) {
        int whichFixture = i % 3;
        if (whichFixture == 0) {
            fixtures_[i].setColor(fixtureA);
        }
        else if (whichFixture == 1) {
            fixtures_[i].setColor(fixtureB);
        }
        else if (whichFixture == 2) {
            fixtures_[i].setColor(fixtureC);
        }
    }

    for (size_t i = halfwayPoint; i < fixtures_.size(); i++) {
        int whichFixture = i % 3;
        if (whichFixture == 0) {
            fixtures_[i].setColor(fixtureC);
        }
        else if (whichFixture == 1) {
            fixtures_[i].setColor(fixtureB);
        }
        else if (whichFixture == 2) {
            fixtures_[i].setColor(fixtureA);
        }
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
        // int fadeInFramesLeft = fixtures_[i].fadeInFramesLeft_;
        // int fadeOutFramesLeft = fixtures_[i].fadeOutFramesLeft_;
        // RGB releaseDelta = fixtures_[i].releaseDelta_;
        uint8_t startAddr = 1 + (i * fixtures_[i].channelsPerFixture_);
        uint8_t data[3] = {color.r, color.g, color.b};
        // uint8_t data[6] = {color.r, color.g, color.b, fadeInFramesLeft, fadeOutFramesLeft, releaseDelta.r};
        dmxTx_.set(startAddr, data, 3);

        // print to serial monitor
        int maxDigits = 3;
        for (const auto &rgbValue : data) {
            int numDigits = String(rgbValue).length();
            for (int j = 0; j < (maxDigits - numDigits); j++) {
                Serial.print("0");
            }
            Serial.print(rgbValue);
            Serial.print(" ");
        }
        Serial.print(" ");
    }
    Serial.println();
}