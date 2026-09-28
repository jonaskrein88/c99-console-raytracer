# use PowerShell instead of sh:
set shell := ["cmd.exe", "/c"]

# just compile to main.exe
build:
	cls
	gcc main.c renderer.c vector.c importer.c objects.c shaders.c shader_utils.c bvh.c graphics2D.c raymarching.c lookup.c -O3 -fopenmp -o main.exe
