/*
 * Altered from https://github.com/ssilverman/TeensyDMX/blob/master/examples/BasicSend/BasicSend.ino (c) 2019-2020 Shawn Silverman
 */

#include <TeensyDMX.h>
#include <TimerOne.h>
#include "util.hpp"
#include <array>
#include <algorithm>

namespace teensydmx = ::qindesign::teensydmx;

// extern constexpr float int_to_float(uint8_t i);
// extern constexpr uint8_t float_to_int(float f);
void printAlignedArray(int arr[], size_t size);
const byte rgbChannels = 3;
const byte fixtures = 6;
const int stateSize = rgbChannels * fixtures;
byte lightState[stateSize];

const byte INPUT_NOTE_FIXTURE_HUES_START = 36;
const byte INPUT_NOTE_COLOR_SCHEME_START = 48;
const byte INPUT_NOTE_SYMMETRICAL_START = 60;
const byte INPUT_NOTE_INDIVIDUAL_START = 65;
const bool DEBUG_DMX = true;

enum BinaryHueSchema {
  Complementary,
  AnalogousLeft,
  AnalogousRight,
  TriadicLeft,
  TriadicRight,
  SplitComplementaryLeft,
  SplitComplementaryRight
};

enum TernaryHueSchema {
  Analogous,
  Triadic,
  SplitComplementary
};

// options
byte frameRate = 60;
// int inputNoteRanges[NoteRangeTypeSize] = [60, 66]; // C3, F3

// fixture state
byte maxRgb[stateSize];
float brightness[fixtures];
int attack[fixtures]; // written as frame amount
int release[fixtures]; // written as frame amount
int fadeInFramesLeft[fixtures]; // specific to the "currently playing note"
int fadeOutFramesLeft[fixtures]; // specific to the "currently playing note"
int attackDelta[stateSize]; // specific to the "currently playing note"
int releaseDelta[stateSize]; // specific to the "currently playing note"

// Pin for enabling or disabling the transmitter.
// This may not be needed for your hardware.
constexpr byte kTXPin = 17;

// Create the DMX sender on Serial1.
teensydmx::Sender dmxTx{Serial1};

// Data for 3 channels.
byte data[3]{0x44, 0x88, 0xcc};

const byte CHANNELS_PER_FIXTURE = 7;

void setup() {
  // https://stackoverflow.com/questions/14446850/filling-up-an-array-in-c
  std::fill(brightness, brightness + fixtures, 1.0);
  std::fill(attack, attack + fixtures, 15);
  std::fill(release, release + fixtures, 15);
  std::fill(fadeInFramesLeft, fadeInFramesLeft + fixtures, 0);
  std::fill(fadeOutFramesLeft, fadeOutFramesLeft + fixtures, 0);
  std::fill(attackDelta, attackDelta + stateSize, 0);
  std::fill(releaseDelta, releaseDelta + stateSize, 0);

  Serial.begin(250000);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleNoteOn(OnNoteOn);

  // Turn on the LED, for indicating activity
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWriteFast(LED_BUILTIN, HIGH);

  // Set the pin that enables the transmitter; may not be needed
  pinMode(kTXPin, OUTPUT);
  digitalWriteFast(kTXPin, HIGH);

  dmxTx.set(7, 255);
  dmxTx.set(7 + CHANNELS_PER_FIXTURE, 255);

  // Set channels 10-12 to the 3 values in 'data'
  // dmxTx.set(10, data, 3);

  // Call this after setting up the channel contents if you want to
  // guarantee that the values are used as the initial contents;
  // transmission doesn't start until after begin() is called.
  // If it doesn't matter, begin() can be called before setting the
  // channel contents.
  dmxTx.begin();
  setFixtureColors(0);

  Timer1.initialize(1000000 / frameRate); // 60 Hz
  Timer1.attachInterrupt(frame);
  Serial.println("Starting program...");
  Serial.printf("Frame rate: %u\n", frameRate);
}

