#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>

#include "importer.h"
#include "vector.h"
#include "bvh.h"

static Vertex get_vertex(Mesh *mesh, size_t index)
{
    if (index >= mesh->vertex_count) printf("vertex out of range");
    return mesh->vertices[index];
}





static void precalculate_face_normals(Mesh *mesh)
{
    for (size_t i = 0; i<mesh->face_count; i++)
    {
        Face *face = &mesh->faces[i];
        Vertex a = get_vertex(mesh,face->v0);
        Vertex b = get_vertex(mesh,face->v1);
        Vertex c = get_vertex(mesh,face->v2);
        Vec3 v0v1 = vec3_sub(b, a);
        Vec3 v0v2 = vec3_sub(c, a);
        Normal N = vec3_normalize(vec3_cross(v0v1, v0v2));
        face->normal = N;
    }
}

static void precalculate_bounding_box(Mesh *mesh)
{
    mesh->bounds_min = (Vec3){ FLT_MAX,  FLT_MAX,  FLT_MAX};
    mesh->bounds_max = (Vec3){-FLT_MAX, -FLT_MAX, -FLT_MAX};

    for (size_t i = 0; i < mesh->vertex_count; i++) {
        Vertex v = mesh->vertices[i];
        if (v.x < mesh->bounds_min.x) mesh->bounds_min.x = v.x;
        if (v.y < mesh->bounds_min.y) mesh->bounds_min.y = v.y;
        if (v.z < mesh->bounds_min.z) mesh->bounds_min.z = v.z;

        if (v.x > mesh->bounds_max.x) mesh->bounds_max.x = v.x;
        if (v.y > mesh->bounds_max.y) mesh->bounds_max.y = v.y;
        if (v.z > mesh->bounds_max.z) mesh->bounds_max.z = v.z;
    }
}



typedef struct{
    size_t num;
    char strings[32][64];
}StringList;


// utility for adding and fetching materials by name when importing
int find_or_insert(StringList *list, char *name)
{
    for (size_t i = 0; i<list->num; i++)
    {
        char *entry = list->strings[i];
        int cmp = strncmp(name, entry, 256);
        if (cmp == 0) return i;
    }
    if (list->num >= 32) return -1;                     // guard if number of strings > 32
    strncpy(list->strings[list->num], name, 256 );
    list->strings[list->num][255] = '\0';                // force null terminator if string longer than 64
    list->num++;
    return list->num-1;
}








// for bvh
static void set_centroid(Mesh *mesh, Face *face)
{
    Vertex a = mesh->vertices[face->v0]; 
    Vertex b = mesh->vertices[face->v1];
    Vertex c = mesh->vertices[face->v2];
    face->centroid = vec3_div(vec3_add(vec3_add(a,b),c),3.0);
}



