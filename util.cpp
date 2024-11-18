#include <array>
#include <iostream>
#include <cmath>
#include <cstdint>


// https://learn.microsoft.com/en-us/cpp/cpp/aliases-and-typedefs-cpp?view=msvc-170
// using Color = std::array<int8_t, 3>;

// // turns a single color into a schema
// // TODO: might want to run this at compile time and just keep an array of 128 different schemas in memory
// std::array<Color, 3> schema(Color initColor) {

// }

constexpr int8_t maxHue = 255;
auto wrapHue = [](int16_t hue) -> int8_t {
    return static_cast<int8_t>((hue + maxHue) % maxHue);
};

// google gemini "c++ hue to rgb"
// void hsvToRgb(float h, float s, float v, float& r, float& g, float& b) {
//     if (s == 0) {
//         r = g = b = v;
//         return;
//     }

//     h /= 60;
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
// }

// https://chatgpt.com/c/673aaa18-fab4-800d-85c3-c2da8d54a95b
void hueToRgb(byte hue, byte& rByte, byte& gByte, byte& bByte) {
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

    rByte = float_to_int(r);
    gByte = float_to_int(g);
    bByte = float_to_int(b);
}


// Create lookup tables at compile time
// https://claude.ai/chat/b9960ef2-319b-4822-9b5f-ca56565b4703
template<std::size_t N>
constexpr auto make_float_to_int_table() {
    std::array<uint8_t, N> table{};
    for (std::size_t i = 0; i < N; ++i) {
        float f = static_cast<float>(i) / (N - 1);
        table[i] = static_cast<uint8_t>(f * 255.0f + 0.5f);
    }
    return table;
}

template<std::size_t N>
constexpr auto make_int_to_float_table() {
    std::array<float, 256> table{};
    for (std::size_t i = 0; i < 256; ++i) {
        table[i] = static_cast<float>(i) / 255.0f;
    }
    return table;
}

// Define lookup tables - adjust precision by changing N
constexpr std::size_t PRECISION = 1024;
constexpr auto FLOAT_TO_INT = make_float_to_int_table<PRECISION>();
constexpr auto INT_TO_FLOAT = make_int_to_float_table<256>();

// Conversion functions
constexpr uint8_t float_to_int(float f) {
    if (f <= 0.0f) return 0;
    if (f >= 1.0f) return 255;
    return FLOAT_TO_INT[static_cast<std::size_t>(f * (PRECISION - 1) + 0.5f)];
}

constexpr float int_to_float(uint8_t i) {
    return INT_TO_FLOAT[i];
}
