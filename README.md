[![MapLibre Logo](https://maplibre.org/img/maplibre-logo-big.svg)](https://maplibre.org/)

## mvt-cpp

[![](https://img.shields.io/badge/Slack-%23maplibre--native-2EB67D?logo=slack)](https://slack.openstreetmap.us/)

C++14 library for decoding [Mapbox Vector Tiles](https://www.mapbox.com/vector-tiles/).

This is a dependency of [MapLibre Native](https://github.com/maplibre/maplibre-native).

Forked from [`mapbox/vector-tile`](https://github.com/mapbox/vector-tile).

## Depends

 - C++14 compiler
 - [protozero](https://github.com/mapbox/protozero)
 - [variant](https://github.com/mapbox/variant)
 - [geometry](https://github.com/mapbox/geometry.hpp)


## Building

Call
```sh
git submodule init
git submodule update
```

to install test fixtures from an external git repository.

To install all dependencies and build the tests in this repo do:

```sh
make test
```

## To bundle the `demo` program do:

```sh
make demo
```

This copies all the includes into the `demo/include` folder such that the demo can be build like:

```sh
make -C demo/
```

Or also like:

```sh
cd demo
make
```
