#pragma once

#include "typedefs.h"
#include <math.h>

//Constants:

#define F_PI 3.14159265358979323846f
#define F_2PI 2.0f * F_PI
#define F_PI_OVER2 F_PI / 2.0f
#define F_PI_OVER4 F_PI / 4.0f
#define F_1OVER_PI 1.0f / F_PI
#define F_1OVER_2PI 1.0f / F_2PI

#define F_SQRT_2 1.41421356237309504880f
#define F_SQRT_3 1.73205080756887729352f
#define F_SQRT_1OVER2 0.70710678118654752440f
#define F_SQRT_1OVER3 0.57735026918962576450f

#define F_2RAD_FACTOR F_PI / 180.0f
#define F_2DEG_FACTOR 180.0f / F_PI

#define F_TO_MS_FACTOR 1000.0f
#define F_TO_S_FACTOR 0.001f

#define F_INFINITY 1e30f //no number should be bigger than this
#define F_EPSILON 1.192092896e-07f //smallest number such that 0.0f + EPSILON != 0


static inline u32 f_Log10(const u64 number) {
	u32 ret;
	if (number>= 100000000) { //9 digits or more
		if (number >=1000000000000) { //13 digits or more
			if (number >=10000000000000000) { //17 to 20 digits
				if (number >=1000000000000000000) {
					ret = (number >= 10000000000000000000) ? 20 : 19;
				} else {
					ret = number >= 100000000000000000 ? 18 : 17;
				}
			} else { //13 to 16 digits
				if (number>=100000000000000) {
					ret = number>=1000000000000000 ? 16 : 15;
				} else {
					ret = number>=10000000000000 ? 14 : 13;
				}
			}
		} else { //9 to 12digits
			if (number>=10000000000) {
				ret = number>=100000000000 ? 12 : 11;
			} else {
				ret = number>=1000000000 ? 10 : 9;
			}
		}
	} else { //8 digits or less
		if (number>=10000) { // 5 to 8digits
			if (number>=1000000) {
				ret = number>=10000000 ? 8 : 7;
			} else {
				ret = number>=100000 ? 6 : 5;
			}
		} else {
			if (number>=100) { //1 to 4 digits
				ret = number>=1000 ? 4 : 3;
			} else {
				ret = number>=10 ? 2 : 1;
			}
		}
	}
	return(ret);
}

static inline bool isPow2(u64 value) {
	return ((value !=0) && (value &(value-1))==0);
};

static inline void swap(r32* a, r32* b) {
	r32 temp = *b;
	*b = *a;
	*a = temp;
}

static inline r32 radians(r32 degrees) {
	degrees *= F_2RAD_FACTOR;
	return(degrees);
}

static inline r32 AbsR32(r32 n) {
	return (n<0.0f ? -n : n);
}

static inline s32 AbsS32(s32 n) {
	return (n<0 ? -n : n);
}

static inline u32 MinU32(u32 a, u32 b) {
	return(a < b ? a : b);
}
static inline u32 MaxU32(u32 a, u32 b) {
	return(a > b ? a : b);
}
static inline s32 MinS32(s32 a, s32 b) {
	return(a < b ? a : b);
}
static inline s32 MaxS32(s32 a, s32 b) {
	return(a > b ? a : b);
}
static inline r32 MinR32(r32 a, r32 b) {
	return(a < b ? a : b);
}
static inline r32 MaxR32(r32 a, r32 b) {
	return(a > b ? a : b);
}

// Linear Algebra:

//vectors:

typedef union vec2 {
	r32 values[2];
	struct {
		union {
			r32 x;
			r32 r;
			r32 u;
			r32 s;
			r32 width;
			r32 right;
		};
		union {
			r32 y;
			r32 g;
			r32 v;
			r32 t;
			r32 height;
			r32 down;
		};
	};
}vec2;

typedef union ivec2 {
	s32 values[2];
	struct {
		union {
			s32 x;
			s32 r;
			s32 u;
			s32 s;
			s32 width;
			s32 right;
		};
		union {
			s32 y;
			s32 g;
			s32 v;
			s32 t;
			s32 height;
			s32 down;
		};
	};
}ivec2;

typedef union uvec2 {
	u32 values[2];
	struct {
		union {
			u32 x;
			u32 r;
			u32 u;
			u32 s;
			u32 width;
			u32 right;
		};
		union {
			u32 y;
			u32 g;
			u32 v;
			u32 t;
			u32 height;
			u32 down;
		};
	};
}uvec2;

