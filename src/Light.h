#pragma once

#include <string>

struct Light
{
    std::string id;
    std::string name;
    std::string room;
    bool on = false;
    int brightness = 0; // Raw API value: 0-255
};