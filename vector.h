#pragma once


typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;


typedef Vec3 Color;

typedef struct {
    float m[16];
} Matrix;



float float_sign(float a);
Vec3 vec3_up();
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_negate(Vec3 v);
Vec3 vec3_mul(Vec3 v, float s);
Vec3 vec3_div(Vec3 v, float s);
Vec3 vec3_pow(Vec3 v, float s);
Vec3 vec3_mul_vec3(Vec3 a, Vec3 b);
float vec3_magnitude(Vec3 v);
float vec3_distance(Vec3 a, Vec3 b);
float vec3_distance2(Vec3 a, Vec3 b);
Vec3 vec3_normalize(Vec3 v);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 vec3_reflect(Vec3 i, Vec3 n);
Vec3 vec3_abs(Vec3 v);
Vec3 vec3_saturate(Vec3 v);
Vec3 vec3_quantize(Vec3 v, int bins);
Vec3 vec3_lerp(Vec3 a, Vec3 b, float t);
float vec3_perceptual_distance(Color c1, Color c2);
Vec3 vec3_xyy(Vec3 v);
Vec3 vec3_yxy(Vec3 v);
Vec3 vec3_yyx(Vec3 v);

void vec3_print(Vec3 v);

Color color_new_mono(float x);

Matrix matrix_identity();
Matrix matrix_fromXRotation(float angle);
Matrix matrix_fromYRotation(float angle);
Matrix matrix_fromZRotation(float angle);
Matrix matrix_fromTranslation(Vec3 v);

void matrix_init_fromScale(Matrix *mat, float x);
Vec3 vec3_transform(Matrix *m, Vec3 v);
//void vec3_transform(Matrix *m, Vec3 *v);
Vec3 vec3_transform_direction(Matrix *mat, Vec3 d);
Vec4 vec4_transform(Matrix *m, Vec4 v);

Vec3 matrix_get_euler_angles(const Matrix *mat);


void matrix_mult(Matrix *a, Matrix *b);
Matrix matrix_inverse_trs(const Matrix *mat);


Vec4 vec3_transform_to_vec4(Matrix m, Vec3 v);

