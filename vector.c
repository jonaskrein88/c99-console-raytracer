#include "vector.h"
#include <math.h>
#include <stdio.h>

float float_sign(float x) {
	return x < 0 ? -1.f : 1.f;
}

Vec3 vec3_up() { return (Vec3){ 0.0, 1.0, 0.0 }; }

Vec3 vec3_add(Vec3 a, Vec3 b){
    return (Vec3){ a.x+b.x, a.y+b.y, a.z+b.z };
}
Vec3 vec3_sub(Vec3 a, Vec3 b){
    return (Vec3){ a.x-b.x, a.y-b.y, a.z-b.z };
}
Vec3 vec3_negate(Vec3 v){
    return (Vec3){ -v.x, -v.y, -v.z };
}
Vec3 vec3_mul(Vec3 v, float s){
    return (Vec3){ s*v.x, s*v.y, s*v.z };
}
Vec3 vec3_mul_vec3(Vec3 a, Vec3 b){
    return (Vec3){ a.x*b.x, a.y*b.y, a.z*b.z };
}
Vec3 vec3_div(Vec3 v, float s){
    return vec3_mul(v, 1.0/s);
}
Vec3 vec3_pow(Vec3 v, float s)
{
	float x = powf(v.x, s);
	float y = powf(v.y, s);
	float z = powf(v.z, s);
	return (Vec3){ x,y,z };
}
float vec3_magnitude(Vec3 v){
    return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
}
float vec3_distance(Vec3 a, Vec3 b){
    return vec3_magnitude(vec3_sub(a,b));
}
float vec3_distance2(Vec3 a, Vec3 b){
    Vec3 dif = vec3_sub(a,b);
	return vec3_dot(dif,dif);
}
Vec3 vec3_normalize(Vec3 v){
    float mag = vec3_magnitude(v);
    return vec3_div(v,mag);
}
float vec3_dot(Vec3 a, Vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    float x = a.y * b.z - a.z * b.y;
    float y = a.z * b.x - a.x * b.z;
    float z = a.x * b.y - a.y * b.x;
    return (Vec3){ x,y,z };
}
Vec3 vec3_reflect(Vec3 I, Vec3 N) {
    float dot = I.x * N.x + I.y * N.y + I.z * N.z;
    Vec3 R;
    R.x = I.x - 2.0f * dot * N.x;
    R.y = I.y - 2.0f * dot * N.y;
    R.z = I.z - 2.0f * dot * N.z;
    return R;
}

Vec3 vec3_abs(Vec3 v){
    return (Vec3){ abs(v.x), abs(v.y), abs(v.z) };
}

Vec3 vec3_saturate(Vec3 v)
{
	float x = fmin(1.f,fmax(0.f,v.x));
    float y = fmin(1.f,fmax(0.f,v.y));
    float z = fmin(1.f,fmax(0.f,v.z));
	return (Vec3){ x,y,z };
}

Vec3 vec3_quantize(Vec3 v, int bins)
{
	bins -= 1;
	float x = roundf(v.x * bins) / bins;
	float y = roundf(v.y * bins) / bins;
	float z = roundf(v.z * bins) / bins;
	return (Vec3){ x,y,z };
}


Vec3 vec3_lerp(Vec3 a, Vec3 b, float t){
	Vec3 out = {0};
	out.x = a.x + t * (b.x - a.x);
	out.y = a.y + t * (b.y - a.y);
	out.z = a.z + t * (b.z - a.z);
	return out;
}



// Perceptually weighted distance calculation to preserve color channels
float vec3_perceptual_distance(Color c1, Color c2) {
    float dr = c1.x - c2.x;
    float dg = c1.y - c2.y;
    float db = c1.z - c2.z;
    
    // Human eyes are highly sensitive to Green, moderately to Red, least to Blue
    return (dr * dr * 0.299f) + (dg * dg * 0.587f) + (db * db * 0.114f);
}

Vec3 vec3_xyy(Vec3 v){
	return (Vec3) {v.x,v.y,v.y};
}
Vec3 vec3_yxy(Vec3 v){
	return (Vec3) {v.y,v.x,v.y};
}
Vec3 vec3_yyx(Vec3 v){
	return (Vec3) {v.y,v.y,v.x};
}









void vec3_print(Vec3 v){
    printf("Vec3 %.4f, %.4f, %.4f\n", v.x,v.y,v.z);
}



Color color_new_mono(float x){
	return (Color) {x,x,x};
}




