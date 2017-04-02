/*
 *  Ryuuk 
 * -------
 *  Ryuuk is an upcoming webserver written by Shinigamis
 *
 * Log - The logging class for Ryuuk
 *
 * TODO: Everything
 *
 */

#ifndef LOG_HPP
#define LOG_HPP


#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <cstring>

#ifndef __FILENAME__
    #define __FILENAME__ __FILE__
#endif

#define LOG(level) \
if (level > ryuuk::Log::get().getLevel()) ; \
else ryuuk::Log::get().getStream() << toErrorString(level) << '[' << __FILENAME__ << ":" << std::dec << __LINE__ << "] "

namespace ryuuk
{
    enum Level
    {
        ERROR ,
        INFO  ,
        DEBUG
    };
    
    static inline std::string toErrorString(Level lvl)
    {
        switch (lvl)
        {
            case ERROR: return "[ ERROR ]";
            case INFO : return "[ INFO  ]";
            case DEBUG: return "[ DEBUG ]";
            default   : return "";
        }
    }
    
    class Log
    {
    public:
    
        ~Log();
        
        void setLogStream(std::ostream& stream);
                
        Log& setLevel(Level level);
        
        Level getLevel();

        std::ostream& getStream();
        
        static Log& get();
        
    private:
        
        Level m_logLevel;
        
        std::ostream* m_logStream;
        
        static std::unique_ptr<Log> m_instance;
    };

    //Courtesy of http://wordaligned.org/articles/cpp-streambufs#toctee-streams
    class TeeBuf : public std::streambuf
    {
        public:
            // Construct a streambuf which tees output to both input
            // streambufs.
            TeeBuf(std::streambuf* sb1, std::streambuf* sb2);
        private:
            // This tee buffer has no buffer. So every character "overflows"
            // and can be put directly into the teed buffers.
            virtual int overflow(int c);
            // Sync both teed buffers.
            virtual int sync();
        private:
            std::streambuf* m_sb1;
            std::streambuf* m_sb2;
    };

    class TeeStream : public std::ostream
    {
        public:
            // Construct an ostream which tees output to the supplied
            // ostreams.
            TeeStream(std::ostream& o1, std::ostream& o2);
        private:
            TeeBuf m_tbuf;
    };

};
#endif // LOG_HPP
