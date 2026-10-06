# use PowerShell instead of sh:
set shell := ["cmd.exe", "/c"]

#  compile to main.exe
build:
	cls
	gcc main.c renderer.c vector.c importer.c objects.c shaders.c shader_utils.c bvh.c graphics2D.c raymarching.c lookup.c -O3 -fopenmp -o main.exe

# convert all textures in models to bmp
convert_textures:
	cls
	py ./models/toBMP_subfolders.py