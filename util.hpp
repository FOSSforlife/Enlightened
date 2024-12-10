// #include <array>
// #include <iostream>
// #include <cmath>
// #include <cstdint>
// #include <algorithm>

// struct RGB {
//     uint8_t r;
//     uint8_t g;
//     uint8_t b;
// };

// enum HueSchemaType {
//   Static, // A = B = C
//   Complementary, // A = C
//   Analogous,
//   Triadic,
//   SplitComplementary
// };

// RGB hueToColor(uint8_t hue) {
//     // Convert note to color using HSV->RGB conversion

//     // Simple HSV to RGB conversion for full saturation and value
//     uint8_t region = hue / 43;
//     uint8_t remainder = (hue - (region * 43)) * 6;

//     uint8_t p = 0;
//     uint8_t q = 0;
//     uint8_t t = 0;

//     switch (region) {
//         case 0: return {255, remainder, p};
//         case 1: return {255 - remainder, 255, p};
//         case 2: return {p, 255, remainder};
//         case 3: return {p, 255 - remainder, 255};
//         case 4: return {remainder, p, 255};
//         case 5: default: return {255, p, 255 - remainder};
//     }
// }

// RGB velocityToColor(uint8_t velocity) {
//     return hueToColor((velocity * 2) % 256);
// }

// constexpr uint8_t maxHue = 255;
// auto wrapHue = [](int16_t hue) -> uint8_t {
//     return static_cast<uint8_t>((hue + maxHue) % maxHue);
// };

// uint8_t complementaryHue(uint8_t hue) {
//   return wrapHue(hue + 128);
// }

// uint8_t analogousHueLeft(uint8_t hue) {
//   return wrapHue(hue - 30);
// }

// uint8_t analogousHueRight(uint8_t hue) {
//   return wrapHue(hue + 30);
// }

// uint8_t triadicHueLeft(uint8_t hue) {
//   return wrapHue(hue - 85);
// }

// uint8_t triadicHueRight(uint8_t hue) {
//   return wrapHue(hue + 85);
// }

// uint8_t splitComplementaryHueLeft(uint8_t hue) {
//   return wrapHue(hue - 106);
// }

// uint8_t splitComplementaryHueRight(uint8_t hue) {
//   return wrapHue(hue + 106);
// }

// // https://claude.ai/chat/56b9cd17-c5b2-4a80-aeed-9ccac40b37d3
// uint8_t colorToHue(const RGB& color) {
//     // Convert uint8_t to float for calculations
//     float r = color.r / 255.0f;
//     float g = color.g / 255.0f;
//     float b = color.b / 255.0f;

//     float max = std::max({r, g, b});
//     float min = std::min({r, g, b});
//     float delta = max - min;

//     // If delta is 0, the color is grayscale (no hue)
//     if (delta == 0) {
//         return 0;
//     }

//     float hue = 0.0f;

//     // Calculate hue based on which color channel is max
//     if (max == r) {
//         hue = 60.0f * fmodf(((g - b) / delta), 6.0f);
//     } else if (max == g) {
//         hue = 60.0f * (((b - r) / delta) + 2.0f);
//     } else { // max == b
//         hue = 60.0f * (((r - g) / delta) + 4.0f);
//     }

//     // Ensure hue is positive
//     if (hue < 0) {
//         hue += 360.0f;
//     }

//     // Scale hue from 0-360 to 0-255
//     return static_cast<uint8_t>(hue * 255.0f / 360.0f);
// }

// // https://learn.microsoft.com/en-us/cpp/cpp/aliases-and-typedefs-cpp?view=msvc-170
// // using Color = std::array<int8_t, 3>;

// // // turns a single color into a schema
// // std::array<Color, 3> schema(Color initColor) {

// // go with ABCCBA
// // enum Arrangement {
// //   ABAABA,
// //   ABBBBA,
// //   ABCCBA
// // };

// // enum BinaryHueSchema {
// //   Complementary,
// //   AnalogousLeft,
// //   AnalogousRight,
// //   TriadicLeft,
// //   TriadicRight,
// //   SplitComplementaryLeft,
// //   SplitComplementaryRight
// // };

// float normalize(uint8_t input) {
//     return static_cast<float>(input) / 255.0f;
// }

// uint8_t denormalize(float input) {
//     return static_cast<uint8_t>(input * 255.0f);;
// }

// // https://chatgpt.com/c/673cdd69-56fc-800d-829d-f56f2abdd4c3
// void printAlignedArray(uint8_t arr[], size_t size) {
//   int maxDigits = 3;

//   // Print each number with leading zeros
//   for (size_t i = 0; i < size; i++) {
//     int numDigits = String(arr[i]).length();
//     for (int j = 0; j < (maxDigits - numDigits); j++) {
//       Serial.print("0");
//     }
//     Serial.print(arr[i]);
//     if (i < size - 1) {
//       Serial.print(" "); // Add space between numbers
//     }
//   }
//   Serial.println(); // Add newline after the row
// }

// // https://chatgpt.com/c/673aaa18-fab4-800d-85c3-c2da8d54a95b
// void hueToRgbFn(uint8_t hue, uint8_t& rByte, uint8_t& gByte, uint8_t& bByte) {
//     constexpr float depthPerSection = 256 / 6;
//     float h = hue / depthPerSection;
//     float r, g, b;
//     int i = static_cast<int>(floor(h)) % 6;
//     float f = h - i; // fractional part

//     float q = 1 - f;
//     float t = f;

//     switch (i) {
//         case 0: r = 1; g = t; b = 0; break;
//         case 1: r = q; g = 1; b = 0; break;
//         case 2: r = 0; g = 1; b = t; break;
//         case 3: r = 0; g = q; b = 1; break;
//         case 4: r = t; g = 0; b = 1; break;
//         default: r = 1; g = 0; b = q; break;
//     }

//     rByte = denormalize(r);
//     gByte = denormalize(g);
//     bByte = denormalize(b);
// }

// void hsvToRgbFn(uint8_t hue, uint8_t sat, uint8_t val, uint8_t& rByte, uint8_t& gByte, uint8_t& bByte) {
//     constexpr float depthPerSection = 256 / 6;
//     float h = hue / depthPerSection;
//     float s = normalize(sat);
//     float v = normalize(val);
//     float r, g, b;
//     int i = static_cast<int>(floor(h)) % 6;
//     float f = h - i; // fractional part

//     float p = v * (1 - s);
//     float q = v * (1 - s * f);
//     float t = v * (1 - s * (1 - f));

//     switch (i) {
//         case 0: r = v; g = t; b = p; break;
//         case 1: r = q; g = v; b = p; break;
//         case 2: r = p; g = v; b = t; break;
//         case 3: r = p; g = q; b = v; break;
//         case 4: r = t; g = p; b = v; break;
//         default: r = v; g = p; b = q; break;
//     }

//     rByte = denormalize(r);
//     gByte = denormalize(g);
//     bByte = denormalize(b);
// }

