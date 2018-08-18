#ifndef WORKER_H
#define WORKER_H
#include "SocketStream.hpp"

namespace ryuuk
{
    void worker(SocketStream&& socket);
}

#endif // WORKER_H
