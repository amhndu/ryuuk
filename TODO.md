# Ryuuk TODO

These are Ryuuk\'s short term and long term goals.

General
-------

* Implement some basic headers.
    - Close connections if Connection: close
    - Date
    - Check Accept parameters and if not satisfiable (ie. anything out the trivial stuff), reply with 406 Not Acceptable
* Implement HEAD method.
* Reply back with HTTP/1.1 instead of currently saying 1.0
* If Connection is keep-alive or just in general, have timeouts and reply with 408 Request Timeout

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
