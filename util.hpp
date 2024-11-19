#include <array>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <algorithm>

// https://learn.microsoft.com/en-us/cpp/cpp/aliases-and-typedefs-cpp?view=msvc-170
// using Color = std::array<int8_t, 3>;

// // turns a single color into a schema
// // TODO: might want to run this at compile time and just keep an array of 128 different schemas in memory
// std::array<Color, 3> schema(Color initColor) {

float normalize(uint8_t input) {
    return static_cast<float>(input) / 255.0f;
}

uint8_t denormalize(float input) {
    return static_cast<uint8_t>(input * 255.0f);;
}

// https://chatgpt.com/c/673cdd69-56fc-800d-829d-f56f2abdd4c3
void printAlignedArray(uint8_t arr[], size_t size) {
  int maxDigits = 3;

  // Print each number with leading zeros
  for (size_t i = 0; i < size; i++) {
    int numDigits = String(arr[i]).length();
    for (int j = 0; j < (maxDigits - numDigits); j++) {
      Serial.print("0");
    }
    Serial.print(arr[i]);
    if (i < size - 1) {
      Serial.print(" "); // Add space between numbers
    }
  }
  Serial.println(); // Add newline after the row
}


enum Arrangement {
  ABAABA,
  ABBBBA,
  ABCCBA
};

// constexpr int8_t maxHue = 255;
// auto wrapHue = [](int16_t hue) -> int8_t {
//     return static_cast<int8_t>((hue + maxHue) % maxHue);
// };

// https://chatgpt.com/c/673aaa18-fab4-800d-85c3-c2da8d54a95b
void hueToRgbFn(int8_t hue, uint8_t& rByte, uint8_t& gByte, uint8_t& bByte) {
    constexpr float depthPerSection = 256 / 6;
    float h = hue / depthPerSection;
    float r, g, b;
    int i = static_cast<int>(floor(h)) % 6;
    float f = h - i; // fractional part

    float q = 1 - f;
    float t = f;

    switch (i) {
        case 0: r = 1; g = t; b = 0; break;
        case 1: r = q; g = 1; b = 0; break;
        case 2: r = 0; g = 1; b = t; break;
        case 3: r = 0; g = q; b = 1; break;
        case 4: r = t; g = 0; b = 1; break;
        default: r = 1; g = 0; b = q; break;
    }

    rByte = denormalize(r);
    gByte = denormalize(g);
    bByte = denormalize(b);
}

void hsvToRgbFn(int8_t hue, int8_t sat, int8_t val, uint8_t& rByte, uint8_t& gByte, uint8_t& bByte) {
    constexpr float depthPerSection = 256 / 6;
    float h = hue / depthPerSection;
    float s = normalize(sat);
    float v = normalize(val);
    float r, g, b;
    int i = static_cast<int>(floor(h)) % 6;
    float f = h - i; // fractional part

    float p = v * (1 - s);
    float q = v * (1 - s * f);
    float t = v * (1 - s * (1 - f));

    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    rByte = denormalize(r);
    gByte = denormalize(g);
    bByte = denormalize(b);
}

