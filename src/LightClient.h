#pragma once

#include "Light.h"
#include "httplib/httplib.h"

#include <string>
#include <unordered_set>

class LightClient
{
public:
    LightClient(const std::string& host, int port); 

    // Get the list of currently existing lights
    bool getLightIds(std::unordered_set<std::string>& ids);

    // Get the complete state of one light
    bool getLight(const std::string& id, Light& light);

private:
    httplib::Client m_client;
};