// use ce_new_object_from_file
// or  ce_import_obj
// the importer has a lot of uncaught fail states
// the savest way is to use a mesh with only triangles and vertex normals
Mesh *_import_obj(const char *filename)
{
    Mesh *mesh = calloc(1,sizeof(Mesh));

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);
        return NULL;
    }

    printf("Loading %s\n", filename);

    StringList materials = {0};
    char mtlName[256];
    int current_material_index = 0;

    char line[512];

    // prepass
    // count the vertices, uvs, normals and faces

    while (fgets(line, sizeof(line), file)) {

        if (strncmp(line, "v ", 2) == 0) {
            mesh->vertex_count++;
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            mesh->normal_count++;
        }
        else if (strncmp(line, "vt ", 3) == 0) {
            mesh->uv_count++;
        }
        else if (strncmp(line, "f ", 2) == 0) {

            int vertex_count = 0;

            char *p = line + 2;

            while (*p) {

                while (*p == ' ' || *p == '\t')
                    p++;

                if (*p == '\0' || *p == '\n')
                    break;

                vertex_count++;

                // skip this face vertex specification
                while (*p != ' ' &&
                       *p != '\t' &&
                       *p != '\0' &&
                       *p != '\n')
                {
                    p++;
                }
            }

            if (vertex_count == 3) {
                mesh->face_count += 1;
            }
            else if (vertex_count == 4) {
                mesh->face_count += 2;
            }
            else if (vertex_count > 4) {
                fprintf(stderr,
                        "Warning: face with %d vertices encountered; "
                        "only triangles/quads are supported.\n",
                        vertex_count);
            }
        }
    }

    printf(" - Vertices: %d\n", mesh->vertex_count);
    printf(" - Normals:  %d\n", mesh->normal_count);
    printf(" - UVs:      %d\n", mesh->uv_count);
    printf(" - Faces:    %d\n", mesh->face_count);

    if (mesh->vertex_count > 0) {
        mesh->vertices = malloc(
            mesh->vertex_count * sizeof(Vertex)
        );
    }

    if (mesh->normal_count > 0) {
        mesh->normals = malloc(
            mesh->normal_count * sizeof(Normal)
        );
    }

    if (mesh->uv_count > 0) {
        mesh->uvs = malloc(
            mesh->uv_count * sizeof(Vec3)
        );
    }

    if (mesh->face_count > 0) {
        mesh->faces = malloc(
            mesh->face_count * sizeof(Face)
        );
    }

    if ((mesh->vertex_count > 0 && !mesh->vertices) ||
        (mesh->normal_count > 0 && !mesh->normals) ||
        (mesh->uv_count > 0 && !mesh->uvs) ||
        (mesh->face_count > 0 && !mesh->faces))
    {
        fprintf(stderr, "Failed to allocate OBJ mesh->memory\n");

        free(mesh->vertices);
        free(mesh->normals);
        free(mesh->uvs);
        free(mesh->faces);

        fclose(file);
        return NULL;
    }

    rewind(file);

    // parse the actual data

    int v_idx = 0;
    int n_idx = 0;
    int t_idx = 0;
    int f_idx = 0;

    while (fgets(line, sizeof(line), file)) {

        // comments / empty lines

        if (line[0] == '#')
            continue;

        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
            continue;


        // object / group / smoothing information

        if (strncmp(line, "g ", 2) == 0 ||
            strncmp(line, "o ", 2) == 0 ||
            strncmp(line, "s ", 2) == 0)
        {
            continue;
        }

        // material

        if (strncmp(line, "usemtl", 6) == 0) {

            int success = sscanf(
                line,
                "usemtl %255s",
                mtlName
            );

            if (success == 1) {
                //printf(" - found material: %s\n", mtlName);

                current_material_index =
                    find_or_insert(&materials, mtlName);

                printf(" - material %s set to index: %d\n",
                       mtlName, current_material_index);
            }
            else {
                fprintf(stderr,
                        " ! could not parse material: %s",
                        line);
            }

            continue;
        }

        // material library
        // TODO same as usemtl???
        if (strncmp(line, "mtllib", 6) == 0) {
            continue;
        }

        // vertex position

        if (strncmp(line, "v ", 2) == 0) {

            if (v_idx >= mesh->vertex_count) {
                fprintf(stderr,
                        "CRITICAL ERROR: Vertex index overflow!\n");
                break;
            }

            int parsed = sscanf(
                line,
                "v %f %f %f",
                &mesh->vertices[v_idx].x,
                &mesh->vertices[v_idx].y,
                &mesh->vertices[v_idx].z
            );

            if (parsed != 3) {
                fprintf(stderr, "Warning: Could not parse vertex: %s", line);
                continue;
            }

            v_idx++;
            continue;
        }

        // vertex normal

        if (strncmp(line, "vn ", 3) == 0) {

            if (n_idx >= mesh->normal_count) {
                fprintf(stderr,"CRITICAL ERROR: Normal index overflow!\n");
                break;
            }

            int parsed = sscanf(
                line,
                "vn %f %f %f",
                &mesh->normals[n_idx].x,
                &mesh->normals[n_idx].y,
                &mesh->normals[n_idx].z
            );

            if (parsed != 3) {fprintf(stderr,"Warning: Could not parse normal: %s",line);
                continue;
            }

            n_idx++;
            continue;
        }


        // uv

        if (strncmp(line, "vt ", 3) == 0) {

            if (t_idx >= mesh->uv_count) {
                fprintf(stderr, "CRITICAL ERROR: UV index overflow!\n"); 
                break;
            }

            // NOTE:
            // obj normally has:
            //
            // vt u v
            //
            // but may optionally have:
            //
            // vt u v w
            //
            // we initialize z for savety

            mesh->uvs[t_idx].x = 0.0f;
            mesh->uvs[t_idx].y = 0.0f;
            mesh->uvs[t_idx].z = 0.0f;

            int parsed = sscanf(
                line,
                "vt %f %f %f",
                &mesh->uvs[t_idx].x,
                &mesh->uvs[t_idx].y,
                &mesh->uvs[t_idx].z
            );

            if (parsed < 2) {
                fprintf(stderr, "Warning: Could not parse UV: %s", line);
                continue;
            }

            t_idx++;
            continue;
        }




        // face

        if (strncmp(line, "f ", 2) == 0) {

            if (f_idx >= mesh->face_count) {
                fprintf(stderr, "CRITICAL ERROR: Face index overflow!\n");
                break;
            }

            int v0 = -1, t0 = -1, n0 = -1;
            int v1 = -1, t1 = -1, n1 = -1;
            int v2 = -1, t2 = -1, n2 = -1;
            int v3 = -1, t3 = -1, n3 = -1;

            // first try: a textured QUAD:
            // f v/vt/vn v/vt/vn v/vt/vn v/vt/vn
            // 12 integer

            int parsed = sscanf(
                line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &v0, &t0, &n0,
                &v1, &t1, &n1,
                &v2, &t2, &n2,
                &v3, &t3, &n3
            );

            if (parsed == 12) {
                v0--; t0--; n0--;
                v1--; t1--; n1--;
                v2--; t2--; n2--;
                v3--; t3--; n3--;

                // triangle 1

                mesh->faces[f_idx].v0 = v0;
                mesh->faces[f_idx].t0 = t0;
                mesh->faces[f_idx].n0 = n0;

                mesh->faces[f_idx].v1 = v1;
                mesh->faces[f_idx].t1 = t1;
                mesh->faces[f_idx].n1 = n1;

                mesh->faces[f_idx].v2 = v2;
                mesh->faces[f_idx].t2 = t2;
                mesh->faces[f_idx].n2 = n2;

                mesh->faces[f_idx].material_id =
                    current_material_index;

                set_centroid(
                    mesh,
                    &mesh->faces[f_idx]
                );

                f_idx++;

                // triangle 2

                mesh->faces[f_idx].v0 = v0;
                mesh->faces[f_idx].t0 = t0;
                mesh->faces[f_idx].n0 = n0;

                mesh->faces[f_idx].v1 = v2;
                mesh->faces[f_idx].t1 = t2;
                mesh->faces[f_idx].n1 = n2;

                mesh->faces[f_idx].v2 = v3;
                mesh->faces[f_idx].t2 = t3;
                mesh->faces[f_idx].n2 = n3;

                mesh->faces[f_idx].material_id =
                    current_material_index;

                set_centroid(
                    mesh,
                    &mesh->faces[f_idx]
                );

                f_idx++;

                continue;
            }

            // try an untextured QUAD:
            // f v//vn v//vn v//vn v//vn
            // 8 integers

            parsed = sscanf(
                line,
                "f %d//%d %d//%d %d//%d %d//%d",
                &v0, &n0,
                &v1, &n1,
                &v2, &n2,
                &v3, &n3
            );

            if (parsed == 8) {

                v0--; n0--;
                v1--; n1--;
                v2--; n2--;
                v3--; n3--;

                // no UVs.
                t0 = -1;
                t1 = -1;
                t2 = -1;
                t3 = -1;

                // triangle 1

                mesh->faces[f_idx].v0 = v0;
                mesh->faces[f_idx].t0 = t0;
                mesh->faces[f_idx].n0 = n0;

                mesh->faces[f_idx].v1 = v1;
                mesh->faces[f_idx].t1 = t1;
                mesh->faces[f_idx].n1 = n1;

                mesh->faces[f_idx].v2 = v2;
                mesh->faces[f_idx].t2 = t2;
                mesh->faces[f_idx].n2 = n2;

                mesh->faces[f_idx].material_id =
                    current_material_index;

                set_centroid(
                    mesh,
                    &mesh->faces[f_idx]
                );

                f_idx++;

                // triangle 2

                mesh->faces[f_idx].v0 = v0;
                mesh->faces[f_idx].t0 = t0;
                mesh->faces[f_idx].n0 = n0;

                mesh->faces[f_idx].v1 = v2;
                mesh->faces[f_idx].t1 = t2;
                mesh->faces[f_idx].n1 = n2;

                mesh->faces[f_idx].v2 = v3;
                mesh->faces[f_idx].t2 = t3;
                mesh->faces[f_idx].n2 = n3;

                mesh->faces[f_idx].material_id =
                    current_material_index;

                set_centroid(
                    mesh,
                    &mesh->faces[f_idx]
                );

                f_idx++;

                continue;
            }

            // try a textured TRIANGLE:
            // f v/vt/vn v/vt/vn v/vt/vn
            // 9 integers

            parsed = sscanf(
                line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &v0, &t0, &n0,
                &v1, &t1, &n1,
                &v2, &t2, &n2
            );

            if (parsed == 9) {

                v0--; t0--; n0--;
                v1--; t1--; n1--;
                v2--; t2--; n2--;

                mesh->faces[f_idx].v0 = v0;
                mesh->faces[f_idx].t0 = t0;
                mesh->faces[f_idx].n0 = n0;

                mesh->faces[f_idx].v1 = v1;
                mesh->faces[f_idx].t1 = t1;
                mesh->faces[f_idx].n1 = n1;

                mesh->faces[f_idx].v2 = v2;
                mesh->faces[f_idx].t2 = t2;
                mesh->faces[f_idx].n2 = n2;

                mesh->faces[f_idx].material_id =
                    current_material_index;

                set_centroid(
                    mesh,
                    &mesh->faces[f_idx]
                );

                f_idx++;

                continue;
            }

            // try an untextured TRIANGLE:
            // f v//vn v//vn v//vn
            // 6 integers

            parsed = sscanf(
                line,
                "f %d//%d %d//%d %d//%d",
                &v0, &n0,
                &v1, &n1,
                &v2, &n2
            );

            if (parsed == 6) {

                v0--; n0--;
                v1--; n1--;
                v2--; n2--;

                // no uvs

                t0 = -1;
                t1 = -1;
                t2 = -1;

                mesh->faces[f_idx].v0 = v0;
                mesh->faces[f_idx].t0 = t0;
                mesh->faces[f_idx].n0 = n0;

                mesh->faces[f_idx].v1 = v1;
                mesh->faces[f_idx].t1 = t1;
                mesh->faces[f_idx].n1 = n1;

                mesh->faces[f_idx].v2 = v2;
                mesh->faces[f_idx].t2 = t2;
                mesh->faces[f_idx].n2 = n2;

                mesh->faces[f_idx].material_id =
                    current_material_index;

                set_centroid(
                    mesh,
                    &mesh->faces[f_idx]
                );

                f_idx++;

                continue;
            }

            // face format not supported
            fprintf(stderr,
                    "Warning: Could not parse face: %s",
                    line);
        }
    }

    fclose(file);
    mesh->face_count = f_idx;


    precalculate_face_normals(mesh);
    precalculate_bounding_box(mesh);

    Matrix m = matrix_identity();
    create_bvh_tree(mesh); 
    return mesh;
}



    


