/**
* Ryuuk
*/


#include <cstdlib>
#include <fstream>
#include <vector>
#include <algorithm>

#include "Log.hpp"
#include "Server.hpp"

#include <signal.h>

// Set the default log level to INFO
#ifndef LOG_LEVEL
    #define LOG_LEVEL INFO
#endif

// Bind the global pointer in an anomyous namespace (thus make it available only in this file)
namespace
{
    ryuuk::Server *serverPtr = nullptr;
}

void printHelp()
{
    std::cout << "Ryuuk - Simple, lightweight, concurrent C++ webserver\n" << std::endl;
    std::cout << "Usage: ryuuk [OPTION-1] [VALUE-1] [OPTION-2] [VALUE-2] ... [OPTION-N] [VALUE-N]\n" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << " -h                      : Display this help message and exit\n" << std::endl;
    std::cout << " -cfg [PATH-TO-FIEL]     : Specifiy a config file\n" << std::endl;
    std::cout << " -lvl [LOG-LEVEL]        : Specifiy a log level\n" << std::endl;
    std::cout << " -log [PATH-TO-LOG-FILE] : Specifiy a log file" << std::endl;
}

struct Options
{
    bool exit               = false;
    int exit_code           = EXIT_SUCCESS;
    std::string logfile     = "ryuuk-log.txt";
    std::string configfile  = "";
    ryuuk::Level loglevel   = ryuuk::LOG_LEVEL;
};

//
// Few rules:
//  * -h takes the utmost precendence. No matter how many
//    options you pass, if the list has -h, then printHelp()
//    is called, followed by exit();
//  * Any no. of supported options can be set as long as
//    they obey the format as prescribed in printHelp()
//
Options applyCommandLineOptions(int _argc, char** _argv)
{
    std::vector<std::string> arguments {_argv, _argv + _argc};
    Options options;

    for (int i = 0; !options.exit && i < arguments.size(); )
    {
        // Get the i-th option, check if it needs a value.
        // If it does, then the subsequent option is the value.
        // If there exist no subsequent option, then current option
        // is ignored silently.

        if (arguments[i] == "-h")
        {
            printHelp();
            options.exit = true;
            options.exit_code = EXIT_SUCCESS;
        }

        if (arguments[i] == "-cfg")
        {
            if (i == arguments.size() - 1)
            {
                std::cerr << "Invalid usage!\nryuuk -h for help and detailed usage." << std::endl;
                options.exit = true;
                options.exit_code = EXIT_FAILURE;
            }
            else
            {
                options.configfile = arguments[i + 1];
                i += 2;
            }
        }

        else if (arguments[i] == "-lvl")
        {
            if (i == arguments.size() - 1)
            {
                std::cerr << "Invalid usage!\nryuuk -h for help and detailed usage." << std::endl;
                options.exit = true;
                options.exit_code = EXIT_FAILURE;
                break;
            }

            if (arguments[i + 1] == "ERROR")
                options.loglevel = ryuuk::Level::ERROR;

            else if (arguments[i + 1] == "INFO")
                options.loglevel = ryuuk::Level::INFO;

            else if (arguments[i + 1] == "DEBUG")
                options.loglevel = ryuuk::Level::DEBUG;

            i += 2;
        }

        else if (arguments[i] == "-log")
        {
            //if (i + 1 == _argc)
            if (i == arguments.size() - 1)
            {
                std::cerr << "Invalid usage!\nryuuk -h for help and detailed usage." << std::endl;
                options.exit = true;
                options.exit_code = EXIT_FAILURE;
            }

            options.logfile = arguments[i + 1];
            i += 2;
        }

        else
            i++;
    }

    return options;
}

int main(int argc, char **argv)
{
    try
    {

    Options opts = applyCommandLineOptions(argc, argv);

    if (opts.exit)
        return opts.exit_code;

    std::ofstream logFile(opts.logfile, std::ios::out | std::ios::ate);
    ryuuk::TeeStream tee(logFile, std::cout);

    if (logFile.is_open() && logFile.good())
        ryuuk::Log::get().setLogStream(tee);
    else
        ryuuk::Log::get().setLogStream(std::cerr);

    ryuuk::Log::get().setLevel(opts.loglevel);

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

    if (!opts.configfile.empty())
        serverPtr->setConfigFile(opts.configfile);

    serverPtr->init();

    // struct keyword required to remove ambiguity with the function
    struct sigaction sa;

    // Shutdown server when SIGINT or SIGTERM is received
    sa.sa_handler =
    [](int sig){ LOG(ryuuk::INFO) << "Received signal SIGTERM." << std::endl; if(serverPtr) serverPtr->shutdown(); };

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
