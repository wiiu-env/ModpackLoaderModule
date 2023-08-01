[![CI-Release](https://github.com/wiiu-env/ModpackLoaderModule/actions/workflows/ci.yml/badge.svg)](https://github.com/wiiu-env/ModpackLoaderModule/actions/workflows/ci.yml)

This is a WIP and not finalised. Nothing is documented, nothing is decided, not everything is fully implemented or even fully tested. 
If you need to ask what this is (or how to use it) this is (currently) nothing for you. Use it at your own risk. See `modpacks/example` for a sample modpack.

## Usage

(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `ModpackLoaderModule.wms` into `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
2. Requires the [WUMSLoader](https://github.com/wiiu-env/WUMSLoader)
   in `sd:/wiiu/environments/[ENVIRONMENT]/modules/setup`.
3. Use [libmodpackloader](https://github.com/wiiu-env/libmodpackloader).

## Buildflags

### Logging

Building via `make` only logs errors (via OSReport). To enable logging via
the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).  
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  
`make DEBUG=VERBOSE` Enables verbose information and error logging
via [LoggingModule](https://github.com/wiiu-env/LoggingModule).

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, it'll fallback to UDP (Port 4405)
and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t modpackloadermodule-builder

# make 
docker run -it --rm -v ${PWD}:/project modpackloadermodule-builder make

# make clean
docker run -it --rm -v ${PWD}:/project modpackloadermodule-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./src -i`
