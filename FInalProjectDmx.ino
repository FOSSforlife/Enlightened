/*
 * Altered from https://github.com/ssilverman/TeensyDMX/blob/master/examples/BasicSend/BasicSend.ino (c) 2019-2020 Shawn Silverman
 */

#include <TeensyDMX.h>
#include <TimerOne.h>
#include <array>
#include <algorithm>

namespace teensydmx = ::qindesign::teensydmx;

const byte rgbChannels = 3;
const byte fixtures = 6;
const int stateSize = rgbChannels * fixtures;
volatile byte lightState[stateSize];

const byte INPUT_NOTE_SYMMETRICAL_START = 60;
const byte INPUT_NOTE_INDIVIDUAL_START = 65;

// https://chatgpt.com/c/673aa2ae-2398-800d-8360-6eaf929c84d3
// enum NoteRangeType {
//   SymmetricalLights,
//   IndividualLights,
//   NoteRangeTypeSize
// };

enum BinaryHueSchema {
  Complementary,
  AnalogousLeft,
  AnalogousRight,
  TriadicLeft,
  TriadicRight,
  SplitComplementaryLeft,
  SplitComplementaryRight
}

enum TernaryHueSchema {
  Analogous,
  Triadic,
  SplitComplementary
}

// options
byte frameRate = 60;
// int inputNoteRanges[NoteRangeTypeSize] = [60, 66]; // C3, F3

// fixture state
byte hues[fixtures];
float brightness[fixtures];
int attack[fixtures];
int release[fixtures];
int fadeInFramesLeft[stateSize]; // specific to the "currently playing note", written as frame amount
int fadeOutFramesLeft[stateSize]; // specific to the "currently playing note", written as frame amount
byte currentMax[stateSize]; // specific to the "currently playing note", written as RGB value

// Pin for enabling or disabling the transmitter.
// This may not be needed for your hardware.
constexpr byte kTXPin = 17;


// constexpr std::optional<int> findNoteIndex(int target) {
//     // https://claude.ai/chat/e4c684e0-876e-48b3-bdb9-cd2b1b125fbd
//     // constexpr int size = sizeof(inputNotes) / sizeof(inputNotes[0]);

//     for (int i = 0; i < inputNotes.length; i++) {
//       if (inputNotes[i] == target) {
//         return i;
//       }
//     }

//     return std::nullopt;
// }

// Create the DMX sender on Serial1.
teensydmx::Sender dmxTx{Serial1};

// Data for 3 channels.
byte data[3]{0x44, 0x88, 0xcc};

const byte CHANNELS_PER_FIXTURE = 7;

void setup() {
  // https://stackoverflow.com/questions/14446850/filling-up-an-array-in-c
  std::fill(brightness, brightness + fixtures, 1.0);
  std::fill(attack, attack + fixtures, 0);
  std::fill(release, release + fixtures, 15);
  std::fill(currentFadeInRate, currentFadeInRate + stateSize, 0);
  std::fill(currentFadeOutRate, currentFadeOutRate + stateSize, 0);

  Serial.begin(250000);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleNoteOn(OnNoteOn);

  // Turn on the LED, for indicating activity
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWriteFast(LED_BUILTIN, HIGH);

  // Set the pin that enables the transmitter; may not be needed
  pinMode(kTXPin, OUTPUT);
  digitalWriteFast(kTXPin, HIGH);

  // Set some channel values. These are being set in setup() to
  // illustrate that values are 'sticky'. They stay set until changed.
  // There's no special function to call for each iteration of loop().

  // Set channel 1 to 128
  dmxTx.set(1, 128);
  dmxTx.set(2, 0);
  dmxTx.set(7, 255);
  dmxTx.set(1 + CHANNELS_PER_FIXTURE, 128);
  dmxTx.set(2 + CHANNELS_PER_FIXTURE, 0);
  dmxTx.set(3 + CHANNELS_PER_FIXTURE, 128);
  dmxTx.set(7 + CHANNELS_PER_FIXTURE, 255);

  // Set channels 10-12 to the 3 values in 'data'
  // dmxTx.set(10, data, 3);

  // Call this after setting up the channel contents if you want to
  // guarantee that the values are used as the initial contents;
  // transmission doesn't start until after begin() is called.
  // If it doesn't matter, begin() can be called before setting the
  // channel contents.
  dmxTx.begin();

  Timer1.initialize(1000000 / frameRate); // 60 Hz
  Timer1.attachInterrupt(frame);
  Serial.println("Starting program...");
}

void OnControlChange(byte channel, byte control, byte value) {
  Serial.println("got a midi cc");
    // int dmxChannel = (channel -1)*128 + control;
    // led_channel(dmxChannel, value*2);
//  Serial.println("got midi");
//  Serial.println(channel);
 Serial.println(control);
 Serial.println(value);

 dmxTx.set(control, value * 2);
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  Serial.println("got a midi note");
  // Serial.println(channel);
  Serial.println(note);
  Serial.println(velocity);

  if (note >= INPUT_NOTE_SYMMETRICAL_START && note <= INPUT_NOTE_SYMMETRICAL_START + 4) { // C3 through E3
    symmetricalLight(note, velocity);
  }

  if (note >= INPUT_NOTE_INDIVIDUAL_START && note <= INPUT_NOTE_INDIVIDUAL_START + 7) { // F3 through C4
    individualLight(note, velocity);
  }
}

void symmetricalLight(byte note, byte velocity) {
  byte leftFixture;
  if (note == 60) {
    leftFixture = 0;
  }
  if (note == 62) {
    leftFixture = 1;
  }
  if (note == 64) {
    leftFixture = 2;
  }
  byte rightFixture = 5 - leftFixture;

  lightState[leftFixture * rgbChannels]
}

void individualLight(byte note) {

}

int adsr() {

}

void frame()
  int currentFixture = 0;
  int rgbCounter = 0;
  // https://learn.microsoft.com/en-us/cpp/cpp/range-based-for-statement-cpp?view=msvc-170
  for (int i = 0; i < stateSize; i++) {
    if (fadeInFramesLeft[i] != 0) {
      lightState[i]++;
      fadeInFramesLeft[i]--;
    } else if (fadeOutFramesLeft[i] != 0) {
      lightState[i]--;
      fadeOutFramesLeft[i]--;
    }

    rgbCounter++;
    if (rgbCounter % 3 == 0) {
      currentFixture++;
    }
  }

  updateDmx();
}

// TEST: exports lightState to DMX
void updateDmx() {
  int currentFixture = 0;
  int dmxCounter = 0;

  // https://learn.microsoft.com/en-us/cpp/cpp/range-based-for-statement-cpp?view=msvc-170
  for (const auto &rgbChannel : lightState) {
    dmxTx.set(currentFixture * CHANNELS_PER_FIXTURE + dmxCounter, rbgChannel)

    dmxCounter++;
    if (dmxCounter % CHANNELS_PER_FIXTURE == 0) {
      currentFixture++;
    }
  }
}

void loop() {
  // Do something, maybe alter channel values.
  usbMIDI.read();
  delay(5);

  // Serial.print(".");
}