static inline vec2 vec2_add(vec2 v1, vec2 v2);
static inline vec2 vec2_sub(vec2 v1, vec2 v2);
static inline vec2 vec2_mul(vec2 v1, vec2 v2);
static inline vec2 vec2_div(vec2 v1, vec2 v2);
static inline r32 vec2_dot(vec2 v1, vec2 v2);

static inline vec2 vec2_scale(vec2 v, r32 s);
static inline r32 vec2_magSq(vec2 v);
static inline r32 vec2_mag(vec2 v);
static inline vec2 vec2_norm(vec2 v);

static inline bool ivec2_IsEqual(ivec2 v1, ivec2 v2);
static inline ivec2 ivec2_add(ivec2 v1, ivec2 v2);
static inline ivec2 ivec2_sub(ivec2 v1, ivec2 v2);
static inline ivec2 ivec2_mul(ivec2 v1, ivec2 v2);

typedef union vec3 {
	r32 values[3];
	struct {
		union {
			r32 x;
			r32 r;
			r32 u;
			r32 s;
			r32 width;
			r32 front;
		};
		union {
			r32 y;
			r32 g;
			r32 v;
			r32 t;
			r32 height;
			r32 right;
		};
		union {
			r32 z;
			r32 b;
			r32 w;
			r32 p;
			r32 depth;
			r32 up;
		};
	};
}vec3;

typedef union ivec3 {
	s32 values[3];
	struct {
		union {
			s32 x;
			s32 r;
			s32 u;
			s32 s;
			s32 width;
			s32 front;
		};
		union {
			s32 y;
			s32 g;
			s32 v;
			s32 t;
			s32 height;
			s32 right;
		};
		union {
			s32 z;
			s32 b;
			s32 w;
			s32 p;
			s32 depth;
			s32 up;
		};
	};
} ivec3;

typedef union uvec3 {
	s32 values[3];
	struct {
		union {
			u32 x;
			u32 r;
			u32 u;
			u32 s;
			u32 width;
			u32 front;
		};
		union {
			u32 y;
			u32 g;
			u32 v;
			u32 t;
			u32 height;
			u32 right;
		};
		union {
			u32 z;
			u32 b;
			u32 w;
			u32 p;
			u32 depth;
			u32 up;
		};
	};
} uvec3;

static inline vec3 vec3_add(vec3 v1, vec3 v2);
static inline vec3 vec3_sub(vec3 v1, vec3 v2);
static inline vec3 vec3_mul(vec3 v1, vec3 v2);
static inline vec3 vec3_div(vec3 v1, vec3 v2);
static inline r32 vec3_dot(vec3 v1, vec3 v2);
static inline vec3 cross(vec3 v1, vec3 v2);

static inline vec3 vec3_scale(vec3 v, r32 s);
static inline r32 vec3_magSq(vec3 v);
static inline r32 vec3_mag(vec3 v);
static inline vec3 vec3_norm(vec3 v);




typedef union vec4 {
	r32 values[4];
	struct {
		union {
			r32 x;
			r32 r;
			r32 s;
		};
		union {
			r32 y;
			r32 g;
			r32 t;
		};
		union {
			r32 z;
			r32 b;
			r32 p;
		};
		union {
			r32 w;
			r32 a;
			r32 q;
		};
	};
}vec4;

typedef union ivec4 {
	s32 values[4];
	struct {
		union {
			s32 x;
			s32 r;
			s32 s;
		};
		union {
			s32 y;
			s32 g;
			s32 t;
		};
		union {
			s32 z;
			s32 b;
			s32 p;
		};
		union {
			s32 w;
			s32 a;
			s32 q;
		};
	};
}ivec4;

typedef union uvec4 {
	u32 values[4];
	struct {
		union {
			u32 x;
			u32 r;
			u32 s;
		};
		union {
			u32 y;
			u32 g;
			u32 t;
		};
		union {
			u32 z;
			u32 b;
			u32 p;
		};
		union {
			u32 w;
			u32 a;
			u32 q;
		};
	};
}uvec4;

static inline vec4 vec4_add(vec4 v1, vec4 v2);
static inline vec4 vec4_sub(vec4 v1, vec4 v2);
static inline vec4 vec4_mul(vec4 v1, vec4 v2);
static inline vec4 vec4_div(vec4 v1, vec4 v2);
static inline r32 vec4_dot(vec4 v1, vec4 v2);

static inline vec4 vec4_scale(vec4 v, r32 s);
static inline r32 vec4_magSq(vec4 v);
static inline r32 vec4_mag(vec4 v);
static inline vec4 vec4_norm(vec4 v);


//Quaternions:

