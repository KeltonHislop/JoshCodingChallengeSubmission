#include "LightClient.h"
#include "LightMonitor.h"
#include "cmdparser/cmdparser.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

// Flag used to request shutdown from the signal handler
volatile std::sig_atomic_t running = 1;

void signalHandler(int)
{
    running = 0;
}

int main(int argc, char** argv)
{
    // Parse command line arguments
    cli::Parser parser(argc, argv);
    parser.set_optional<std::string>("n", "host", "localhost", "Hostname or IP address of the light simulator");
    parser.set_optional<int>("p", "port", 8080, "Port of the light simulator");
    parser.run_and_exit_if_error();

    const auto host = parser.get<std::string>("n");
    const auto port = parser.get<int>("p");

    // Verify valid arguments
    if (host.empty())
    {
        std::cerr << "Host must not be empty\n";
        return 1;
    }
    if (port < 1 || port > 65535)
    {
        std::cerr << "Port must be between 1 and 65535\n";
        return 1;
    }

    // Handle Ctrl+C
    std::signal(SIGINT, signalHandler);

    LightClient client(host, port);
    LightMonitor monitor(client);

    const auto pollInterval = std::chrono::seconds(1);

    // Obtain and print the initial state
    while (running && !monitor.initialize())
    {
        std::this_thread::sleep_for(pollInterval);
    }

    // Monitor subsequent changes
    while (running)
    {
        monitor.poll();
        std::this_thread::sleep_for(pollInterval);
    }

    return 0;
}