#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>
#include "vector.h"
#include "importer.h"
#include "bvh.h"

static float min3(float a, float b, float c) {
    float m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

static float max3(float a, float b, float c) {
    float m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}



void compute_face_bounds(Mesh *mesh, const Face* face, AABB* out_bounds) {    
    Vertex a = mesh->vertices[face->v0]; 
    Vertex b = mesh->vertices[face->v1];
    Vertex c = mesh->vertices[face->v2];
    float xmin = min3(a.x,b.x,c.x);
    float ymin = min3(a.y,b.y,c.y);
    float zmin = min3(a.z,b.z,c.z);
    float xmax = max3(a.x,b.x,c.x);
    float ymax = max3(a.y,b.y,c.y);
    float zmax = max3(a.z,b.z,c.z);
    out_bounds->min = (Vec3) {xmin,ymin,zmin};
    out_bounds->max = (Vec3) {xmax,ymax,zmax};
}

// create an accumulated bounding box
void grow_aabb(AABB* to_grow, const AABB* to_include) {
    if (to_include->min.x < to_grow->min.x) to_grow->min.x = to_include->min.x;
    if (to_include->min.y < to_grow->min.y) to_grow->min.y = to_include->min.y;
    if (to_include->min.z < to_grow->min.z) to_grow->min.z = to_include->min.z;

    if (to_include->max.x > to_grow->max.x) to_grow->max.x = to_include->max.x;
    if (to_include->max.y > to_grow->max.y) to_grow->max.y = to_include->max.y;
    if (to_include->max.z > to_grow->max.z) to_grow->max.z = to_include->max.z;
}

void update_node_bounds(Mesh* mesh, AABB* node_bounds, const int* face_indices, int first_face, int face_count) {
    node_bounds->min = (Vec3){  FLT_MAX, FLT_MAX, FLT_MAX };
    node_bounds->max = (Vec3){  -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (int i = 0; i < face_count; i++) {
        int face_idx = face_indices[first_face + i];
        AABB face_box;
        compute_face_bounds(mesh, &mesh->faces[face_idx], &face_box);
        grow_aabb(node_bounds, &face_box);
    }
}

// Helper to determine the best axis and the middle position to split on
int choose_split_plane(const BVHNode* node, float* out_split_pos) {
    float extent_x = node->bounds.max.x - node->bounds.min.x;
    float extent_y = node->bounds.max.y - node->bounds.min.y;
    float extent_z = node->bounds.max.z - node->bounds.min.z;

    int axis = 0;
    if (extent_y > extent_x && extent_y > extent_z) {
        axis = 1; // Y axis
    } else if (extent_z > extent_x && extent_z > extent_y) {
        axis = 2; // Z axis
    }
    if (axis == 0) {
        *out_split_pos = node->bounds.min.x + extent_x * 0.5f;
    } else if (axis == 1) {
        *out_split_pos = node->bounds.min.y + extent_y * 0.5f;
    } else {
        *out_split_pos = node->bounds.min.z + extent_z * 0.5f;
    }

    return axis; // Returns 0, 1, or 2
}







void subdivide(Mesh* mesh, int node_idx, int* nodes_used) {
    BVHNode* node = &mesh->bvh_nodes[node_idx];

    if (node->face_count <= 2) {
        return;
    }

    float split_pos;
    int axis = choose_split_plane(node, &split_pos);

    // partitioning loop
    int i = node->left_first;
    int j = node->left_first + node->face_count - 1;

    while (i <= j) {
        int face_idx = mesh->face_indices[i];
        float centroid_coord;
        
        if (axis == 0)      centroid_coord = mesh->faces[face_idx].centroid.x;
        else if (axis == 1) centroid_coord = mesh->faces[face_idx].centroid.y;
        else                centroid_coord = mesh->faces[face_idx].centroid.z;

        if (centroid_coord < split_pos) {
            i++; 
        } else {
            int temp = mesh->face_indices[i];
            mesh->face_indices[i] = mesh->face_indices[j];
            mesh->face_indices[j] = temp;
            j--;
        }
    }

    // if all faces ended up on one side, do not split
    int left_count = i - node->left_first;
    if (left_count == 0 || left_count == node->face_count) {
        return; 
    }

    // allocate child node slots from the pre-allocated pool
    int left_child_idx = *nodes_used;
    int right_child_idx = *nodes_used + 1;
    *nodes_used += 2;

    // init left
    mesh->bvh_nodes[left_child_idx].left_first = node->left_first;
    mesh->bvh_nodes[left_child_idx].face_count = left_count;

    // init right
    mesh->bvh_nodes[right_child_idx].left_first = i;
    mesh->bvh_nodes[right_child_idx].face_count = node->face_count - left_count;

    // parent node to branch
    node->left_first = left_child_idx;
    node->face_count = 0; 

    update_node_bounds(mesh, &mesh->bvh_nodes[left_child_idx].bounds,  mesh->face_indices, mesh->bvh_nodes[left_child_idx].left_first,  mesh->bvh_nodes[left_child_idx].face_count);
    update_node_bounds(mesh, &mesh->bvh_nodes[right_child_idx].bounds, mesh->face_indices, mesh->bvh_nodes[right_child_idx].left_first, mesh->bvh_nodes[right_child_idx].face_count);

    subdivide(mesh, left_child_idx, nodes_used);
    subdivide(mesh, right_child_idx, nodes_used);
}



BVHNode *create_bvh_tree(Mesh *mesh)
{
    mesh->face_indices = malloc(sizeof(int) * mesh->face_count);
    for (int i = 0; i < mesh->face_count; i++) {
        mesh->face_indices[i] = i; 
    }

    int max_nodes = 2 * mesh->face_count - 1;
    
    // FIX: Assign the malloc'd pointer directly into the mesh struct!
    mesh->bvh_nodes = malloc(sizeof(BVHNode) * max_nodes);

    // Set up the root node
    int nodes_used = 1; 
    mesh->bvh_nodes[0].left_first = 0; // Now safe! No more segfault.
;
    mesh->bvh_nodes[0].face_count = mesh->face_count;
    
    // Get initial root bounds
    update_node_bounds(mesh, &mesh->bvh_nodes[0].bounds, mesh->face_indices, 0, mesh->face_count);

    // Start the recursive breakdown
    subdivide(mesh, 0, &nodes_used);
}



