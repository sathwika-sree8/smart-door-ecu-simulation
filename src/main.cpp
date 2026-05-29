#include <csignal>
#include <iostream>

#include "ecu_app.hpp"

static EcuApp* g_app = nullptr;

static void signalHandler(int signal)
{
    if (signal == SIGINT && g_app != nullptr)
    {
        std::cout << "\nSIGINT received. Requesting ECU stop..." << std::endl;
        g_app->requestStop();
    }
}

int main()
{
    EcuApp app;
    g_app = &app;

    std::signal(SIGINT, signalHandler);

    if (!app.init())
    {
        return 1;
    }

    app.run();
    return 0;
}