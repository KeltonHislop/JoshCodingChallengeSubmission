#pragma once

#include "Light.h"
#include "LightClient.h"
#include "nlohmann/json.hpp"

#include <string>
#include <unordered_map>

class LightMonitor
{
public:
    explicit LightMonitor(LightClient& client);

    bool initialize();
    void poll();

private:
    static int brightnessToPercent(int rawBrightness);
    static nlohmann::ordered_json lightToJson(const Light& light);

    LightClient& m_client;

    std::unordered_map<std::string, Light> m_lights;
};