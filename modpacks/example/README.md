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