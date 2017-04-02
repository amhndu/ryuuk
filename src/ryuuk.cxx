#include <cstdlib>
#include <fstream>

#include "../include/Log.hpp"

int main(int argc, char **argv)
{
    std::ofstream logFile("ryuuk-log.txt", std::ios::out | std::ios::ate);
    ryuuk::TeeStream tee(logFile, std::cout);
    
    //ryuuk::Log::get();
    if (logFile.is_open() && logFile.good())
        ryuuk::Log::get().setLogStream(tee);
    else
        ryuuk::Log::get().setLogStream(std::cout);
        
    ryuuk::Log::get().setLevel(ryuuk::DEBUG);

    std::cout << "---------------------------------------------------" << std::endl
              << "   Ryuuk - Simple, multi-threaded, C++ webserver" << std::endl
              << "---------------------------------------------------" << std::endl
              << "          (Written by true Shinigamis)" << std::endl
              << "===================================================" << std::endl;
                     
    LOG(ryuuk::DEBUG) << "Foo Debug" << std::endl;
    LOG(ryuuk::INFO) << "Foo Info" << std::endl;
    LOG(ryuuk::ERROR) << "Foo Error" << std::endl;
    LOG(ryuuk::DEBUG) << "Foo Debug" << std::endl;
    
        
    return EXIT_SUCCESS;
}
