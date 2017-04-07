# Ryuuk TODO

These are Ryuuk\'s short term and long term goals.

General
-------

* Put together a simple HTTP server for supporting simple GET requests
    1. Design a high-level, abstraction of a TCP socket (WIP)
    2. Design a high-level, abstraction of a TCP listener (WIP)
    3. Design a high-level, abstraction of a TCP selector

* Access restriction (to sensitive files & directories)
* After a request, properly shutdown the connection before close()ing (ie. removing the object)

Concurrency
-----------

TBD


Unit testing
------------

* Write custom, well-defined tests
* Setup automated build system (e.g. using Travis Ci)

FIXME
-----------

* Close socket fds