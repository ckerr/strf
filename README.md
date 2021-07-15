# Strf

*Strf* is C++11 formatting library that supports encoding conversion

Branch   | Travis | Appveyor | codecov.io
---------|--------|----------|-----------
develop  | [![Build Status](https://travis-ci.com/robhz786/strf.svg?branch=develop)](https://travis-ci.com/robhz786/strf)| [![Build Status](https://ci.appveyor.com/api/projects/status/github/robhz786/strf?branch=develop&svg=true)](https://ci.appveyor.com/project/robhz786/strf/branch/develop)| [![codecov](https://codecov.io/gh/robhz786/robhz786/branch/develop/graph/badge.svg)](https://codecov.io/gh/robhz786/strf/branch/develop)
master   | [![Build Status](https://travis-ci.com/robhz786/strf.svg?branch=master)](https://travis-ci.com/robhz786/strf)| [![Build Status](https://ci.appveyor.com/api/projects/status/github/robhz786/strf?branch=master&svg=true)](https://ci.appveyor.com/project/robhz786/strf/branch/master)| [![codecov](https://codecov.io/gh/robhz786/robhz786/branch/master/graph/badge.svg)](https://codecov.io/gh/robhz786/strf/branch/master)

[![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/cpp-strf/strf)
Version: 0.14.0

## Documentation

* Overview
  * [Tutorial](http://robhz786.github.io/strf/v0.14.0/tutorial.html)
  * [Quick reference](http://robhz786.github.io/strf/v0.14.0/quick_reference.html)
* How to extend strf:
  * [Adding destination](http://robhz786.github.io/strf/v0.14.0/howto_add_destination.html)
  * [Adding printable types](http://robhz786.github.io/strf/v0.14.0/howto_add_printable_types.html)
  * [Overriding printable types](http://robhz786.github.io/strf/v0.14.0/howto_override_printable_types.html)
* Header references:
  * [`<strf.hpp>`](http://robhz786.github.io/strf/v0.14.0/strf_hpp.html) is the main header. This document is big and covers many details that you will probably never need to know. So it's not the best starting point.
  * [`<strf/outbuff.hpp>`](http://robhz786.github.io/strf/v0.14.0/outbuff_hpp.html) is a lightweight and freestanding header that defines the `basic_outbuff` class template. All other headers depend on this one.
  * [`<strf/to_string.hpp>`](http://robhz786.github.io/strf/v0.14.0/to_string_hpp.html) adds support for writting to `std::basic_string`. It includes `<strf.hpp>`.
  * [`<strf/to_cfile.hpp>`](http://robhz786.github.io/strf/v0.14.0/to_cfile_hpp.html)  adds support for writting to `FILE*`. It includes `<strf.hpp>`.
  * [`<strf/to_streambuf.hpp>`](http://robhz786.github.io/strf/v0.14.0/to_streambuf_hpp.html) adds support for writting to `std::basic_streambuf`. It includes `<strf.hpp>`.
* Miscellaneous
  * [Strf versus {fmt}](http://robhz786.github.io/strf/v0.14.0/versus_fmtlib.html)  ( an incomplete comparison )
  * [How to use strf on CUDA devices](http://robhz786.github.io/strf/v0.14.0/cuda.html)
  * [Benchmarks](http://robhz786.github.io/strf-benchmarks/v0.14.0/results.html)

## Requirements

Strf has been tested in the following compilers:

* Clang 3.8
* GCC 6.5
* Visual Studio 2017 15.8
* NVCC 11.0