void setFixtureColors(byte hue) {
  // for now, set all to the same color
  for (int i = 0; i < stateSize; i += 3) {
    // hueToRgbFn(hue, maxRgb[i], maxRgb[i + 1], maxRgb[i + 2]);
    hsvToRgbFn(hue, 255, 255, maxRgb[i], maxRgb[i + 1], maxRgb[i + 2]);
  }
}

void OnControlChange(byte channel, byte control, byte value) {
  Serial.println("got a midi cc");
  Serial.println(control);
  Serial.println(value);

  // dmxTx.set(control, value * 2);
  // setFixtureColors(value * 2);
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  Serial.println("got a midi note");
  // Serial.println(channel);
  Serial.println(note);
  // Serial.println(velocity);

  if (note == INPUT_NOTE_COLOR_START) { // F2
    setFixtureColors(0);
  }
  if (note == INPUT_NOTE_COLOR_START + 2) { // G2
    setFixtureColors(60);
  }
  if (note == INPUT_NOTE_COLOR_START + 4) { // A2
    setFixtureColors(120);
  }
  if (note == INPUT_NOTE_COLOR_START + 6) { // B2
    setFixtureColors(240);
  }

  if (note >= INPUT_NOTE_SYMMETRICAL_START && note <= INPUT_NOTE_SYMMETRICAL_START + 4) { // C3 through E3
    symmetricalLight(note, velocity);
  }

  if (note >= INPUT_NOTE_INDIVIDUAL_START && note <= INPUT_NOTE_INDIVIDUAL_START + 7) { // F3 through C4
    individualLight(note, velocity);
  }
}

void symmetricalLight(byte note, byte velocity) {
  byte leftFixture = 0;
  if (note == 60 || note == 61) {
    leftFixture = 0;
  }
  else if (note == 62 || note == 63) {
    leftFixture = 1;
  }
  else if (note == 64) {
    leftFixture = 2;
  }
  byte rightFixture = fixtures - 1 - leftFixture;

  // set max values
  byte currentMaxLeft[] = {
    byte(maxRgb[leftFixture * rgbChannels] * normalize(velocity * 2)),
    byte(maxRgb[leftFixture * rgbChannels + 1] * normalize(velocity * 2)),
    byte(maxRgb[leftFixture * rgbChannels + 2] * normalize(velocity * 2))
  };
  // set value to increment with each frame
  attackDelta[leftFixture * rgbChannels] = currentMaxLeft[0] / attack[leftFixture];
  attackDelta[leftFixture * rgbChannels + 1] = currentMaxLeft[1] / attack[leftFixture];
  attackDelta[leftFixture * rgbChannels + 2] = currentMaxLeft[2] / attack[leftFixture];

  // set value to decrement with each frame
  releaseDelta[leftFixture * rgbChannels] = currentMaxLeft[0] / release[leftFixture];
  releaseDelta[leftFixture * rgbChannels + 1] = currentMaxLeft[1] / release[leftFixture];
  releaseDelta[leftFixture * rgbChannels + 2] = currentMaxLeft[2] / release[leftFixture];

  // set amount of frames left until max value
  fadeInFramesLeft[leftFixture] = attack[leftFixture];

  // set amount of frames from max value to 0
  fadeOutFramesLeft[leftFixture] = release[leftFixture];

  byte currentMaxRight[] = {
    byte(maxRgb[rightFixture * rgbChannels] * normalize(velocity * 2)),
    byte(maxRgb[rightFixture * rgbChannels + 1] * normalize(velocity * 2)),
    byte(maxRgb[rightFixture * rgbChannels + 2] * normalize(velocity * 2))
  };
  attackDelta[rightFixture * rgbChannels] = currentMaxRight[0] / attack[rightFixture];
  attackDelta[rightFixture * rgbChannels + 1] = currentMaxRight[1] / attack[rightFixture];
  attackDelta[rightFixture * rgbChannels + 2] = currentMaxRight[2] / attack[rightFixture];
  releaseDelta[rightFixture * rgbChannels] = currentMaxRight[0] / release[rightFixture];
  releaseDelta[rightFixture * rgbChannels + 1] = currentMaxRight[1] / release[rightFixture];
  releaseDelta[rightFixture * rgbChannels + 2] = currentMaxRight[2] / release[rightFixture];
  fadeInFramesLeft[rightFixture] = attack[rightFixture];
  fadeOutFramesLeft[rightFixture] = release[rightFixture];
}

