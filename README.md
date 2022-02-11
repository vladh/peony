# peony

![peony screenshot](https://vladh.net/static/peony@1000px.jpg)

Peony is a 3D game engine written from scratch in C++. It's currently a work
in progress.

## Why build a 3D game engine from scratch?

I've been in love with the technology behind video games my entire life. As a kid, it was
incomprehensible to me how anyone could build something like Pokémon Blue, let alone
Half-Life 2. Despite that, I'd never built anything resembling a game engine myself.

That's why in 2020, I decided to build my own 3D game engine from scratch in C++. My
objective is to create something that, while being an indie engine, has many features
that you would find in production game engines, such as nice lighting and shadows, basic
physics and collision, water and atmospheric effects and so on.

Aside from furthering my own education, I have two aims in building Peony. First of all,
I'd like to see how compelling of a player experience can be created with a relatively
minimal amount of code and small budget. To this end, I am aiming to eventually build a
game using this engine. Secondly, there is a dearth of practical game engine development
resources on the internet, and I hope to make a small contribution to this community by
publishing Peony's source code.

After a little over a year of working on this engine, I've found the “from scratch”
approach even more fun and rewarding than I could have hoped. Aside from a couple of
dependencies such as GLFW and Freetype (and the soon-to-be-removed assimp), I've created
all features of this engine myself, with no dependencies.

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
* Custom GUI system
* Development tools such as a game console
* Skeletal animation
* Custom entity system
* Dynamic scene and asset loading from files
* Custom scene and material file format
* Shader hot-loading

## Roadmap

Here are some features that I'm currently working on or planning to add soon, in order of
descending priority:

* Simple rigidbody physics
* Cascading shadow maps
* Spatially-aware audio
* Dialogue system using existing GUI
* A better render queue system
* Better development tools such as scene editing

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

## License

This project is licensed under a restrictive license. You are free to use the
source code only for personal, educational, non-commercial purposes. The full
license is included.
