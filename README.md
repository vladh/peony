# peony

![peony screenshot](https://vladh.net/static/peony@1000px.jpg)

Peony is a 3D game engine written from scratch in C++. It's currently a work
in progress.

## Why build a 3D game engine from scratch?

I remember being so impressed by many video games as a kid — it was so
incomprehensible to me how anyone could build something like Pokémon Blue,
let alone Half-Life 2! I've always wanted to create something like that myself.

That's why in 2020, I decided to build my own 3D game engine from scratch
in C++. My objective is to create something that, while being an indie engine,
has many features that you would find in production game engines, such as
nice lighting and shadows, basic physics and collision, water and atmospheric
effects and so on.

To me, using a game library would completely defeat the purpose. Building the
rendering, lighting, physics and all the other components of the engine has
honestly been even more fun and rewarding than I could have hoped. I try
to stay dependency-free as much as I can. Currently, my only real
dependencies are GLFW (for platform-specific things) and Freetype
(for loading fonts). I am currently using assimp to load the models,
but it will get kicked out soon enough.

## A list of features

Here's a quick summary of some of the features I've built so far:

* PBR-compatible lighting
* Albedo/normal/metallic/roughness/AO support
* Cascading shadow maps
* Dynamic lights
* Procedural water
* Forward and deferred rendering passes
* Bloom, HDR, fog and other postprocessing effects
* Procedural skybox
* OBB-based collision detection
* Simple rigidbody physics
* Custom GUI system
* Development tools such as a game console
* Skeletal animation
* Dynamic scene and asset loading from files
* Custom scene and material file format
* Shader hot-loading

## Why open source?

While building this engine, I found it quite helpful to look at existing resources on
game engines, including existing open-source game engines. If publishing Peony can help
someone learn, I'd be happy about that.

## Practical details

If you'd like to read the code, you can start at `src/main.cpp`.

There is more information about Peony and its features on my website:

* [Peony — a 3D game engine written from scratch in C++](https://vladh.net/peony.html)
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

Peony should compile and run fine on Windows, macOS and Linux. However, macOS support
might be a bit flaky.

## Peony Game Engine License

All of the source code in the Peony Game Engine is Copyright 2020 by Vlad-Stefan Harbuz,
unless otherwise specified. All rights are reserved. You are granted a personal,
non-assignable, non-transferable, non-commercial license to use the source code for your
own personal educational purposes. Any other use, including any redistribution in whole
or in part, requires explicit, written permission from Vlad-Stefan Harbuz.

The Peony Game Engine is provided "as is" without warranty of any kind, either express or
implied, including without limitation any implied warranties of condition, uninterrupted
use, merchantability, fitness for a particular purpose, or non-infringement.
