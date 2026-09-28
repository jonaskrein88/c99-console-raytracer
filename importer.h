#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "vector.h"
#include "bvh.h"




typedef Vec3 Vertex;
typedef Vec3 Normal;





typedef struct Face {
    int v0, v1, v2;
    int n0, n1, n2;
    int t0, t1, t2;
    Vec3 centroid;
    Normal normal;
    int material_id;
} Face;





typedef struct Mesh {
    Vertex *vertices;
    Normal *normals;
    Vec3 *uvs;
    Face *faces;
    int vertex_count;
    int face_count;
    int normal_count;
    int uv_count;
    Vec3 bounds_min;
    Vec3 bounds_max;

    // --- BVH Acceleration Data ---
    BVHNode* bvh_nodes;     // The allocated pool array (size: 2 * face_count - 1)
    int* face_indices;      // The indirection array (size: face_count)
    int bvh_root_idx;       // Always 0, but good for tracking
} Mesh;

Mesh *_import_obj(const char *filename);
void free_mesh(Mesh *mesh);







typedef struct TextureImage{
    int width;
    int height;
    Color *pixels;
} TextureImage;

bool _load_bmp_file(const char *filename, TextureImage *img);
void _debug_print_texture(TextureImage *img);