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
        
    ryuuk::Log::get().setLevel(ryuuk::INFO);

    LOG(ryuuk::INFO) << "   Ryuuk - Simple, multi-threaded, C++ webserver" << std::endl
                     << "===================================================" << std::endl;
                     
    LOG(ryuuk::INFO) << "Foo" << std::endl;
    LOG(ryuuk::ERROR) << "Foo" << std::endl;
    LOG(ryuuk::DEBUG) << "Foo" << std::endl;
    
        
    return EXIT_SUCCESS;
}
