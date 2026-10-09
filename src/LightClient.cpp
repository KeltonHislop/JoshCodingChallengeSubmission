#include "LightClient.h"
#include "httplib/httplib.h"
#include "nlohmann/json.hpp"

#include <unordered_set>
#include <iostream>
#include <utility>

using json = nlohmann::json;

LightClient::LightClient(const std::string& host, int port)
    : m_client(host, port)
{
    // Explicitly set timeouts
    m_client.set_connection_timeout(3, 0);
    m_client.set_read_timeout(5, 0);
    m_client.set_write_timeout(5, 0);
}

bool LightClient::getLightIds(std::unordered_set<std::string>& ids)
{
    auto result = m_client.Get("/lights");

    // Check for a valid response
    if (!result)
    {
        std::cerr << "Failed to get lights: " << httplib::to_string(result.error()) << '\n';
        return false;
    }
    if (result->status != 200)
    {
        std::cerr << "HTTP error " << result->status << " for lights request" << '\n';
        return false;
    }

    try
    {
        std::unordered_set<std::string> tempIds{};
        // Parse the returned json
        json data = json::parse(result->body);

        // The simulator returns null instead of an empty array when there are no lights
        if (data.is_null())
        {
            ids.clear();
            return true;
        }

        // Make sure the data we received is an array like expected
        if (!data.is_array())
        {
            std::cerr << "Expected array from /lights\n";
            return false;
        }

        // Push each id to the id list
        for (const auto& item : data)
        {
            std::string id = item.at("id").get<std::string>();

            if (id.empty())
            {
                std::cerr << "Empty light ID\n";
                return false;
            }
            if (!tempIds.insert(id).second)
            {
                std::cerr << "Duplicate light ID: " << id << '\n';
                return false;
            }
        }
        
        // If we have a full valid list of IDs, move it to the output value
        ids = std::move(tempIds);
    }
    catch (const json::exception& e)
    {
        std::cerr << "Invalid JSON response: " << e.what() << '\n';
        return false;
    }

    return true;
}

bool LightClient::getLight(const std::string& id, Light& light)
{
    auto lightDetails = m_client.Get("/lights/" + httplib::encode_uri_component(id));

    // Check for a valid response
    if (!lightDetails)
    {
        std::cerr << "Failed to get details for light " << id << ": " << httplib::to_string(lightDetails.error()) << '\n';
        return false;
    }
    if (lightDetails->status != 200)
    {
        std::cerr << "HTTP error " << lightDetails->status << " for light " << id << '\n';
        return false;
    }

    try
    {
        json lightData = json::parse(lightDetails->body);
        
        // Verify the brightness is valid
        const auto& brightness = lightData.at("brightness");
        if (!brightness.is_number_integer() || brightness < 0 || brightness > 255)
        {
            std::cerr << "Invalid brightness for light " << id << '\n';
            return false;
        }

        // NOTE Use .get rather than implicit conversion for proper type checking
        // Set Light values
        Light tempLight{};
        tempLight.id = lightData.at("id").get<std::string>();
        tempLight.name = lightData.at("name").get<std::string>();
        tempLight.room = lightData.at("room").get<std::string>();
        tempLight.on = lightData.at("on").get<bool>();
        tempLight.brightness = lightData.at("brightness").get<int>();

        // Verify matching IDs
        if (tempLight.id != id)
        {
            std::cerr << "Returned ID didn't match the requested ID for " << id << '\n';
            return false;
        }

        // If we have a full valid Light object, move it to the output object
        light = std::move(tempLight);
    }
    catch (const json::exception& e)
    {
        std::cerr << "Invalid JSON response: " << e.what() << '\n';
        return false;
    }

    return true;
}