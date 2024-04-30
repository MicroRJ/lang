/*
**
** Copyright(c) 2023 - Dayan Rodriguez - Dyr
**
** -+- vec -+-
**
*/

lgi_API float lgi_mix(float ratio, float min, float max) {
	return min + (max - min) * ratio;
}
lgi_API float lgi_unmix(float val, float min, float max) {
	return (val - min) / (max - min);
}
lgi_API float lgi_remix(float val, float val_min, float val_max, float min, float max) {
	return lgi_mix(lgi_unmix(val,val_min,val_max),min,max);
}
lgi_API float lgi_clamp(float val, float min, float max) {
	return val < min ? min : val > max ? max : val;
}

int iclamp(int val, int min, int max) {
	return val<min?min: val>max?max: val;
}

// #ifndef max
// float max(float x, float y) {
// 	return x>y?x:y;
// }
// float fmin(float x, float y) {
// 	return x<y?x:y;
// }
// #endif
// #ifndef min
// float min(float x, float y) {
// 	return x<y?x:y;
// }
// float fmax(float x, float y) {
// 	return x>y?x:y;
// }
// #endif


// int imax(int x, int y) {
// 	return x>y?x:y;
// }

// int imin(int x, int y) {
// 	return x<y?x:y;
// }



// typedef struct {
// 	short x,y;
// } rxvec2i16_t;

// typedef struct {
// 	int x,y;
// } rxvec2i_t;

// typedef struct {
// 	union {
// 		struct {
// 			int x,y,z;
// 		};
// 		rxvec2i_t xy;
// 	};
// } rxvec3i_t;

// rxvec3i_t  rxvec3i    (float xyz);
// rxvec3i_t  rxvec3i_xyz(float x, float y, float z);
// rxvec3i_t  rxvec3i_xy (float x, float y);
// rxvec3i_t  rxvec3i_x  (float x);
// rxvec3i_t  rxvec3i_y  (float y);
// rxvec3i_t  rxvec3i_z  (float z);

typedef struct {
	float x,y;
} vec2;

typedef struct {
	union {
		struct {
			float x,y,z;
		};
		vec2 xy;
	};
} rxvec3_t;

rxvec3_t   rxvec3    (float xyz);
rxvec3_t   rxvec3_xyz(float x, float y, float z);
rxvec3_t   rxvec3_xy (float x, float y);
rxvec3_t   rxvec3_x  (float x);
rxvec3_t   rxvec3_y  (float y);
rxvec3_t   rxvec3_z  (float z);

typedef struct {
	union {
		float e[4];
		struct {
			float x,y,z,w;
		};
		struct {
			float r,g,b,a;
		};
		rxvec3_t xyz;
		rxvec3_t rgb;
		vec2 xy;
	};
} vec4;

vec4 Vec4_xyzw(float x, float y, float z, float w);

// typedef struct {
// 	union {
// 		struct {
// 			int x,y,z,w;
// 		};
// 		struct {
// 			int r,g,b,a;
// 		};
// 		rxvec3i_t xyz;
// 		rxvec3i_t rgb;
// 		rxvec2i_t xy;
// 	};
// } rxvec4i_t;

typedef struct {
	float m[4][4];
} lgi_Matrix;

lgi_Matrix lgi_Matrix__identity();
lgi_Matrix lgi_Matrix__multiply(lgi_Matrix, lgi_Matrix);


rxvec3_t
rxvec3_xyz(
float x, float y, float z)
{
	rxvec3_t r;
	r.x = x;
	r.y = y;
	r.z = z;
	return r;
}

vec4
Vec4_xyzw(
float x, float y, float z, float w)
{
	vec4 r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;
	return r;
}

// rxvec3_t rxvec3_xy(float x, float y)
// {
// 	return rxvec3_xyz(x,y,0);
// }

// rxvec3_t rxvec3_x(float x)
// {
// 	return rxvec3_xyz(x,0,0);
// }

// rxvec3_t rxvec3_y(float y)
// {
// 	return rxvec3_xyz(0,y,0);
// }

// rxvec3_t rxvec3_z(float z)
// {
// 	return rxvec3_xyz(0,0,z);
// }

// rxvec3_t rxvec3(float xyz)
// {
// 	return rxvec3_xyz(xyz,xyz,xyz);
// }

