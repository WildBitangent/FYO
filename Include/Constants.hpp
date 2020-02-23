#pragma once

constexpr auto WIDTH = 1280u; // must be divisible by 32
constexpr auto HEIGHT = 720u; // must be divisible by 8

constexpr auto WIDTHF = static_cast<float>(WIDTH);
constexpr auto HEIGHTF = static_cast<float>(HEIGHT);

constexpr auto FOV = 60.f;
