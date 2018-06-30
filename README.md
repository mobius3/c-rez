# C-Rez

**c-rez** is a small tool to generate `C` arrays of data from a list of
input files. You can then compile them in your project and reference
them just as regular variables.

It features:

- An easy to use command-line tool
- A neat CMake interface for CMake users
- Allows you to reference assets both as strings
(`c_rez_resource("sprites.png")`) and as a variables (`sprites_png`)
- Written in portable C89 code
- Generates portable C89 code
- Its MIT licensed

You might be wondering why would anyone ever need this tool since there
are many ways of opening and reading a file, below are some of my reasons:

- Cross-platform resource loading is a pain in the arse (specially when
targeting mobile devices (I'm looking at you, Android Assets))
- Cross-platform resource bundling is even worse
- No loading time, your assets are available as soon as `main` starts
(even before!)
- No loose files, you can distribute a single executable file
- Asset bundling is now part of the compile and link process.

Of course, there are downsides:

- Makes your executable very very big depending on the size of your assets
- Which in turn makes it slow to load
- Depending on the final size of your app, the device might decide not to
load it
- You have no way of freeing up the loaded memory once you're done using
the asset
- No way of deciding what *not* to load at runtime. It is always all or
nothing (you can go around this by making your assets a shared library
and then loading them with `dlopen`, but if you're doing this you might
as well use plain old `fopen`)

## Output

A struct type is declared and used to represent each processed file.
This is how the struct looks:

```cpp
typedef struct c_rez_resource {
  unsigned char const * const data;
  unsigned int const length;
} c_rez_resource;
```

The path of input files given to the **c-rez** tool will be used to generate
an identifier. This identifier will be the variable name declared
in the `.h` file and defined it in the `.c`file. **c-rez** also accepts an `key`
that will be part of the generated identifiers.

For instance, if you pass a resource file `img/sprites.png` and key
`resources`, the `.h` file will have a variable called
`resources_img_sprites_png` that will have a `data` member pointing to its
bytes and a `length` member with the byte count.

You can also use the function `c_rez_locate_resources("img/sprites.png")`, it
will return `&resources_img_sprites_png`

Suppose you generate an `img.h` header file, it would look like this:
```cpp
#ifndef c_rez_resources_img_h
#define c_rez_resources_img_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef c_rez_resource_struct
#define c_rez_resource_struct
typedef struct c_rez_resource {
  unsigned char const * const data;
  unsigned int const length;
} c_rez_resource;
#endif /* c_rez_resource_struct */

extern c_rez_resource const resources_img_sprites_png;
struct c_rez_resource const * c_rez_locate_resources(const char name[]);

#ifdef __cplusplus
}
#endif

#endif /* c_rez_resources_img_h */
```

Aftwewards you can `#include` it in your project and use the generated structs
to get to your data:

```cpp
#include "resources/img.h"

int main() {
    printf("sprites.png length: %u\n", resources_img_sprites_png.length);
    printf("address of sprites.png: %p\n", c_rez_locate_resources("img/sprites.png");
    return 0;
}
```

You can then either commit these files to your project or make it part of your
build process.

Refer to either [CMake usage](#cmake-usage) or [Command-line
tool](#command-line-tool) on how to generate and use these files in your
project.

## CMake usage

### As a subproject

**c-rez** is ready to be used as a sub-project. Simply download the
source archive and use `add_subdirectory` before adding targets using
**c-rez**

```cmake
add_subdirectory(path/to/c-rez)
```

### As a CMake package

You can either compile from source or install a release archive.

**Compile from source**

1. Have a compiler environment ready (GCC, LLVM, MSVC, MinGW, etc);
2. Have [CMake](http://cmake.org) 3.0 (minimum) installed;
3. Download this repository;
4. Create a `build` folder inside it;
5. Run `cmake` (or `cmake-gui`), set the binary dir to the newly created
build folder and the source dir to the repository folder;
6. Build it with your IDE of choice.
7. Install it with `make install` or run the *INSTALL* target (you might
need to use **sudo** here or to adjust `CMAKE_INSTALL_PREFIX`)

**Install a release**

1. Download a suitable archive from the releases section
2. Unzip it somewhere you can later find with CMake (you may need to
adjust the `CMAKE_PREFIX_PATH` of your project)

**Add package**

Use `find_package` to add it to your project:

```cmake
list(APPEND CMAKE_PREFIX_PATH /path/to/c-rez/prefix)
find_package(c-rez REQUIRED)
```

### Create a resource target

After using either `find_package` or `add_subdirectory`, a new CMake
function will be available: `add_c_rez`. This function will create a new
`C` *STATIC* library under the `c-rez` namespace that you can link your
targets to:

```cmake
add_executable(mygame mygame.c engine.c)
add_c_rez(images
    assets/tileset.png
    assets/sprites.png
    assets/gui.png
)
target_link_libraries(mygame c-rez::images)
```

If you place the word `TEXT` before an path, it will be treated as text and an
`\0` will be appended to its data:

```cmake
add_c_rez(texts
    TEXT texts/chapter01.txt
    TEXT texts/chapter02.txt
    TEXT texts/chapter03.txt
)
```

The header files for your target will be under a `c-rez` folder in your build
path (*different* targets might not be in the *same* path, though)

```c
#include "c-rez/images.h"
#include "c-rez/texts.h"
```

## Command-line tool
If added to your `PATH`, you can call **c-rez** as a command line tool to
 generate a header
and/or source files:

```
c-rez -k <resource key> [-h <output.h>] [-c <output.c>] [--text] <input_1> [[--text] <input_2>] [[--text] <input_n>]*
```

- **-h \<file.h\>**: specifies the header output file. If omitted, only
 source gets generated.
- **-c \<file.c\>**: specifies the source output file. If omitted, only
header gets generated.
- **-k \<resource key\>**: specifies an key to identify this resource. It will
 be used in header guards and resource functions.
- **--text**: appends a `\0` when processing the next *\<input\>*
file. This helps when using its data as a string resource.
- **\<input\>**: space separated list of files to read from.
Declarations and definitions will be generated based on the file name. If
`--text` is specified before the file name, an `\0` will be appended after
processing.

# License

**c-rez** is MIT-licensed.