typedef struct quat {
	r32 w;
	r32 i;
	r32 j;
	r32 k;
}quat ;

typedef struct dquat {
	r32 w;
	r32 i;
	r32 j;
	r32 k;
	r32 x;
	r32 y;
	r32 z;
	r32 screw;
} dquat;

//Matrices:

typedef union mat2 {
	r32 data[4];
	r32 entry[2][2];
	vec2 columns[2];
} mat2;
typedef union mat3 {
	r32 data[9];
	r32 entry[3][3];
	vec3 columns[3];
} mat3;
typedef union mat4 {
	r32 data[16];
	r32 entry[4][4];
	vec4 columns[4];
} mat4;

static inline mat2 identityMat2(void);

static inline mat2 mat2_mul(mat2 m1, mat2 m2);
static inline vec2 mat2_mul_vec2(mat2 m, vec2 v);
static inline mat2 mat2_mul_scalar(mat2 m, r32 s);

static inline mat2 mat2_add(mat2 m1, mat2 m2);

static inline r32 mat2_det(mat2 m);
static inline mat2 mat2_transpose(mat2 m);




static inline mat3 identityMat3(void);

static inline mat3 mat3_mul(mat3 m1, mat3 m2);
static inline vec3 mat3_mul_vec3(mat3 m, vec3 v);
static inline mat3 mat3_mul_scalar(mat3 m, r32 s);

static inline mat3 mat3_add(mat3 m1, mat3 m2);

static inline r32 mat3_det(mat3 m);
static inline mat3 mat3_transpose(mat3 m);
static inline mat2 mat3_minor(mat3 m, r32 c, r32 r);
static inline r32 mat3_cofactor(mat3, r32 c, r32 r);
static inline mat3 mat3_inverse(mat3 m);




static inline mat4 identityMat4(void);
static inline mat4 mat4_mul(m1, m2);
static inline vec4 mat4_mul_vec4(mat4 m, vec4 v);
static inline mat4 mat4_mul_scalar(mat4 m, r32 s);

static inline mat4 mat4_add(mat4 m1, mat4 m2);

static inline r32 mat4_det(mat4 m);
static inline mat4 mat4_transpose(mat4 m);
static inline mat3 mat4_minor(mat4 m, r32 c, r32 r);
static inline r32 mat4_cofactor(mat4 m, r32 c, r32 r);
static inline mat4 mat4_inverse(mat4 m);

static inline mat4 model(vec3 t, quat o, vec3 s);
static inline mat4 view(vec3 t, quat o);
static inline mat4 inverseView(vec3 t, quat o);

//creates a perspective matrix with infinite far plane for a depth buffer with 1 = close, 0 = far
static inline mat4 perspective(r32 aspectRatio, r32 vFoV, r32 nearPlane);
static inline mat4 inversePerspective(mat4 perspective);






//Implementation:

static inline vec2 vec2_add(vec2 v1, vec2 v2) {
	vec2 s = {
		.x = v1.x+v2.x,
		.y = v1.y+v2.y,
	};
	return(s);
}

//returns a vector pointing from v2 to v1
static inline vec2 vec2_sub(vec2 v1, vec2 v2) {
	vec2 d = {
		.x = v1.x-v2.x,
		.y = v1.y-v2.y,
	};
	return(d);
}

static inline vec2 vec2_mul(vec2 v1, vec2 v2) {
	vec2 p = {
		.x = v1.x*v2.x,
		.y = v1.y*v2.y,
	};
	return(p);
}

static inline vec2 vec2_div(vec2 v1, vec2 v2) {
	vec2 q = {
		.x = v1.x/v2.x,
		.y = v1.y/v2.y,
	};
	return(q);
}

static inline r32 vec2_dot(vec2 v1, vec2 v2) {
	return(v1.x*v2.x + v1.y*v2.y);
}

static inline vec2 vec2_scale(vec2 v, r32 s) {
	v.x = s*v.x;
	v.y = s*v.y;
	return(v);
}

static inline r32 vec2_magSq(vec2 v) {
	return(vec2_dot(v, v));
}
static inline r32 vec2_mag(vec2 v) {
	r32 mag = sqrt(vec2_magSq(v));
	return(mag);
}

static inline vec2 vec2_norm(vec2 v) {
	r32 mag = vec2_mag(v);
	return (vec2_scale(v, 1.0f/mag));
}

static inline bool ivec2_IsEqual(ivec2 v1, ivec2 v2) {
	return((v1.x==v2.x)&&(v1.y==v2.y));

}