float rxvector_dot(rxvec3_t a, rxvec3_t b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

rxvec3_t rxvector_cross(rxvec3_t a, rxvec3_t b)
{
	/* sometimes I leave comments like these ones around, I spend so much
	time in front of a computer that the only photographs that I have of me
	are these comments. I don't know why I wrote it, nor how old it is,
	but is here, and so am I, for now - 1/29/24 */

  	// note: this is how I memorize the cross product formula, think of rotations around an axis.

	return rxvec3_xyz
    ( a.y*b.z - a.z*b.y,    // x-axis -> z/y plane
      a.z*b.x - a.x*b.z,    // y-axis -> x/z plane
      a.x*b.y - a.y*b.x );  // z-axis -> y/x plane
 }

 float rxvector_length(rxvec3_t a)
 {
 	return sqrtf(rxvector_dot(a,a));
 }

 float vec2_dot(vec2 a, vec2 b)
 {
 	return a.x*b.x + a.y*b.y;
 }

 float vec2_len(vec2 a)
 {
 	return sqrtf(vec2_dot(a,a));
 }

 vec2 vec2_add(vec2 a, vec2 b)
 {
 	vec2 r;
 	r.x = a.x+b.x;
 	r.y = a.y+b.y;
 	return r;
 }

 vec2 vec2_sub(vec2 a, vec2 b)
 {
 	vec2 r;
 	r.x = a.x-b.x;
 	r.y = a.y-b.y;
 	return r;
 }

 rxvec3_t rxvector_add(rxvec3_t a, rxvec3_t b)
 {
 	rxvec3_t r;
 	r.x = a.x+b.x;
 	r.y = a.y+b.y;
 	r.z = a.z+b.z;
 	return r;
 }

 rxvec3_t rxvector_sub(rxvec3_t a, rxvec3_t b)
 {
 	rxvec3_t r;
 	r.x = a.x-b.x;
 	r.y = a.y-b.y;
 	r.z = a.z-b.z;
 	return r;
 }

 rxvec3_t rxvector_mul(rxvec3_t a, rxvec3_t b)
 {
 	rxvec3_t r;
 	r.x = a.x*b.x;
 	r.y = a.y*b.y;
 	r.z = a.z*b.z;
 	return r;
 }

 rxvec3_t rxvector_downscale(rxvec3_t a, float b)
 {
 	rxvec3_t r;
 	r.x = a.x/b;
 	r.y = a.y/b;
 	r.z = a.z/b;
 	return r;
 }

 rxvec3_t rxvector_scale(rxvec3_t a, float b)
 {
 	rxvec3_t r;
 	r.x = a.x*b;
 	r.y = a.y*b;
 	r.z = a.z*b;
 	return r;
 }

 rxvec3_t rxvector_negate(rxvec3_t a)
 {
 	rxvec3_t r;
 	r.x = - a.x;
 	r.y = - a.y;
 	r.z = - a.z;
 	return r;
 }

 rxvec3_t rxvector_min(rxvec3_t a, float min)
 {
 	rxvec3_t r;
 	r.x = a.x > min ? min : a.x;
 	r.y = a.y > min ? min : a.y;
 	r.z = a.z > min ? min : a.z;
 	return r;
 }

 rxvec3_t rxvector_max(rxvec3_t a, float max)
 {
 	rxvec3_t r;
 	r.x = a.x < max ? max : a.x;
 	r.y = a.y < max ? max : a.y;
 	r.z = a.z < max ? max : a.z;
 	return r;
 }

 rxvec3_t rxvector_abs(rxvec3_t a)
 {
 	rxvec3_t r;
 	r.x = a.x < 0 ? - a.x : a.x;
 	r.y = a.y < 0 ? - a.y : a.y;
 	r.z = a.z < 0 ? - a.z : a.z;
 	return r;
 }


 rxvec3_t rxvector_normalize(rxvec3_t a)
 {
 	float length=rxvector_length(a);

 	if(length != 0)
 	a=rxvector_scale(a,1/length);

 return a;
}

lgi_Matrix lgi_Matrix__projection(float r, float v, float zmin, float zmax)
{
	lgi_Matrix m = lgi_Matrix__identity();

	v = 1. / tan(v / 180 * lgi_PI * .5);

	m.m[0][0] = v * r;
	m.m[1][1] = v;
	m.m[2][2] = zmax / (zmax - zmin);
	m.m[3][2] = - (zmax * zmin) / (zmax - zmin);
	m.m[3][3] = 0.;
	m.m[2][3] = 1.;
	return m;
}

lgi_Matrix lgi_Matrix__identity()
{
	lgi_Matrix r;
	r.m[0][0]=1.f;r.m[1][0]=0.f;r.m[2][0]=0.f;r.m[3][0]=0.f;
	r.m[0][1]=0.f;r.m[1][1]=1.f;r.m[2][1]=0.f;r.m[3][1]=0.f;
	r.m[0][2]=0.f;r.m[1][2]=0.f;r.m[2][2]=1.f;r.m[3][2]=0.f;
	r.m[0][3]=0.f;r.m[1][3]=0.f;r.m[2][3]=0.f;r.m[3][3]=1.f;
	return r;
}



// todo!!: remove loop
lgi_Matrix lgi_Matrix__multiply(lgi_Matrix a, lgi_Matrix b)
{ lgi_Matrix result;
	for(int r=0; r<4; ++r)
	{ for(int c=0; c<4; ++c)
		{  result.m[r][c] =
			(a.m[r][0]*b.m[0][c]) +
			(a.m[r][1]*b.m[1][c]) +
			(a.m[r][2]*b.m[2][c]) +
			(a.m[r][3]*b.m[3][c]);
		}
	}
	return result;
}

lgi_Matrix lgi_Matrix_rotationZ(float angle) {
	lgi_Matrix r = lgi_Matrix__identity();
	float cosres = cosf(angle);
	float sinres = sinf(angle);
	r.m[0][0]= + cosres;
	r.m[0][1]= + sinres;
	r.m[1][0]= - sinres;
	r.m[1][1]= + cosres;
	return r;
}

lgi_Matrix rxmatrix_rotY(float angle) {
	lgi_Matrix r = lgi_Matrix__identity();
	float cosres = cos(angle);
	float sinres = sin(angle);
	r.m[0][0] =   cosres;
	r.m[0][2] = - sinres;
	r.m[2][0] =   sinres;
	r.m[2][2] =   cosres;
	return r;
}

lgi_Matrix rxmatrix_rotX(float angle) {
	lgi_Matrix result = lgi_Matrix__identity();
	float cosres = cosf(angle);
	float sinres = sinf(angle);
	result.m[1][1] =   cosres;
	result.m[1][2] =   sinres;
	result.m[2][1] = - sinres;
	result.m[2][2] =   cosres;
	return result;
}



lgi_Matrix rxmatrix_translate_xyz(float x, float y, float z)
{ lgi_Matrix result=lgi_Matrix__identity();
	result.m[3][0]=x;
	result.m[3][1]=y;
	result.m[3][2]=z;
	return result;
}

lgi_Matrix rxmatrix_flip_vertically()
{ lgi_Matrix result=lgi_Matrix__identity();
	result.m[1][1]=-1.f;
	return result;
}


/* All this has to be worked out */
vec4 rxmul_matvec(lgi_Matrix m, vec4 v)
{
	vec4 r;
	r.x = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0];
	r.y = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1];
	r.z = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2];
	r.w = m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3];
	return r;
}

vec2 rxadd_vec2(vec2 v0, vec2 v1)
{
	vec2 r;
	r.x = v0.x + v1.x;
	r.y = v0.y + v1.y;
	return r;
}

vec2 rxmul_vec2(vec2 v0, vec2 v1)
{
	vec2 r;
	r.x = v0.x * v1.x;
	r.y = v0.y * v1.y;
	return r;
}

vec2 vec2_xy(float x, float y)
{

	vec2 r;
	r.x = x;
	r.y = y;

	return r;
}

// vec2 rxvec2i_vec2(rxvec2i_t v)
// {
// 	vec2 r;
// 	r.x = v.x;
// 	r.y = v.y;
// 	return r;
// }


lgi_API vec4 vec4_mix(float ratio, vec4 min, vec4 max) {
	vec4 r;
	r.r = (float) (min.r + ratio * (max.r - min.r));
	r.g = (float) (min.g + ratio * (max.g - min.g));
	r.b = (float) (min.b + ratio * (max.b - min.b));
	r.a = (float) (min.a + ratio * (max.a - min.a));
	return r;
}