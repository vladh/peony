# peony

![peony screenshot](https://vladh.net/static/peony@1000px.jpg)

Peony is a 3D game engine written from scratch in C++. It's currently a work
in progress.

If you'd like to read the code, you can start at `src/main.cpp`.

There is more information about Peony and its features in these blog posts:

* [Game Engine: How I Implemented Skeletal
  Animation](https://vladh.net/articles/game-engine-skeletal-animation.html)

## Dependencies

Peony requires a GPU with support for at least OpenGL 4.1.

* [glfw](https://github.com/glfw/glfw)
* [freetype](https://www.freetype.org/)
* [assimp](https://www.assimp.org/)
* [glm](https://github.com/g-truc/glm)

If you install these libraries in a kind-of standard place, CMakeLists.txt should
be able to find them. If you've installed them in a place you think is a commonly
used path and it's not finding them, you might want to add some paths to the files
in `cmake/`.

Peony also depends on the following packages which are included in the source, and
you therefore don't need to install:

* [stb-image.h](https://github.com/nothings/stb/blob/master/stb_image.h)
* [Glad](https://glad.dav1d.de/)

## Building and running

You'll need `make` installed, including on Windows. Once you have that, you only need
to do:

```
make
make run
```
