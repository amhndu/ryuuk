# Ryuuk TODO

These are Ryuuk\'s short term and long term goals.

General
-------

* Put together a simple HTTP server for supporting simple GET requests (WIP)
* Read and handle headers properly
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

* Better handling a socket spamming a lot of data ? (simulate by nc localhost 8000 < /dev/random)
  Right now, it's sort of poorly handled (?)
  
  So, do something like this ?
  ```
  do
    (received', buffer') = socket.receive()
    if received + received' < MAX_BUFFER_CAPACITY
        received += received'
        buffer += buffer'
  while received' == socket.getBufferSize() && (?? if it's end of transmission ??)
  ```
  So this will probably not work. To figure out if it's end of transmission of an HTTP request, we need to read the data.
  Possibly, augment the HTTP class to tell us if it needs more data ?
  This needs more thought as it'll require changes to how things are passed between the server class and http.
  
  
* TODOs spread throughout the code
