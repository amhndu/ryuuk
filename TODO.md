# Ryuuk TODO

These are Ryuuk\'s short term and long term goals.

General
-------

* Implement some basic headers.
    - Range (and return Accept-Ranges: bytes instead of the current : none) and If-Range and thus add 206 Partial Content
    - Check Accept parameters and if not satisfiable (ie. anything out the trivial stuff), reply with 406 Not Acceptable
    - If-Match then send 412 Precondition fail, If-None-Match then send it again
* Have timeouts and reply with 408 Request Timeout
* sendResource: What if the resource is big.

Concurrency
-----------

*  Move to Worker Pool model, with each worker handling multiple sockets (using SocketSelector)

Unit testing
------------

* Write custom, well-defined tests

FIXME
-----------

* Create a typedef for int (preferably using 'using X = int') and use that for file/socket descriptor instead of regular int.
  Need fixin' throughout the codebase.
* TODOs spread throughout the code