/*


	void operator+=(const Vec3& v) {
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
	}
	Vec3 operator-() const {
		return Vec3{ -x, -y,-z };
	}
	Vec3 operator-(const Vec3 &v) const {
		return Vec3{ *this + (-v)};
	}
	Vec3 operator*(float v) const {
		return Vec3{ x * v, y * v, z * v };
	}
	Vec3 operator*(const Vec3& v) const {
		return Vec3{ x * v.x, y * v.y, z * v.z };
	}
	void operator*=(float v) {
		x *= v; y *= v; z *= v;
	}
	bool operator==(const Vec3& v) {
		return v.x==x && v.y==y && v.z==z;
	}
	Vec3 operator/(float v) const {
		return Vec3{ x / v, y / v, z / v };
	}
	float magnitude() const {
		return sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
	}
	float distance(const Vec3& v) const {
		return (*this + v).magnitude();
	}
	Vec3 normalize() const { 
		return *this / magnitude();
	}
	Vec3 cross(const Vec3 &v) const {
		float x = this->y * v.z - this->z * v.y;
		float y = this->z * v.x - this->x * v.z;
		float z = this->x * v.y - this->y * v.x;
		return Vec3{ x,y,z };
	}
	float dot(const Vec3& v) const{
		return v.x * x + v.y * y + v.z * z;
	}
	float dot2() const {
		return this->dot(*this);
	}
	Vec3 abs() const {
		return Vec3{ std::abs(x), std::abs(y), std::abs(z) };
	}

    */




Matrix matrix_identity() {
	Matrix m = {0};
    m.m[0] = 1.0f;
    m.m[5] = 1.0f;
    m.m[10] = 1.0f;
    m.m[15] = 1.0f;
	return m;
}
Matrix matrix_fromXRotation(float angle) {
	Matrix mat = matrix_identity();
    mat.m[5]  = cosf(angle);
	mat.m[6]  = -sinf(angle);
	mat.m[9]  = sinf(angle);
	mat.m[10] = cosf(angle);
	return mat;
}

Matrix matrix_fromYRotation(float angle) {
    Matrix mat = matrix_identity();
    mat.m[0] = cosf(angle);
    mat.m[2] = -sinf(angle); 
    mat.m[8] = sinf(angle); 
    mat.m[10] = cosf(angle);
    return mat;
}

Matrix matrix_fromZRotation(float angle) {
	Matrix mat = matrix_identity();
    mat.m[0] = cosf(angle);
	mat.m[1] = -sinf(angle);
	mat.m[4] = sinf(angle);
	mat.m[5] = cosf(angle);
	return mat;
}

Matrix matrix_fromTranslation(Vec3 v) {
	Matrix mat = matrix_identity();
    mat.m[3] = v.x;
	mat.m[7] = v.y;
	mat.m[11] = v.z;
	return mat;
}
Matrix matrix_fromScale(float x) {
	Matrix mat = matrix_identity();
    mat.m[0] = x;
	mat.m[5] = x;
	mat.m[10] = x;
    return mat;
}

Vec3 vec3_transform(Matrix *m, Vec3 v) {
    float x = m->m[0] * v.x + m->m[1] * v.y + m->m[2] * v.z + m->m[3];
    float y = m->m[4] * v.x + m->m[5] * v.y + m->m[6] * v.z + m->m[7];
    float z = m->m[8] * v.x + m->m[9] * v.y + m->m[10] * v.z + m->m[11];
    return (Vec3){ x,y,z };
}


Vec3 vec3_transform_direction(Matrix *mat, Vec3 d) {
    Vec3 res;
    res.x = mat->m[0]*d.x + mat->m[1]*d.y + mat->m[2]*d.z;
    res.y = mat->m[4]*d.x + mat->m[5]*d.y + mat->m[6]*d.z;
    res.z = mat->m[8]*d.x + mat->m[9]*d.y + mat->m[10]*d.z;
    return res;
}







Vec4 vec4_transform(Matrix *m, Vec4 v) {
    Vec4 result;
    
    result.x = m->m[0]  * v.x + m->m[1]  * v.y + m->m[2]  * v.z + m->m[3]  * v.w;
    result.y = m->m[4]  * v.x + m->m[5]  * v.y + m->m[6]  * v.z + m->m[7]  * v.w;
    result.z = m->m[8]  * v.x + m->m[9]  * v.y + m->m[10] * v.z + m->m[11] * v.w;
    result.w = m->m[12] * v.x + m->m[13] * v.y + m->m[14] * v.z + m->m[15] * v.w;
    
    return result;
}






Vec3 matrix_get_euler_angles(const Matrix *mat) {
    Vec3 angles;
	float m00 = mat->m[0];
    float m10 = mat->m[1];
    float m20 = mat->m[2];
    
    float m01 = mat->m[4];
    float m11 = mat->m[5];
    
    float m02 = mat->m[8];
    float m12 = mat->m[9];
    float m22 = mat->m[10];

    float sin_pitch = -m12;
    
    if (sin_pitch < -1.0f) sin_pitch = -1.0f;
    if (sin_pitch > 1.0f)  sin_pitch = 1.0f;
    angles.x = asinf(sin_pitch);

    if (fabsf(m12) < 0.99999f) {
        angles.y = atan2f(m02, m22); // Yaw (Y)
        angles.z = atan2f(m10, m11); // Roll (Z)
    } else {
        angles.y = atan2f(-m20, m00); 
        angles.z = 0.0f;              
    }

    return angles;
}





