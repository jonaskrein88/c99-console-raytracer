
#pragma once

#include <stdint.h>
#include "vector.h"
//#include "importer.h"

typedef struct Mesh Mesh; 
typedef struct Face Face; 

typedef struct AABB{
    Vec3 min; 
    Vec3 max;
} AABB;

typedef struct {
    AABB bounds;
    int16_t left_first;      // If count > 0: index to primitive_indices array. If count == 0: index to left child node.
    int16_t face_count; // Number of primitives inside this node. 0 means internal branch node.
} BVHNode;


void compute_face_bounds(Mesh *mesh, const Face* face, AABB* out_bounds);    

BVHNode *create_bvh_tree(Mesh *mesh);