static inline ivec2 ivec2_add(ivec2 v1, ivec2 v2) {
	ivec2 s = {
		.x = v1.x+v2.x,
		.y = v1.y+v2.y,
	};
	return(s);
}

static inline ivec2 ivec2_sub(ivec2 v1, ivec2 v2) {
	ivec2 d = {
		.x = v1.x-v2.x,
		.y = v1.y-v2.y,
	};
	return(d);
}

static inline ivec2 ivec2_mul(ivec2 v1, ivec2 v2) {
	ivec2 d = {
		.x = v1.x*v2.x,
		.y = v1.y*v2.y,
	};
	return(d);
}

static inline float ivec2_mag(ivec2 v) {
	r32 mag = vec2_mag((vec2) { (r32)v.x, (r32)v.y });
	return(mag);
}




static inline vec3 vec3_add(vec3 v1, vec3 v2) {
	vec3 s = {
		.x = v1.x+v2.x,
		.y = v1.y+v2.y,
		.z = v1.z+v2.z,
	};
	return(s);
}

static inline vec3 vec3_sub(vec3 v1, vec3 v2) {
	vec3 d = {
		.x = v1.x-v2.x,
		.y = v1.y-v2.y,
		.z = v1.z-v2.z,
	};
	return(d);
}

static inline vec3 vec3_mul(vec3 v1, vec3 v2) {
	vec3 p = {
		.x = v1.x*v2.x,
		.y = v1.y*v2.y,
		.z = v1.z*v2.z,
	};
	return(p);
}

static inline vec3 vec3_div(vec3 v1, vec3 v2) {
	vec3 q = {
		.x = v1.x/v2.x,
		.y = v1.y/v2.y,
		.z = v1.z/v2.z,
	};
	return(q);
}

