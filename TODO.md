# Ryuuk TODO

These are Ryuuk\'s short term and long term goals.

General
-------

* Put together a simple HTTP server for supporting simple GET requests (WIP)
* Read and handle headers properly
* Read MIME types from config file. (Pass a ref to manifest to the HTTP class ?)
* After a request, properly shutdown the connection before close()ing (ie. removing the object)
* Command line arguments
    - Path to config file
    - Logging level
* POST and PUT methods

Concurrency
-----------

TBD


Unit testing
------------

* Write custom, well-defined tests
* Setup automated build system (e.g. using Travis Ci)

FIXME
-----------

* TODOs spread throughout the code