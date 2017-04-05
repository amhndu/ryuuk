/**
* Ryuuk
*/


#include <cstdlib>
#include <fstream>

#include "Log.hpp"
#include "Server.hpp"

// If POSIX compliant OS, we bind SIGINT to shutdown the application
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

    #include <signal.h>

    namespace
    {
        ryuuk::Server *serverPtr = nullptr;
    }

#endif

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

    // If POSIX compliant OS, we bind SIGINT to shutdown the application
    #if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

        // Set the global ptr for the handler
        serverPtr = &Ryuuk;

        // struct keyword required to remove ambiguity with the function
        struct sigaction sa;

        // Shutdown server when SIGINT or SIGTERM is received
        sa.sa_handler =
            [](int sig){ if(serverPtr) serverPtr->shutdown(); };

        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

    #endif

    Ryuuk.run();

    }
    catch(const std::exception& e)
    {
        std::cerr << "\nException occurred:" << std::endl;
        std::cerr << e.what() << std::endl;
    }


    return EXIT_SUCCESS;
}
