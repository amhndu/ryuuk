# Ryuuk

Ryuuk is a prospective concurrent web-server written in C++.

Currently Ryuuk runs only in POSIX compliant environments only.

## Building

***nix systems**
```
$ git clone https://github.com/amhndu/ryuuk
$ cd ryuuk
$ mkdir build && cd build
$ cmake ..
$ make -ji # where i = no. of cores you can spare
```

*Windows*

1. Nuke your Windows installation
2. Install a Linux flavour
3. Follow the build instructions for *nix systems

## Why

* Becuase why not ?
* The original authors wanted to know a thing or two about sockets & HTTP


## Keikaku


#### Short term

* Basic HTTP 1.1 GET requests
* A Functional web server


#### Long term

* Most of HTTP/1.1
* World domination

For a more detailed keikaku, please see the [TODO](TODO.md)

## References

* https://tools.ietf.org/html/rfc723<0-7>
* https://tools.ietf.org/html/rfc2616
* http://www2005.org/cdrom/docs/p730.pdf
