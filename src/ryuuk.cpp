/**
* Ryuuk
*/


#include <cstdlib>
#include <fstream>

#include "Log.hpp"
#include "Server.hpp"


#include <signal.h>

// Bind the global pointer in an anomyous namespace (thus make it available only in this file)
namespace
{
    ryuuk::Server *serverPtr = nullptr;
}

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO
#endif

int main(int argc, char **argv)
{
    try
    {

    std::ofstream logFile("ryuuk-log.txt", std::ios::out | std::ios::ate);
    ryuuk::TeeStream tee(logFile, std::cout);

    if (logFile.is_open() && logFile.good())
        ryuuk::Log::get().setLogStream(tee);
    else
        ryuuk::Log::get().setLogStream(std::cerr);

    ryuuk::Log::get().setLevel(ryuuk::LOG_LEVEL);

    ryuuk::Log::get().getStream()
              << "---------------------------------------------------" << std::endl
              << "   Ryuuk - Simple, multi-threaded, C++ webserver" << std::endl
              << "---------------------------------------------------" << std::endl
              << "          (Written by true Shinigamis)" << std::endl
              << "===================================================" << std::endl;

    LOG(ryuuk::INFO) << "Logging level set to " << ryuuk::toLevelString(ryuuk::Log::get().getLevel()) << std::endl;

    ryuuk::Server Ryuuk;

    // Set the global ptr for the handler
    serverPtr = &Ryuuk;

    // struct keyword required to remove ambiguity with the function
    struct sigaction sa;

    // Shutdown server when SIGINT or SIGTERM is received
    sa.sa_handler =
    [](int sig){ if(serverPtr) serverPtr->shutdown(); };

    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    // LOG and carry on if SIGPIPE is received
    // That is, some socket was abrutpty closed that we didn't notice and kept trying to writing to.
    struct sigaction sa_pipe;
    sa_pipe.sa_handler = [](int sig) { LOG(ryuuk::ERROR) << "Received SIGPIPE" << std::endl; };
    sigaction(SIGPIPE, &sa_pipe, nullptr);

    Ryuuk.run();

    }
    catch(const std::exception& e)
    {
        std::cerr << "\nException occurred:" << std::endl;
        std::cerr << e.what() << std::endl;
    }


    return EXIT_SUCCESS;
}