void matrix_mult(Matrix *a, Matrix *b) {
	Matrix out;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            out.m[r * 4 + c] = 
                a->m[r * 4 + 0] * b->m[0 * 4 + c] +
                a->m[r * 4 + 1] * b->m[1 * 4 + c] +
                a->m[r * 4 + 2] * b->m[2 * 4 + c] +
                a->m[r * 4 + 3] * b->m[3 * 4 + c];
        }
    }
    *a = out;
}


Matrix matrix_inverse_trs_old(const Matrix *mat) {
    Matrix inv = {0};

    // 1. Transpose the 3x3 Rotation block (Flips rows and columns)
    inv.m[0] = mat->m[0]; inv.m[1] = mat->m[4]; inv.m[2] = mat->m[8];
    inv.m[4] = mat->m[1]; inv.m[5] = mat->m[5]; inv.m[6] = mat->m[9];
    inv.m[8] = mat->m[2]; inv.m[9] = mat->m[6]; inv.m[10] = mat->m[10];

    // Extract the original translation vector components (Indices 3, 7, 11)
    float tx = mat->m[3];
    float ty = mat->m[7];
    float tz = mat->m[11];

    // 2. Compute the new translation column via dot products multiplied by -1
    inv.m[3]  = -(tx * inv.m[0] + ty * inv.m[1] + tz * inv.m[2]);
    inv.m[7]  = -(tx * inv.m[4] + ty * inv.m[5] + tz * inv.m[6]);
    inv.m[11] = -(tx * inv.m[8] + ty * inv.m[9] + tz * inv.m[10]);

    // 3. Set the immutable bottom row
    inv.m[12] = 0.0f;
    inv.m[13] = 0.0f;
    inv.m[14] = 0.0f;
    inv.m[15] = 1.0f;

    return inv;
}
#include <math.h>

Matrix matrix_inverse_trs(const Matrix *mat) {
    Matrix inv = {0};

    // 1. Calculate the squared scale factors from the original matrix columns
    float sx_sq = mat->m[0]*mat->m[0] + mat->m[4]*mat->m[4] + mat->m[8]*mat->m[8];
    float sy_sq = mat->m[1]*mat->m[1] + mat->m[5]*mat->m[5] + mat->m[9]*mat->m[9];
    float sz_sq = mat->m[2]*mat->m[2] + mat->m[6]*mat->m[6] + mat->m[10]*mat->m[10];

    // Avoid division by zero if an object is scaled to 0
    float inv_sx_sq = (sx_sq > 1e-6f) ? 1.0f / sx_sq : 0.0f;
    float inv_sy_sq = (sy_sq > 1e-6f) ? 1.0f / sy_sq : 0.0f;
    float inv_sz_sq = (sz_sq > 1e-6f) ? 1.0f / sz_sq : 0.0f;

    // 2. Transpose and divide by squared scale
    inv.m[0] = mat->m[0] * inv_sx_sq;  inv.m[1] = mat->m[4] * inv_sx_sq;  inv.m[2] = mat->m[8] * inv_sx_sq;
    inv.m[4] = mat->m[1] * inv_sy_sq;  inv.m[5] = mat->m[5] * inv_sy_sq;  inv.m[6] = mat->m[9] * inv_sy_sq;
    inv.m[8] = mat->m[2] * inv_sz_sq;  inv.m[9] = mat->m[6] * inv_sz_sq;  inv.m[10] = mat->m[10] * inv_sz_sq;

    // Extract original translation components
    float tx = mat->m[3];
    float ty = mat->m[7];
    float tz = mat->m[11];

    // 3. Compute the new translation column using the scaled inverse rows
    inv.m[3]  = -(tx * inv.m[0] + ty * inv.m[1] + tz * inv.m[2]);
    inv.m[7]  = -(tx * inv.m[4] + ty * inv.m[5] + tz * inv.m[6]);
    inv.m[11] = -(tx * inv.m[8] + ty * inv.m[9] + tz * inv.m[10]);

    // 4. Set the immutable bottom row
    inv.m[12] = 0.0f;
    inv.m[13] = 0.0f;
    inv.m[14] = 0.0f;
    inv.m[15] = 1.0f;

    return inv;
}





Vec4 vec3_transform_to_vec4(Matrix m, Vec3 v) {
    Vec4 result;
    result.x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*v.z + m.m[3]*1.0f;
    result.y = m.m[4]*v.x + m.m[5]*v.y + m.m[6]*v.z + m.m[7]*1.0f;
    result.z = m.m[8]*v.x + m.m[9]*v.y + m.m[10]*v.z + m.m[11]*1.0f;
    result.w = m.m[12]*v.x + m.m[13]*v.y + m.m[14]*v.z + m.m[15]*1.0f;
    return result;
}