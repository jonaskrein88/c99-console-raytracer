#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "objects.h"
#include "renderer.h"
#include "vector.h"
#include "importer.h"



void _register_mesh(RenderSettings *renderSettings, Mesh *ptr)
{
    size_t index = renderSettings->num_meshes;
    void *new = realloc(renderSettings->meshes, sizeof(Mesh*) * (index+1));
    if (new == NULL) return;
    renderSettings->meshes = (Mesh**)new;
    renderSettings->meshes[index] = ptr;
}

Object ce_new_object(Mesh *mesh)
{
    return (Object) {
        .mesh = mesh,
        .transform = matrix_identity(),
        .inverse_transform = matrix_identity()
    };
}


// registers the mesh with the rendersettings for cleanup
Mesh *ce_import_obj(RenderSettings *renderSettings, const char* filepath)
{
    Mesh *mesh = _import_obj(filepath);
    if (mesh != NULL) _register_mesh(renderSettings, mesh);
    return mesh;
}


// calls ce_import_obj and wraps the result in an object
// registers the mesh with the rendersettings for cleanup
Object ce_new_object_from_file(RenderSettings *renderSettings, const char *filepath)
{
    Mesh *mesh = ce_import_obj(renderSettings, filepath);
    return ce_new_object(mesh);
}

void ce_object_assign_material(Object *obj, Material *mtl, size_t index){
    if (index < MAX_MTLS) obj->materials[index] = mtl;
}




static void object_transform(Object *object, Matrix *m)
{
    matrix_mult(&object->transform, m);
    object->inverse_transform = matrix_inverse_trs(&object->transform);
}


void ce_rotate_x(Object *object, float degrees)
{
    Matrix m = matrix_fromXRotation(degrees);
    object_transform(object,&m);
}
// rotate in radians
void ce_rotate_y(Object *object, float degrees)
{
    Matrix m = matrix_fromYRotation(degrees);
    object_transform(object,&m);
}
// rotate in radians
void ce_rotate_z(Object *object, float degrees)
{
    Matrix m = matrix_fromZRotation(degrees);
    object_transform(object,&m);
}
void ce_translate(Object *object, Vec3 v)
{
    Matrix m = matrix_fromTranslation(v);
    object_transform(object,&m);
}




CE_PointCloud ce_make_point_grid(
    uint8_t num_x,
    uint8_t num_y,
    float spacing_x,
    float spacing_y)
{
    Vec3 *positions = calloc(num_x * num_y, sizeof(Vec3));
    
    // so we can center the grid
    float center_x = (num_x*spacing_x)/2.0f;
    float center_y = (num_y*spacing_y)/2.0f;

    if (!positions)
        return (CE_PointCloud){0};

    for (uint8_t x = 0; x < num_x; x++) 
    {
        for (uint8_t y = 0; y < num_y; y++) 
        {
            float pos_x = x * spacing_x;
            float pos_y = y * spacing_y;
            pos_x -= center_x;
            pos_y -= center_y;
            positions[x + y * num_x] = (Vec3){pos_x, 0.f, pos_y};
        }
    }

    return (CE_PointCloud) {.points = positions, .num_points= num_x*num_y};
}

void ce_scatter_objects(RenderSettings *renderSettings, CE_PointCloud *pc, Object sources[], size_t num_sources)
{
    Object *objects;
    objects = calloc(pc->num_points,sizeof(Object));
    
    for (uint16_t i=0; i<pc->num_points; i++)
    {
        Object sourceObject = sources[i % num_sources];
        objects[i] = sourceObject;
        Vec3 offset = pc->points[i];
        ce_translate(&objects[i], offset);
        ce_add_object_to_scene(renderSettings, &objects[i]);
        vec3_print(offset);
    }
}