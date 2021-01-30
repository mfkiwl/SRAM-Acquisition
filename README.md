<h1 align='center'>SRAM Acquisition</h1>

<h3 align='center'>Platform for Acquisition of SRAM-based PUFs from Micro-Controllers</h3>

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

## Getting started

It is recommended to consult the documentation in order to set up the station correctly. In order to compile the code, [meson](https://mesonbuild.com/) and [ninja](https://ninja-build.org/) are needed and to build the documentation, [Doxygen](https://www.doxygen.nl/index.html) and [shpinx](https://www.sphinx-doc.org/en/master/) need to be installed.

In order to set up the build directory:

```
$ git clone https://github.com/servinagrero/SRAM-Acquisition.git
$ meson build && cd build
```

To build the documentation run the next command. The entry point of the documentation is at 'docs/build/index.html'.

```
$ meson compile docs
```

## LICENSE

This project is licensed under the [GPL v3](https://github.com/servinagrero/SRAM-Acquisition/blob/master/LICENSE)
