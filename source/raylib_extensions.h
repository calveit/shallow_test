#pragma once
#include <vector>

#include "raylib.h"


Color operator+(const Color& c1, const Color& c2);
Color& operator+=(Color& c, const Color& rhs);
Color operator*(const Color& c, float mul);
Color operator*(const Color& c1, const Color& c2);

Color hsv2rgb(int h, float s, float v);
std::vector<Color> generateRandomColors(int count);