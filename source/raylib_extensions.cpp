#include "raylib_extensions.h"

#include <algorithm>
#include <cmath>

Color operator+(const Color& c1, const Color& c2)
{
    Color c;
    c.r = std::clamp(c1.r + c2.r, 0, 255);
    c.g = std::clamp(c1.g + c2.g, 0, 255);
    c.b = std::clamp(c1.b + c2.b, 0, 255);
    c.a = std::clamp(c1.a + c2.a, 0, 255);
    return c;
};

Color& operator+=(Color& c, const Color& rhs)
{
    c = c + rhs;
    return c;
}

Color operator*(const Color& c, float mul)
{
    Color out;
    out.r = (char)(c.r * mul);
    out.g = (char)(c.g * mul);
    out.b = (char)(c.b * mul);
    out.a = (char)(c.a * mul);
    return c;
}

Color hsv2rgb(int h, float s, float v)
{
    float hp = h / 60.0f;
    float c = s * v;
    float x = c * (float)(1 - std::abs(std::fmod(hp, 2) - 1));
    float m = v - c;
    float r = 0, g = 0, b = 0;
    if (hp <= 1) {
        r = c;
        g = x;
    }
    else if (hp <= 2) {
        r = x;
        g = c;
    }
    else if (hp <= 3) {
        g = c;
        b = x;
    }
    else if (hp <= 4) {
        g = x;
        b = c;
    }
    else if (hp <= 5) {
        r = x;
        b = c;
    }
    else {
        r = c;
        b = x;
    }
    r += m;
    g += m;
    b += m;
    return { (unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255), 255 };
}

std::vector<Color> generateRandomColors(int count)
{
    std::vector<Color> colors;
    float sep = 360.0f / (float)count;
    float h = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        h += sep;
        colors.emplace_back(hsv2rgb((int)std::round(h) % 360, 0.5f, 0.15f));
    }

    return colors;
}