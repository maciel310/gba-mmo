# GBA MMO

This directory contains the GBA ROM implementation.

## Building

Set up Docker, then run the following command to build. Alternatively, if you have devkitpro installed
and configured, you can just run `make`.

```
docker run --rm -v $(pwd):/source -it docker.io/devkitpro/devkitarm make -C /source
```

The project will be built as `gba-mmo_mb.gba`.

