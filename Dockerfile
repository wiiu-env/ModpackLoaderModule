FROM ghcr.io/wiiu-env/devkitppc:20230621

COPY --from=ghcr.io/wiiu-env/libcontentredirection:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:20230719 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libromfs_wiiu:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmodpackloader:20230731 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/librpxloader:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libuwhbutils:20230621 /artifacts $DEVKITPRO

WORKDIR project
