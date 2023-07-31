# Example modpack

This is a WIP and not finalized. Nothing is documented, nothing is decided, not everything is fully implemented or
even fully tested. If you need to ask what this is how or to use it this is (currently) nothing for your. Use at your
own risk.

Replaces files of SSBU EUR (00050000-10145000). You can test it by putting any .mp4
in `content/modpack/movie/crs-f_mov_01_OP_EN.mp4` and then launch the resulting `.wuhb`.

Requires the [ModpackLoaderModule](https://github.com/wiiu-env/ModpackLoaderModule).

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t example_modpack-builder

# make 
docker run -it --rm -v ${PWD}:/project example_modpack-builder make

# make clean
docker run -it --rm -v ${PWD}:/project example_modpack-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./src -i`