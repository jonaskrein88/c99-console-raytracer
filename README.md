# c99-console-raytracer
A very primitve raytracer that renders directly to the console (Windows only atm).


This is a learning project in C99 with no external dependencies or libraries (other than win32).

- uses a binary volume hierarchy for acceleration
- real time viable for small meshes
- scriptable shaders for surfaces and post processing
- can output to older consoles with ANSI 16 or XTERM 256 palettes.
- supports up to three byte (extended) ASCII characters - for example: ░▒▓█

Some shaders draw the final character cell explicitly:

![City](images/city_00.png)

But most shaders generate a full float RGB color which will then be mapped to the final color encoding.
To accelerate the process we can precalculate look-up tables on how to best replicate the color with our chosen palette and ASCII characters.
![Tie](images/tie.png)


This is often the case when using anything that resamples a pixel, ie. post processing or reflections
![tape](images/tape.png)

We can also output to XTERM with 256 colors
![ReflectionTest](images/reflection_test.png)
![Elephant](images/elephant.png)
