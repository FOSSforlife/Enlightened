#pragma once
#include <cstdint>

struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class DMXFixture {
public:
    DMXFixture(uint8_t startAddress, uint8_t channelsPerFixture = 7);

    void setColor(const RGB& color);
    void setColor();
    void setBrightness(float brightness);
    void setAttack(int frames);
    void setRelease(int frames);
    void update(); // Called every frame

    RGB getCurrentColor() const;
    bool isAnimating() const;
    uint8_t startAddress_;
    uint8_t channelsPerFixture_;

private:
    RGB targetColor_;
    RGB currentColor_;
    float brightness_;
    int attackFrames_;
    int releaseFrames_;
    int fadeInFramesLeft_;
    int fadeOutFramesLeft_;
    RGB attackDelta_;
    RGB releaseDelta_;
};