static inline r32 vec3_dot(vec3 v1, vec3 v2) {
	return(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

static inline vec3 cross(vec3 v1, vec3 v2) {
	vec3 c = {
	.x = v1.y*v2.z - v1.z*v2.y,
	.y = v1.z*v2.x - v1.x*v2.z,
	.z = v1.x*v2.y - v1.y*v2.x,
	};
	return(c);
}


static inline vec3 vec3_scale(vec3 v, r32 s) {
	v.x = s*v.x;
	v.y = s*v.y;
	v.z = s*v.z;
	return(v);
}

static inline r32 vec3_magSq(vec3 v) {
	return(vec3_dot(v, v));
}

static inline r32 vec3_mag(vec3 v) {
	r32 mag = sqrt(vec3_magSq(v));
	return(mag);
}

static inline vec3 vec3_norm(vec3 v) {
	r32 mag = vec3_mag(v);
	return (vec3_scale(v, 1.0f/mag));
}

static inline bool ivec3_IsEqual(ivec3 v1, ivec3 v2) {
	return((v1.x==v2.x)&&(v1.y==v2.y)&&(v1.z==v2.z));
}

static inline vec4 vec4_add(vec4 v1, vec4 v2) {
	vec4 s = {
		.x = v1.x+v2.x,
		.y = v1.y+v2.y,
		.z = v1.z+v2.z,
		.w = v1.w+v2.w,
	};
	return(s);
}

static inline vec4 vec4_sub(vec4 v1, vec4 v2) {
	vec4 d = {
		.x = v1.x-v2.x,
		.y = v1.y-v2.y,
		.z = v1.z-v2.z,
		.w = v1.w-v2.w,
	};
	return(d);
}

static inline vec4 vec4_mul(vec4 v1, vec4 v2) {
	vec4 p = {
		.x = v1.x*v2.x,
		.y = v1.y*v2.y,
		.z = v1.z*v2.z,
		.w = v1.w*v2.w,
	};
	return(p);
}

static inline vec4 vec4_div(vec4 v1, vec4 v2) {
	vec4 q = {
		.x = v1.x/v2.x,
		.y = v1.y/v2.y,
		.z = v1.z/v2.z,
		.w = v1.w/v2.w
	};
	return(q);
}

static inline r32 vec4_dot(vec4 v1, vec4 v2) {
	return(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);
}

static inline vec4 vec4_scale(vec4 v, r32 s) {
	v.x = s*v.x;
	v.y = s*v.y;
	v.z = s*v.z;
	v.w = s*v.w;
	return(v);
}

static inline r32 vec4_magSq(vec4 v) {
	return(vec4_dot(v, v));
}
static inline r32 vec4_mag(vec4 v) {
	r32 mag = sqrt(vec4_magSq(v));
	return(mag);
}

static inline vec4 vec4_norm(vec4 v) {
	r32 mag = vec4_mag(v);
	return (vec4_scale(v, 1.0f/mag));
}

static inline vec2 vec3to2(vec3 v) {
	vec2 v_out = { v.x, v.y };
	return(v_out);
}
static inline vec3 vec2to3(vec2 v, r32 z) {
	vec3 v_out = { v.x, v.y, z };
	return(v_out);
}
static inline vec3 vec4to3(vec4 v) {
	vec3 v_out = { v.x, v.y, v.z };
	return(v_out);
}
static inline vec4 vec3to4(vec3 v, r32 w) {
	vec4 v_out = { v.x, v.y, v.z, w };
	return(v_out);
}

#define WorldForward (vec3) {1.0f, 0.0f, 0.0f}
#define WorldRight (vec3) {0.0f, -1.0f, 0.0f}
#define WorldUp (vec3) {0.0f, 0.0f, 1.0f}


static inline quat conjugate(quat q) {
	q.i *= -1;
	q.j *= -1;
	q.k *= -1;
	return(q);
}

static inline quat quat_add(quat q1, quat q2) {
	quat s = {
		.w = q1.w+q2.w,
		.i = q1.i+q2.i,
		.j = q1.j+q2.j,
		.k = q1.k+q2.k,
	};
	return(s);
}

static inline quat quat_sub(quat q1, quat q2) {
	quat d = {
		.w = q1.w-q2.w,
		.i = q1.i-q2.i,
		.j = q1.j-q2.j,
		.k = q1.k-q2.k,
	};
	return(d);
}

static inline quat quat_mul(quat q1, quat q2) {
	quat p = {
		.w = q1.w*q2.w - q1.i*q2.i - q1.j*q2.j - q1.k*q2.k,
		.i = q1.w*q2.i + q1.i*q2.w + q1.j*q2.k - q1.k*q2.j,
		.j = q1.w*q2.j - q1.i*q2.k + q1.j*q2.w + q1.k*q2.i,
		.k = q1.w*q2.k + q1.i*q2.j - q1.j*q2.i + q1.k*q2.w
	};
	return(p);
}

//stub: returns a 0 quaternion
static inline quat quat_div(quat q1, quat q2) {
	quat q = { 0 };
	return(q);
}

static inline r32 quat_mag(quat q) {
	return(sqrt(q.w*q.w + q.i*q.i + q.j*q.j + q.k*q.k));
}
static inline quat quat_norm(quat q) {
	r32 m = quat_mag(q);
	q.w /= m;
	q.i /= m;
	q.j /= m;
	q.k /= m;
	return(q);
}
//unit quaternion

//add optimzed codepath >.>
static inline vec3 quat_mul_vec3DEBUG(quat q, vec3 v) {
	quat qPrime = conjugate(q);
	quat pureV = {0, v.x, v.y, v.z};

	quat r = quat_mul(quat_mul(q, pureV), qPrime);
	vec3 rPrime = { r.i, r.j, r.k };
	return(rPrime);
}

static inline vec3 quat_mul_vec3(quat q, vec3 v) {
	vec3 u = {q.i, q.j, q.k};
	r32 s = q.w;
	vec3 r	= vec3_scale(u, (2.0f*vec3_dot(u, v)));
	r		= vec3_add(r, vec3_scale(v, s*s-vec3_dot(u, u)));
	r		= vec3_add(r, vec3_scale(cross(u, v), 2.0f*s));
	return(r);
}

static inline quat axisAngle(r32 angle, vec3 axis) {
	const r32 s = sinf(angle*0.5f);
	axis = vec3_norm(axis);
	axis = vec3_scale(axis, s);
	quat q = {
		.w = cosf(angle*0.5f),
		.i = axis.x,
		.j = axis.y,
		.k = axis.z};
	return(q);
}

// q = unit quaternion
static inline mat4 quatToMat4(quat q) {
	mat4 r = { 0 };
	const r32 x = q.i*q.i;
	const r32 y = q.j*q.j;
	const r32 z = q.k*q.k;
	const r32 wx = q.w*q.i;
	const r32 wy = q.w*q.j;
	const r32 wz = q.w*q.k;
	const r32 xy = q.i*q.j;
	const r32 xz = q.i*q.k;
	const r32 yz = q.j*q.k;
	
	r.columns[0] = (vec4)	{1.0f-2.0f*(y+z)	,2.0f*(xy+wz)		,2.0f*(xz-wy)		,0.0f};
	r.columns[1] = (vec4)	{2.0f*(xy-wz)		,1.0f-2.0f*(x+z)	,2.0f*(yz+wx)		,0.0f};
	r.columns[2] = (vec4)	{2.0f*(xz+wy)		,2.0f*(yz-wx)		,1.0f-2.0f*(x+y)	,0.0f};
	r.columns[3] = (vec4)	{0.0f				,0.0f				,0.0f				,1.0f};

	return(r);
}


static inline mat2 identityMat2(void) {
	mat2 m = { 0.0f };
	m.entry[0][0] = 1.0f;
	m.entry[1][1] = 1.0f;
	return(m);
}

static inline mat2 mat2_mul(mat2 m1, mat2 m2) {
	mat2 m_out;
	m_out.columns[0] = vec2_add(vec2_scale(m1.columns[0], m2.data[0]), vec2_scale(m1.columns[1], m2.data[1]));
	m_out.columns[1] = vec2_add(vec2_scale(m1.columns[0], m2.data[2]), vec2_scale(m1.columns[1], m2.data[3]));
	return(m_out);
}

static inline vec2 mat2_mul_vec2(mat2 m, vec2 v) {
	v = vec2_add(vec2_scale(m.columns[0], v.x), vec2_scale(m.columns[1], v.y));
	return(v);
}
static inline mat2 mat2_mul_scalar(mat2 m, r32 s) {
	m.data[0] *= s;
	m.data[1] *= s;
	m.data[2] *= s;
	m.data[3] *= s;
	return(m);
}

static inline mat2 mat2_add(mat2 m1, mat2 m2) {
	mat2 m_out;
	m_out.data[0] = m1.data[0]+m2.data[0];
	m_out.data[1] = m1.data[1]+m2.data[1];
	m_out.data[2] = m1.data[2]+m2.data[2];
	m_out.data[3] = m1.data[3]+m2.data[3];
	return(m_out);
}


static inline r32 mat2_det(mat2 m) {
	return(m.data[0]*m.data[3]-m.data[1]*m.data[2]);
}

static inline mat2 mat2_transpose(mat2 m) {
	swap(&m.entry[0][1], &m.entry[1][0]);
	return(m);
}


static inline mat3 identityMat3(void) {
	mat3 m = { 0.0f };
	m.entry[0][0] = 1.0f;
	m.entry[1][1] = 1.0f;
	m.entry[2][2] = 1.0f;
	return(m);
}

static inline mat3 mat3_mul(mat3 m1, mat3 m2) {
	mat3 m_out;
	m_out.columns[0] = vec3_add(vec3_add(vec3_scale(m1.columns[0], m2.data[0]), vec3_scale(m1.columns[1], m2.data[1])), vec3_scale(m1.columns[2], m2.data[2]));
	m_out.columns[1] = vec3_add(vec3_add(vec3_scale(m1.columns[0], m2.data[3]), vec3_scale(m1.columns[1], m2.data[4])), vec3_scale(m1.columns[2], m2.data[5]));
	m_out.columns[2] = vec3_add(vec3_add(vec3_scale(m1.columns[0], m2.data[6]), vec3_scale(m1.columns[1], m2.data[7])), vec3_scale(m1.columns[2], m2.data[8]));
	return(m_out);
}

static inline vec3 mat3_mul_vec3(mat3 m, vec3 v) {
	v = vec3_add(vec3_add(vec3_scale(m.columns[0], v.x), vec3_scale(m.columns[1], v.y)),vec3_scale(m.columns[2], v.z));
	return(v);
}
static inline mat3 mat3_mul_scalar(mat3 m, r32 s) {
	m.data[0] *= s;
	m.data[1] *= s;
	m.data[2] *= s;
	m.data[3] *= s;
	m.data[4] *= s;
	m.data[5] *= s;
	m.data[6] *= s;
	m.data[7] *= s;
	m.data[8] *= s;
	return(m);
}

static inline mat3 mat3_add(mat3 m1, mat3 m2) {
	mat3 m_out;
	m_out.data[0] = m1.data[0]+m2.data[0];
	m_out.data[1] = m1.data[1]+m2.data[1];
	m_out.data[2] = m1.data[2]+m2.data[2];
	m_out.data[3] = m1.data[3]+m2.data[3];
	m_out.data[4] = m1.data[4]+m2.data[4];
	m_out.data[5] = m1.data[5]+m2.data[5];
	m_out.data[6] = m1.data[6]+m2.data[6];
	m_out.data[7] = m1.data[7]+m2.data[7];
	m_out.data[8] = m1.data[8]+m2.data[8];

	return(m_out);
}

static inline r32 mat3_det(mat3 m) {
	const r32 i = m.data[0] * (m.data[4]*m.data[8]-m.data[5]*m.data[7]);
	const r32 j = m.data[3] * (m.data[1]*m.data[8]-m.data[2]*m.data[7]);
	const r32 k = m.data[6] * (m.data[1]*m.data[5]-m.data[2]*m.data[4]);
	return(i-j+k);
}

static inline mat3 mat3_transpose(mat3 m) {
	swap(&m.entry[0][1], &m.entry[1][0]);
	swap(&m.entry[0][2], &m.entry[2][0]);
	swap(&m.entry[1][2], &m.entry[2][1]);
	return(m);
}

static inline mat2 mat3_minor(mat3 m, r32 c, r32 r) {
	mat2 m_out;

	u32 tempRow = 0;
	u32 tempCol = 0;

	for (u32 y = 0; y<3; ++y) {
		if (y != r) {
			tempCol = 0;
			for (u32 x = 0; x<3; ++x) {
				if (x!=c) {
					m_out.entry[tempCol][tempRow] = m.entry[x][y];
					tempCol++;
				}
			}
			tempRow++;
		}
	}
	return(m_out);
}

static inline r32 mat3_cofactor(mat3 m, r32 c, r32 r) {
	mat2 minorMat = mat3_minor(m, c, r);
	r32 cf = (r32)(pow(-1, c+r))* mat2_det(minorMat);
	return(cf);
}

static inline mat3 mat3_inverse(mat3 m) {
	mat3 i;
	for (u32 r = 0; r<3; ++r) {
		for (u32 c = 0; c<3; ++c) {
			i.entry[c][r] = mat3_cofactor(m, r, c);
		}
	}
	r32 det = mat3_det(m);
	det = 1.0f/det;
	i = mat3_mul_scalar(i, det);
	return(i);
}






static inline mat4 identityMat4(void) {
	mat4 m = { 0.0f };
	m.entry[0][0] = 1.0f;
	m.entry[1][1] = 1.0f;
	m.entry[2][2] = 1.0f;
	m.entry[3][3] = 1.0f;
	return(m);
}

static inline mat4 mat4_mul(mat4 m1, mat4 m2) {
	mat4 m_out = {};
	m_out.columns[0] = vec4_add(vec4_add(vec4_scale(m1.columns[0], m2.data[0]), vec4_scale(m1.columns[1], m2.data[1])), vec4_add(vec4_scale(m1.columns[2], m2.data[2]), vec4_scale(m1.columns[3], m2.data[3])));
	m_out.columns[1] = vec4_add(vec4_add(vec4_scale(m1.columns[0], m2.data[4]), vec4_scale(m1.columns[1], m2.data[5])), vec4_add(vec4_scale(m1.columns[2], m2.data[6]), vec4_scale(m1.columns[3], m2.data[7])));
	m_out.columns[2] = vec4_add(vec4_add(vec4_scale(m1.columns[0], m2.data[8]), vec4_scale(m1.columns[1], m2.data[9])), vec4_add(vec4_scale(m1.columns[2], m2.data[10]), vec4_scale(m1.columns[3], m2.data[11])));
	m_out.columns[3] = vec4_add(vec4_add(vec4_scale(m1.columns[0], m2.data[12]), vec4_scale(m1.columns[1], m2.data[13])), vec4_add(vec4_scale(m1.columns[2], m2.data[14]), vec4_scale(m1.columns[3], m2.data[15])));
	return(m_out);
}

static inline vec4 mat4_mul_vec4(mat4 m, vec4 v) {
	v = vec4_add(vec4_add(vec4_scale(m.columns[0], v.x), vec4_scale(m.columns[1], v.y)), vec4_add(vec4_scale(m.columns[2], v.z), vec4_scale(m.columns[3], v.w)));
	return(v);
}
static inline mat4 mat4_mul_scalar(mat4 m, r32 s) {
	m.data[0] *= s;
	m.data[1] *= s;
	m.data[2] *= s;
	m.data[3] *= s;
	m.data[4] *= s;
	m.data[5] *= s;
	m.data[6] *= s;
	m.data[7] *= s;
	m.data[8] *= s;
	m.data[9] *= s;
	m.data[10] *= s;
	m.data[11] *= s;
	m.data[12] *= s;
	m.data[13] *= s;
	m.data[14] *= s;
	m.data[15] *= s;
	return(m);
}

static inline mat4 mat4_add(mat4 m1, mat4 m2) {
	m1.data[0] += m2.data[0];
	m1.data[1] += m2.data[1];
	m1.data[2] += m2.data[2];
	m1.data[3] += m2.data[3];
	m1.data[4] += m2.data[4];
	m1.data[5] += m2.data[5];
	m1.data[6] += m2.data[6];
	m1.data[7] += m2.data[7];
	m1.data[8] += m2.data[8];
	m1.data[9] += m2.data[8];
	m1.data[10] += m2.data[10];
	m1.data[11] += m2.data[11];
	m1.data[12] += m2.data[12];
	m1.data[13] += m2.data[13];
	m1.data[14] += m2.data[14];
	m1.data[15] += m2.data[15];
	return(m1);
}

static inline r32 mat4_det(mat4 m) {
	r32 det = 0.0f;
	r32 sign = 1.0f;
	for (u32 c = 0; c<4; ++c) {
		mat3 minorMat = mat4_minor(m, c, 0);
		det += m.columns[c].x* mat3_det(minorMat)*sign;
		sign *= -1.0f;
	}
	return(det);
}

static inline mat4 mat4_transpose(mat4 m) {
	swap(&m.entry[0][1], &m.entry[1][0]);
	swap(&m.entry[0][2], &m.entry[2][0]);
	swap(&m.entry[0][3], &m.entry[3][0]);
	swap(&m.entry[1][2], &m.entry[2][1]);
	swap(&m.entry[1][3], &m.entry[3][1]);
	swap(&m.entry[2][3], &m.entry[3][2]);
	return(m);
}

static inline mat3 mat4_minor(mat4 m, r32 c, r32 r) {
	mat3 m_out;

	u32 tempRow = 0;
	u32 tempCol = 0;

	for (u32 x = 0; x<4; ++x) {
		if (x != c) {
			tempRow = 0;
			for (u32 y = 0; y<4; ++y) {
				if (y!=r) {
					m_out.entry[tempCol][tempRow] = m.entry[x][y];
					tempRow++;
				}
			}
			tempCol++;
		}
	}
	return(m_out);
}

static inline r32 mat4_cofactor(mat4 m, r32 c, r32 r) {
	mat3 minorMat = mat4_minor(m, c, r);
	r32 cf = (r32)(pow(-1, c+r))* mat3_det(minorMat);
	return(cf);
}
static inline mat4 mat4_inverse(mat4 m) {
	mat4 i;
	for (u32 r = 0; r<4; ++r) {
		for (u32 c = 0; c<4; ++c) {
			i.entry[c][r] = mat4_cofactor(m, r, c);
		}
	}
	r32 det = mat4_det(m);
	det = 1.0f/det;
	i = mat4_mul_scalar(i, det);
	return(i);
}


static inline mat4 model(vec3 t, quat o, vec3 s) {
	mat4 m = quatToMat4(o);
	m.columns[0] = vec4_scale(m.columns[0], s.x);
	m.columns[1] = vec4_scale(m.columns[1], s.y);
	m.columns[2] = vec4_scale(m.columns[2], s.z);
	m.columns[3] = vec3to4(t, 1.0f);
	return(m);
}

static inline mat4 view(vec3 t, quat o) {
	t = vec3_scale(t, -1.0f);
	o = conjugate(o);
	mat4 m = quatToMat4(o);
	t = quat_mul_vec3(o, t);
	m.columns[3] = vec3to4(t, 1.0f);
	return(m);
}

static inline mat4 inverseView(vec3 t, quat o) {
	mat4 m = quatToMat4(o);
	m.columns[3] = vec3to4(t, 1.0f);
	return(m);
}

static inline mat4 perspective(r32 aspectRatio, r32 vFoV, r32 nearPlane) {
	const r32 t = tan(vFoV * 0.5f);
	const r32 scaleX = 1.0f/(t);
	const r32 scaleY = 1.0f/(t*aspectRatio);
	
	mat4 m_per = {0};
	
	m_per.entry[0][3] = 1;
	m_per.entry[1][0] = -scaleY;
	m_per.entry[2][1] = -scaleX;
	m_per.entry[3][2] = nearPlane;
	return(m_per);
}
static inline mat4 inversePerspective(mat4 m_proj) {
	mat4 m_inv = {0};

	m_inv.entry[0][1] = 1.0f/m_proj.entry[1][0];
	m_inv.entry[1][2] = 1.0f/m_proj.entry[2][1];
	m_inv.entry[2][3] = 1.0f/m_proj.entry[3][2];
	m_inv.entry[3][0] = 1.0f;
	return(m_inv);
}

static inline mat4 orthographic() {
	return(identityMat4());
}

//physics:

typedef struct Transform {
	vec3 position;
	quat orientation;
	vec3 scale;
} Transform;