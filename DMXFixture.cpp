#include "DMXFixture.hpp"
#include <algorithm>

DMXFixture::DMXFixture(uint8_t startAddress, uint8_t channelsPerFixture)
    : startAddress_(startAddress)
    , channelsPerFixture_(channelsPerFixture)
    , targetColor_({0, 0, 0})
    , currentColor_({0, 0, 0})
    , brightness_(1.0f)
    , attackFrames_(2)
    , releaseFrames_(6)
    , fadeInFramesLeft_(0)
    , fadeOutFramesLeft_(0)
    , attackDelta_({0, 0, 0})
    , releaseDelta_({0, 0, 0}) {}

void DMXFixture::reset() {
  currentColor_ = {0, 0, 0};
}

void DMXFixture::noteHit() {
    // Calculate attack deltas
    attackDelta_.r = targetColor_.r / attackFrames_;
    attackDelta_.g = targetColor_.g / attackFrames_;
    attackDelta_.b = targetColor_.b / attackFrames_;

    // Calculate release deltas
    releaseDelta_.r = targetColor_.r / releaseFrames_;
    releaseDelta_.g = targetColor_.g / releaseFrames_;
    releaseDelta_.b = targetColor_.b / releaseFrames_;

    fadeInFramesLeft_ = attackFrames_;
    fadeOutFramesLeft_ = releaseFrames_ + 1; // add 1 to account for integer division (gives the release more time to go to 0)
}

void DMXFixture::noteHit(const RGB& color) {
    this->setColor(color);

    // Calculate attack deltas
    attackDelta_.r = targetColor_.r / attackFrames_;
    attackDelta_.g = targetColor_.g / attackFrames_;
    attackDelta_.b = targetColor_.b / attackFrames_;

    // Calculate release deltas
    releaseDelta_.r = targetColor_.r / releaseFrames_;
    releaseDelta_.g = targetColor_.g / releaseFrames_;
    releaseDelta_.b = targetColor_.b / releaseFrames_;

    fadeInFramesLeft_ = attackFrames_;
    fadeOutFramesLeft_ = releaseFrames_ + 1;
}

void DMXFixture::setBrightness(float brightness) {
    brightness_ = std::clamp(brightness, 0.0f, 1.0f);
}

void DMXFixture::setColor(const RGB& color) {
    targetColor_ = color;
}

void DMXFixture::setAttack(int frames) {
    attackFrames_ = std::max(1, frames);
}

void DMXFixture::setRelease(int frames) {
    releaseFrames_ = std::max(1, frames);
}

void DMXFixture::update() {
    if (fadeInFramesLeft_ > 0) {
        currentColor_.r = std::min(255, currentColor_.r + attackDelta_.r);
        currentColor_.g = std::min(255, currentColor_.g + attackDelta_.g);
        currentColor_.b = std::min(255, currentColor_.b + attackDelta_.b);
        fadeInFramesLeft_--;
    }
    else if (fadeOutFramesLeft_ > 0) {
        currentColor_.r = std::max(0, currentColor_.r - releaseDelta_.r);
        currentColor_.g = std::max(0, currentColor_.g - releaseDelta_.g);
        currentColor_.b = std::max(0, currentColor_.b - releaseDelta_.b);
        fadeOutFramesLeft_--;
    }
}

RGB DMXFixture::getCurrentColor() const {
    return {
        static_cast<uint8_t>(currentColor_.r * brightness_),
        static_cast<uint8_t>(currentColor_.g * brightness_),
        static_cast<uint8_t>(currentColor_.b * brightness_)
    };
}

bool DMXFixture::isAnimating() const {
    return fadeInFramesLeft_ > 0 || fadeOutFramesLeft_ > 0;
}