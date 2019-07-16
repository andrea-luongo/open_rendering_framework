#ifndef MyComplex_H
#define MyComplex_H
#include <optixu/optixu_math_namespace.h>
#include "helpers.h"


typedef struct {
	 double real;
	 double im;
}MyComplex;

typedef struct {
	MyComplex x;
	MyComplex y;
	MyComplex z;
}MyComplex3;

__host__ __device__ __inline__ float abs(MyComplex c)
{
	float r = c.real;
	float i = c.im;
	return sqrt(r*r + i*i);
}

__host__ __device__ __inline__ double sqr_abs(MyComplex c)
{
	double r = c.real;
	double i = c.im;
	return (r*r + i*i);
}

__host__ __device__ __inline__ MyComplex conj(MyComplex c)
{
	double r = c.real;
	double i = -c.im;
	MyComplex result = { r, i };
	return result;
}

__host__ __device__ __inline__ MyComplex operator+(MyComplex c1, MyComplex c2)
{
	MyComplex result = { c1.real + c2.real, c1.im + c2.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator+(MyComplex c1, float c2)
{
	MyComplex result = { c1.real + c2, c1.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator+(MyComplex c1, double c2)
{
	MyComplex result = { c1.real + c2, c1.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator+(float c1, MyComplex c2)
{
	MyComplex result = { c1 + c2.real, c2.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator+(double c1, MyComplex c2)
{
	MyComplex result = { c1 + c2.real, c2.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator-(MyComplex c1, MyComplex c2)
{
	MyComplex result = { c1.real - c2.real, c1.im - c2.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator-(MyComplex c1, float c2)
{
	MyComplex result = { c1.real - c2, c1.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator-(MyComplex c1, double c2)
{
	MyComplex result = { c1.real - c2, c1.im };
	return result;
}
__host__ __device__ __inline__ MyComplex operator-(float c1, MyComplex c2)
{
	MyComplex result = { c1 - c2.real, -c2.im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator-(double c1, MyComplex c2)
{
	MyComplex result = { c1 - c2.real, -c2.im };
	return result;
}
__host__ __device__ __inline__ MyComplex operator*(MyComplex c1, MyComplex c2)
{
	double real = c1.real*c2.real - c1.im*c2.im;
	double im = c1.real*c2.im + c1.im*c2.real;
	MyComplex result = { real, im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator*(MyComplex c1, float c2)
{
	double real = c1.real*c2;
	double im = c1.im*c2;
	MyComplex result = { real, im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator*(float c1, MyComplex c2)
{
	double real = c1*c2.real;
	double im = c1*c2.im;
	MyComplex result = { real, im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator*(MyComplex c1, double c2)
{
	double real = c1.real*c2;
	double im = c1.im*c2;
	MyComplex result = { real, im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator*(double c1, MyComplex c2)
{
	double real = c1*c2.real;
	double im = c1*c2.im;
	MyComplex result = { real, im };
	return result;
}

__host__ __device__ __inline__ MyComplex operator/(MyComplex c1, float c2)
{
	return c1*(1.0 / c2);
}

__host__ __device__ __inline__ MyComplex operator/(MyComplex c1, double c2)
{
	return c1*(1.0 / c2);
}

__host__ __device__ __inline__ MyComplex operator/(MyComplex c1, MyComplex c2)
{
	MyComplex num = c1*conj(c2);
	double den = powf(abs(c2), 2);
	return num / den;
}

__host__ __device__ __inline__ MyComplex operator/(float c1, MyComplex c2)
{
	MyComplex num = c1*conj(c2);
	double den = powf(abs(c2), 2);
	return num / den;
}

__host__ __device__ __inline__ MyComplex operator/(double c1, MyComplex c2)
{
	MyComplex num = c1*conj(c2);
	double den = powf(abs(c2), 2);
	return num / den;
}

__host__ __device__ __inline__ float phase(MyComplex c)
{
	double r = c.real;
	double i = c.im;
	double theta;
	if (r == 0 && i > 0)
		theta = M_PIf / 2;
	else if (r == 0 && i < 0)
		theta = -M_PIf / 2;
	else if (r == 0 && i == 0)
		theta = 0;
	else theta = atan(i / r);
	if (r < 0)
		theta += M_PIf;
	return theta;

}

__host__ __device__ __inline__ MyComplex pow(MyComplex c, int exp)
{
	double ab = pow(abs(c), exp);
	double theta = phase(c)*exp;
	MyComplex result = { ab*cos(theta), ab*sin(theta) };
	return result;
}

__host__ __device__ __inline__ MyComplex sqrt(MyComplex c)
{
	double ab = sqrt(abs(c));
	double theta = phase(c) / 2;
	MyComplex r1 = { ab*cos(theta), ab*sin(theta) };
	MyComplex r2 = { -r1.real, -r1.im };
	return r1;
}

__host__ __device__ __inline__ MyComplex exp(MyComplex c)
{
	double a = exp(c.real);
	MyComplex result = {a*cos(c.im),a* sin(c.im) };
	return result;
}


__host__ __device__ __inline__ optix::float3 abs(MyComplex3 c)
{

	optix::float3 result;
	result.x = abs(c.x);
	result.y = abs(c.y);
	result.z = abs(c.z);
	return result;
}

__host__ __device__ __inline__ MyComplex3 conj(MyComplex3 c)
{

	MyComplex3 result = { conj(c.x), conj(c.y), conj(c.z) };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator+(MyComplex3 c1, MyComplex3 c2)
{
	MyComplex3 result = { c1.x + c2.x, c1.y + c2.y, c1.z + c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator+(MyComplex3 c1, float c2)
{
	MyComplex3 result = { c1.x + c2, c1.y + c2, c1.z + c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator+(float c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 + c2.x, c1 + c2.y, c1 + c2.z };
	return result;

}

__host__ __device__ __inline__ MyComplex3 operator+(MyComplex3 c1, double c2)
{
	MyComplex3 result = { c1.x + c2, c1.y + c2, c1.z + c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator+(double c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 + c2.x, c1 + c2.y, c1 + c2.z };
	return result;

}

__host__ __device__ __inline__ MyComplex3 operator-(MyComplex3 c1, MyComplex3 c2)
{
	MyComplex3 result = { c1.x - c2.x, c1.y - c2.y, c1.z - c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator-(MyComplex3 c1, float c2)
{
	MyComplex3 result = { c1.x - c2, c1.y - c2, c1.z - c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator-(float c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 - c2.x, c1 - c2.y, c1 - c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator-(MyComplex3 c1, double c2)
{
	MyComplex3 result = { c1.x - c2, c1.y - c2, c1.z - c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator-(double c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 - c2.x, c1 - c2.y, c1 - c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator*(MyComplex3 c1, MyComplex3 c2)
{

	MyComplex3 result = { c1.x * c2.x, c1.y * c2.y, c1.z * c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator*(MyComplex3 c1, float c2)
{
	MyComplex3 result = { c1.x * c2, c1.y * c2, c1.z * c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator*(float c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 * c2.x, c1 * c2.y, c1 * c2.z };
	return result;
}


__host__ __device__ __inline__ MyComplex3 operator*(MyComplex3 c1, double c2)
{
	MyComplex3 result = { c1.x * c2, c1.y * c2, c1.z * c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator*(double c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 * c2.x, c1 * c2.y, c1 * c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator/(MyComplex3 c1, float c2)
{
	return c1*(1 / c2);
}

__host__ __device__ __inline__ MyComplex3 operator/(MyComplex3 c1, double c2)
{
	return c1*(1 / c2);
}


__host__ __device__ __inline__ MyComplex3 operator/(MyComplex3 c1, MyComplex3 c2)
{
	MyComplex3 result = { c1.x / c2.x, c1.y / c2.y, c1.z / c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator/(MyComplex3 c1, MyComplex c2)
{
	MyComplex3 result = { c1.x / c2, c1.y / c2, c1.z / c2 };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator/(MyComplex c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 / c2.x, c1 / c2.y, c1 / c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator/(float c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 / c2.x, c1 / c2.y, c1 / c2.z };
	return result;
}

__host__ __device__ __inline__ MyComplex3 operator/(double c1, MyComplex3 c2)
{
	MyComplex3 result = { c1 / c2.x, c1 / c2.y, c1 / c2.z };
	return result;
}

__host__ __device__ __inline__ optix::float3 phase(MyComplex3 c)
{
	optix::float3 result;
	result.x = phase(c.x);
	result.y = phase(c.y);
	result.z = phase(c.z);
	return result;
}

__host__ __device__ __inline__ MyComplex3 pow(MyComplex3 c, int exp)
{
	MyComplex3 result = { pow(c.x, exp), pow(c.y, exp), pow(c.z, exp) };
	return result;
}

__host__ __device__ __inline__ MyComplex3 sqrt(MyComplex3 c)
{
	MyComplex3 result = { sqrt(c.x), sqrt(c.y), sqrt(c.z) };
	return result;
}

#endif // MyComplex_H