void individualLight(byte note, byte velocity) {
  byte fixture = note - 65;

  byte currentMax[] = {
    byte(maxRgb[fixture * rgbChannels] * normalize(velocity * 2)),
    byte(maxRgb[fixture * rgbChannels + 1] * normalize(velocity * 2)),
    byte(maxRgb[fixture * rgbChannels + 2] * normalize(velocity * 2))
  };
  attackDelta[fixture * rgbChannels] = currentMax[0] / attack[fixture * rgbChannels];
  attackDelta[fixture * rgbChannels + 1] = currentMax[1] / attack[fixture * rgbChannels + 1];
  attackDelta[fixture * rgbChannels + 2] = currentMax[2] / attack[fixture * rgbChannels + 2];
  releaseDelta[fixture * rgbChannels] = currentMax[0] / release[fixture * rgbChannels];
  releaseDelta[fixture * rgbChannels + 1] = currentMax[1] / release[fixture * rgbChannels + 1];
  releaseDelta[fixture * rgbChannels + 2] = currentMax[2] / release[fixture * rgbChannels + 2];
  fadeInFramesLeft[fixture] = attack[fixture];
  fadeOutFramesLeft[fixture] = release[fixture];
}

void frame() {
  int currentFixture = 0;
  int rgbCounter = 0;
  for (int i = 0; i < stateSize; i++) {
    if (fadeInFramesLeft[i / rgbChannels] != 0) {
      lightState[i] += attackDelta[i];
      fadeInFramesLeft[i / rgbChannels]--;
    } else if (fadeOutFramesLeft[i / rgbChannels] != 0) {
      lightState[i] -= releaseDelta[i];
      fadeOutFramesLeft[i / rgbChannels]--;
    }

    rgbCounter++;
    if (rgbCounter % 3 == 0) {
      currentFixture++;
    }
  }

  updateDmx();
}

void updateDmx() {
  if (DEBUG_DMX) {
    Serial.print("Sending DMX values: ");
    printAlignedArray(lightState, sizeof(lightState) / sizeof(lightState[0]));
    // printAlignedArray(maxRgb, sizeof(maxRgb) / sizeof(maxRgb[0]));
  }

  for (int i = 0; i < fixtures; i++) {
    byte data[3]{lightState[i * rgbChannels], lightState[i * rgbChannels + 1], lightState[i * rgbChannels + 2]};
    dmxTx.set(1 + (i * CHANNELS_PER_FIXTURE), data, 3);

    // other implementations:

    // dmxTx.set(i * CHANNELS_PER_FIXTURE, [lightState[i * rgbChannels], lightState[i * rgbChannels + 1], lightState[i * rgbChannels + 2]]);

    // dmxTx.set(i * CHANNELS_PER_FIXTURE, lightState[i * rgbChannels]);
    // dmxTx.set(i * CHANNELS_PER_FIXTURE + 1, lightState[i * rgbChannels + 1]);
    // dmxTx.set(i * CHANNELS_PER_FIXTURE + 2, lightState[i * rgbChannels + 2]);

    // int currentFixture = i / rgbChannels;
    // dmxTx.set(currentFixture * CHANNELS_PER_FIXTURE, lightState[i]);
    // dmxTx.set(currentFixture * CHANNELS_PER_FIXTURE + 1, lightState[i + 1]);
    // dmxTx.set(currentFixture * CHANNELS_PER_FIXTURE + 2, lightState[i + 2]);
  }

  // for (const auto &rgbValue : lightState) {
  //   // 0 0, 0 1, 0 2, 0 3 -> 1 0
  //   // dmxTx.set(currentFixture * CHANNELS_PER_FIXTURE + rgbCounter, rgbValue);

  //   rgbCounter++;
  //   if (rgbCounter == 3) {
  //     currentFixture++;
  //     rgbCounter = 0;
  //   }
  // }
}


void loop() {
  // Do something, maybe alter channel values.
  usbMIDI.read();
  delay(5);

  // Serial.print(".");
}