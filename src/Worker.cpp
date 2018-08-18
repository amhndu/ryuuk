#include "Worker.hpp"
#include "HTTP.hpp"

#include <string_view>


namespace ryuuk
{
    namespace
    {
    std::size_t handleRequest(const std::string& request, SocketStream& socket)
    {
        HTTP http;
        HTTP::Result result = http.buildResponse(request);

        // If bytesRead is 0, that means the request is incomplete (or possibly malformed)
        // We thus return here, and wait for it to complete in the next attempt.
        if (result.bytesRead == 0)
            return 0;

        for (auto chunk = result.response->nextChunk(); !chunk.empty();
                  chunk = result.response->nextChunk())
        {
            if (socket.send(chunk) != chunk.size())
            {
                LOG(ERROR) << "couldn't send http response. errno: " << errno << std::endl;
                throw std::runtime_error("send error");
            }
        }

        return result.bytesRead;
    }
    }

    void worker(SocketStream&& sock)
    {
        SocketStream socket(std::move(sock));
        LOG(DEBUG) << "Worker starting up with socket " << socket.getSocketFd() << std::endl;

        std::string request;
        while (true)
        {
            auto [result, reply] = socket.receive();
            request += reply;

            if (request.size() > 4096)   // An arbitrary ceiling
            {
                LOG(INFO) << "Terminating connection assuming client is sending gibberish" << std::endl;
                return;
            }

            switch(result)
            {
                case ReceiveResult::Disconnected:
                    LOG(DEBUG) << "Removing socket " << socket.getSocketFd() << std::endl;
                    return;
                case ReceiveResult::Error:
                    LOG(ERROR) << "Receive error with socket " << socket.getSocketFd() << " and errno " << errno << std::endl;
                    return;
                case ReceiveResult::Success:
                {
                    LOG(DEBUG) << "Received data from " << socket.getSocketFd() << std::endl;
                    std::size_t used;
                    try
                    {
                        do
                            used = handleRequest(request, socket);
                        while (used != 0 && used != request.size());
                        request.erase(0, used);
                    }
                    catch(const std::runtime_error& e)
                    {
                        LOG(DEBUG) << e.what() << std::endl;
                        return;
                    }
                }
            }
        }
    }
}
