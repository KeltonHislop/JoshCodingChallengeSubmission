#include "LightMonitor.h"
#include "nlohmann/json.hpp"

#include <cmath>
#include <iostream>
#include <unordered_set>
#include <utility>

using ordered_json = nlohmann::ordered_json;

LightMonitor::LightMonitor(LightClient& client)
    : m_client(client)
{
}


int LightMonitor::brightnessToPercent(int rawBrightness)
{
    return static_cast<int>(std::lround(rawBrightness * 100.0 / 255.0));
}


ordered_json LightMonitor::lightToJson(const Light& light)
{
    return {
        {"name", light.name},
        {"id", light.id},
        {"room", light.room},
        {"on", light.on},
        {"brightness", brightnessToPercent(light.brightness)}
    };
}


bool LightMonitor::initialize()
{
    std::unordered_set<std::string> ids;

    if (!m_client.getLightIds(ids))
    {
        return false;
    }

    // Build the map locally so nothing is stored or printed unless the whole operation is a success
    std::unordered_map<std::string, Light> initialLights;
    ordered_json output = ordered_json::array();

    for (const auto& id : ids)
    {
        Light light{};

        if (!m_client.getLight(id, light))
        {
            return false;
        }

        output.push_back(lightToJson(light));

        initialLights.emplace(id, std::move(light));
    }

    // Copy the completed map and print it
    m_lights = std::move(initialLights);
    std::cout << output.dump(4) << '\n';

    return true;
}


void LightMonitor::poll()
{
    std::unordered_set<std::string> currentIds;

    if (!m_client.getLightIds(currentIds))
    {
        // Failed to get light IDs
        return;
    }

    // Handle new and existing lights
    for (const auto& id : currentIds)
    {
        Light currentLight{};

        if (!m_client.getLight(id, currentLight))
        {
            // Preserve previous state on failure
            continue;
        }

        auto it = m_lights.find(id);

        if (it == m_lights.end())
        {
            // New light, print full state
            std::cout << lightToJson(currentLight).dump(4) << '\n';

            m_lights.emplace(id, std::move(currentLight));
        }
        else
        {
            const Light& previousLight = it->second;

            // Each change is printed as its own object containing only the ID and changed property
            auto printChange = [&id](const char* key, const ordered_json& value)
            {
                ordered_json change = {{"id", id}, {key, value}};
                std::cout << change.dump(4) << '\n';
            };

            if (currentLight.name != previousLight.name)
            {
                printChange("name", currentLight.name);
            }
            if (currentLight.room != previousLight.room)
            {
                printChange("room", currentLight.room);
            }
            if (currentLight.on != previousLight.on)
            {
                printChange("on", currentLight.on);
            }

            // Compare output percentages so raw changes that round the same aren't reported
            const int previousPercent = brightnessToPercent(previousLight.brightness);
            const int currentPercent = brightnessToPercent(currentLight.brightness);
            if (currentPercent != previousPercent)
            {
                printChange("brightness", currentPercent);
            }

            // Update saved state
            it->second = std::move(currentLight);
        }
    }

    // Handle removed lights
    for (auto it = m_lights.begin(); it != m_lights.end(); )
    {
        if (currentIds.find(it->first) == currentIds.end())
        {
            std::cout << it->second.name
                      << " (" << it->first
                      << ") has been removed\n";

            it = m_lights.erase(it);
        }
        else
        {
            ++it;
        }
    }
}