void free_mesh(Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->faces);
    free(mesh->normals);
    mesh->vertices = NULL;
    mesh->normals = NULL;
    mesh->faces = NULL;
    free(mesh);
}












// only to be called from the renderer
// because it needs to be regisered
bool _load_bmp_file(const char *filename, TextureImage *img) {

    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open BMP file %s\n", filename);
        return false;
    }

    // 1. Read key positions from the 54-byte Windows header
    uint8_t header[54];
    if (fread(header, 1, 54, file) != 54) {
        fprintf(stderr, "Error: Invalid BMP file header size\n");
        fclose(file);
        return false;
    }

    // Verify the "BM" signature bytes at indices 0 and 1
    if (header[0] != 'B' || header[1] != 'M') {
        fprintf(stderr, "Error: Not a valid Windows BMP asset file\n");
        fclose(file);
        return false;
    }

    // Extract width, height, and color bit depth from fixed header locations
    img->width  = *(int32_t*)&header[18];
    img->height = *(int32_t*)&header[22];
    uint16_t bits_per_pixel = *(uint16_t*)&header[28];

    // Ensure it's a standard 24-bit uncompressed RGB image
    if (bits_per_pixel != 24) {
        fprintf(stderr, "Error: Unsupported bit depth. Must be an uncompressed 24-bit BMP.\n");
        fclose(file);
        return false;
    }

    // allocate memory array pool
    int total_pixels = img->width * img->height;
    img->pixels = (Color*)malloc(total_pixels * sizeof(Color));
    
    // BMP image pixels are natively stored upside down (bottom-to-top) 
    // and using a BGR color channel order instead of RGB!
    // for (int y = img->height - 1; y >= 0; y--) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t bgr[3];
            fread(bgr, 1, 3, file);
            
            // Re-order BGR layout variables back to custom standard RGB channels
            int idx = y * img->width + x;
            img->pixels[idx].x = bgr[2] / 255.f;
            img->pixels[idx].y = bgr[1] / 255.f;
            img->pixels[idx].z = bgr[0] / 255.f;
        }
        int row_bytes = img->width * 3;
        int padding = (4 - (row_bytes % 4)) % 4;
        fseek(file, padding, SEEK_CUR); // Safely skip the alignment padding bytes
    }

    fclose(file);
    
    printf("Successfully loaded BMP texture: %dx%d pixels\n", img->width, img->height);
    return true;
}


void debug_print_texture(TextureImage *img)
{
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            int idx = y * img->width + x;
            Color c = img->pixels[idx];
            vec3_print(c);
        }
    }
}
