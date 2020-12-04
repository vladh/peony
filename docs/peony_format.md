# Peony Format

The peony file format looks very much like an .ini file.

It contains sections delimited with section titles:

```
[section]
// things here

[other section]
// other things here
```

Section names must match `[a-zA-Z0-9-_\s]+`.

Comments are preceded by `//` or `;`.

Blank lines are ignored.

Sections contain key-value pairs.

The key must be a string of form `[a-zA-Z0-9-_]+`.

The equal sign may be surrounded by spaces, or not.

The value may be specified, or may be absent, in which case the equal sign is
also omitted (see below). If specified, the value itself may be anything, as
it is parsed by the module responsible for it.  For example, a `texture_sets`
file will expect a file path for an `albedo_texture` key, but a vec4 for an
`albedo_static` key. However, the notation of these different formats is
uniform, and can be seen below.

```
[demo-section]
bare-key
bool = true
string = Schwarzwälder Kirschtorte
path = resources/file.jpg
int = 5
float = 5.0
vec2 = vec2(1.0, 0.5)
vec3 = vec3(-0.5, 2.0, 1.0)
vec4 = vec4(0.2, -1.0, 0.0, 1.0)
```
