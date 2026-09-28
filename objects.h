#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "vector.h"
#include "importer.h"
#include "renderer.h"

#define MAX_MTLS 32

typedef struct Material Material; 
typedef struct RenderSettings RenderSettings; 

typedef struct Object{
    Mesh *mesh;
    Material *materials[MAX_MTLS];
    Matrix transform;
    Matrix inverse_transform;
} Object;


typedef struct CE_PointCloud{
    size_t num_points;
    Vec3 *points;
} CE_PointCloud;



void _register_mesh(RenderSettings *renderSettings, Mesh *ptr);

Object ce_new_object(Mesh *mesh);
Mesh *ce_import_obj(RenderSettings *renderSettings, const char* filepath);
Object ce_new_object_from_file(RenderSettings *renderSettings, const char *filepath);

void ce_object_assign_material(Object *obj, Material *mtl, size_t index);

static void object_transform(Object *object, Matrix *m);
void ce_rotate_x(Object *object, float degrees);
void ce_rotate_y(Object *object, float degrees);
void ce_rotate_z(Object *object, float degrees);
void ce_translate(Object *object, Vec3 v);

CE_PointCloud ce_make_point_grid(uint8_t num_x, uint8_t num_y,float spacing_x,float spacing_y);
CE_PointCloud ce_make_point_cloud(Vec3 bounds, size_t num);
void ce_scatter_objects(RenderSettings *renderSettings, CE_PointCloud *pc, Object sources[], size_t num_sources);

