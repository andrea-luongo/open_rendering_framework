
/*
 * Copyright (c) 2008 - 2010 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#pragma once

#include <optixu/optixu_math_namespace.h>

// Convert a float3 in [0,1)^3 to a uchar4 in [0,255]^4 -- 4th channel is set to 255
#ifdef __CUDACC__
static __device__ __inline__ optix::uchar4 make_color(const optix::float3& c)
{
    return optix::make_uchar4( static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* B */
                               static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                               static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* R */
                               255u);                                                 /* A */
}
#endif

// Sample Phong lobe relative to U, V, W frame
static
__host__ __device__ __inline__ optix::float3 sample_phong_lobe( optix::float2 sample, float exponent, 
                                                                optix::float3 U, optix::float3 V, optix::float3 W )
{
  const float power = expf( logf(sample.y)/(exponent+1.0f) );
  const float phi = sample.x * 2.0f * (float)M_PIf;
  const float scale = sqrtf(1.0f - power*power);
  
  const float x = cosf(phi)*scale;
  const float y = sinf(phi)*scale;
  const float z = power;

  return x*U + y*V + z*W;
}

// Sample Phong lobe relative to U, V, W frame
static
__host__ __device__ __inline__ optix::float3 sample_phong_lobe( const optix::float2 &sample, float exponent, 
                                                                const optix::float3 &U, const optix::float3 &V, const optix::float3 &W, 
                                                                float &pdf, float &bdf_val )
{
  const float cos_theta = powf(sample.y, 1.0f/(exponent+1.0f) );

  const float phi = sample.x * 2.0f * M_PIf;
  const float sin_theta = sqrtf(1.0f - cos_theta*cos_theta);
  
  const float x = cosf(phi)*sin_theta;
  const float y = sinf(phi)*sin_theta;
  const float z = cos_theta;

  const float powered_cos = powf( cos_theta, exponent );
  pdf = (exponent+1.0f) / (2.0f*M_PIf) * powered_cos;
  bdf_val = (exponent+2.0f) / (2.0f*M_PIf) * powered_cos;  

  return x*U + y*V + z*W;
}

// Get Phong lobe PDF for local frame
static
__host__ __device__ __inline__ float get_phong_lobe_pdf( float exponent, const optix::float3 &normal, const optix::float3 &dir_out, 
                                                         const optix::float3 &dir_in, float &bdf_val)
{  
  using namespace optix;

  float3 r = -reflect(dir_out, normal);
  const float cos_theta = fabs(dot(r, dir_in));
  const float powered_cos = powf(cos_theta, exponent );

  bdf_val = (exponent+2.0f) / (2.0f*M_PIf) * powered_cos;  
  return (exponent+1.0f) / (2.0f*M_PIf) * powered_cos;
}

// Create ONB from normal.  Resulting W is parallel to normal
static
__host__ __device__ __inline__ void create_onb( const optix::float3& n, optix::float3& U, optix::float3& V, optix::float3& W )
{
  using namespace optix;

  W = normalize( n );
  U = cross( W, optix::make_float3( 0.0f, 1.0f, 0.0f ) );

  if ( fabs( U.x ) < 0.001f && fabs( U.y ) < 0.001f && fabs( U.z ) < 0.001f  )
    U = cross( W, make_float3( 1.0f, 0.0f, 0.0f ) );

  U = normalize( U );
  V = cross( W, U );
}

// Create ONB from normalized vector
static
__device__ __inline__ void create_onb( const optix::float3& n, optix::float3& U, optix::float3& V)
{
  using namespace optix;

  U = cross( n, make_float3( 0.0f, 1.0f, 0.0f ) );

  if ( dot( U, U ) < 1e-3f )
    U = cross( n, make_float3( 1.0f, 0.0f, 0.0f ) );

  U = normalize( U );
  V = cross( n, U );
}

// Compute the origin ray differential for transfer
static
__host__ __device__ __inline__ optix::float3 differential_transfer_origin(optix::float3 dPdx, optix::float3 dDdx, float t, optix::float3 direction, optix::float3 normal)
{
  float dtdx = -optix::dot((dPdx + t*dDdx), normal)/optix::dot(direction, normal);
  return (dPdx + t*dDdx)+dtdx*direction;
}

// Compute the direction ray differential for a pinhole camera
static
__host__ __device__ __inline__ optix::float3 differential_generation_direction(optix::float3 d, optix::float3 basis)
{
  float dd = optix::dot(d,d);
  return (dd*basis-optix::dot(d,basis)*d)/(dd*sqrtf(dd));
}

// Compute the direction ray differential for reflection
static
__host__ __device__ __inline__
optix::float3 differential_reflect_direction(optix::float3 dPdx, optix::float3 dDdx, optix::float3 dNdP, 
                                             optix::float3 D, optix::float3 N)
{
  using namespace optix;

  float3 dNdx = dNdP*dPdx;
  float dDNdx = dot(dDdx,N) + dot(D,dNdx);
  return dDdx - 2*(dot(D,N)*dNdx + dDNdx*N);
}

// Compute the direction ray differential for refraction
static __host__ __device__ __inline__ 
optix::float3 differential_refract_direction(optix::float3 dPdx, optix::float3 dDdx, optix::float3 dNdP, 
                                             optix::float3 D, optix::float3 N, float ior, optix::float3 T)
{
  using namespace optix;

  float eta;
  if(dot(D,N) > 0.f) {
    eta = ior;
    N = -N;
  } else {
    eta = 1.f / ior;
  }

  float3 dNdx = dNdP*dPdx;
  float mu = eta*dot(D,N)-dot(T,N);
  float TN = -sqrtf(1-eta*eta*(1-dot(D,N)*dot(D,N)));
  float dDNdx = dot(dDdx,N) + dot(D,dNdx);
  float dmudx = (eta - (eta*eta*dot(D,N))/TN)*dDNdx;
  return eta*dDdx - (mu*dNdx+dmudx*N);
}

// Color space conversions
static __host__ __device__ __inline__ optix::float3 Yxy2XYZ( const optix::float3& Yxy )
{
  return optix::make_float3(  Yxy.y * ( Yxy.x / Yxy.z ),
                              Yxy.x,
                              ( 1.0f - Yxy.y - Yxy.z ) * ( Yxy.x / Yxy.z ) );
}

static __host__ __device__ __inline__ optix::float3 XYZ2rgb( const optix::float3& xyz)
{
  const float R = optix::dot( xyz, optix::make_float3(  3.2410f, -1.5374f, -0.4986f ) );
  const float G = optix::dot( xyz, optix::make_float3( -0.9692f,  1.8760f,  0.0416f ) );
  const float B = optix::dot( xyz, optix::make_float3(  0.0556f, -0.2040f,  1.0570f ) );
  return optix::make_float3( R, G, B );
}

static __host__ __device__ __inline__ optix::float3 Yxy2rgb( optix::float3 Yxy )
{
  using namespace optix;

  // First convert to xyz
  float3 xyz = make_float3( Yxy.y * ( Yxy.x / Yxy.z ),
                            Yxy.x,
                            ( 1.0f - Yxy.y - Yxy.z ) * ( Yxy.x / Yxy.z ) );

  const float R = dot( xyz, make_float3(  3.2410f, -1.5374f, -0.4986f ) );
  const float G = dot( xyz, make_float3( -0.9692f,  1.8760f,  0.0416f ) );
  const float B = dot( xyz, make_float3(  0.0556f, -0.2040f,  1.0570f ) );
  return make_float3( R, G, B );
}

static __host__ __device__ __inline__ optix::float3 rgb2Yxy( optix::float3 rgb)
{
  using namespace optix;

  // convert to xyz
  const float X = dot( rgb, make_float3( 0.4124f, 0.3576f, 0.1805f ) );
  const float Y = dot( rgb, make_float3( 0.2126f, 0.7152f, 0.0722f ) );
  const float Z = dot( rgb, make_float3( 0.0193f, 0.1192f, 0.9505f ) );

  // convert xyz to Yxy
  return make_float3( Y, 
                      X / ( X + Y + Z ),
                      Y / ( X + Y + Z ) );
}

static __host__ __device__ __inline__ optix::float3 tonemap( const optix::float3 &hdr_value, float Y_log_av, float Y_max)
{
  using namespace optix;

  float3 val_Yxy = rgb2Yxy( hdr_value );
  
  float Y        = val_Yxy.x; // Y channel is luminance
  const float a = 0.04f;
  float Y_rel = a * Y / Y_log_av;
  float mapped_Y = Y_rel * (1.0f + Y_rel / (Y_max * Y_max)) / (1.0f + Y_rel);

  float3 mapped_Yxy = make_float3( mapped_Y, val_Yxy.y, val_Yxy.z ); 
  float3 mapped_rgb = Yxy2rgb( mapped_Yxy ); 

  return mapped_rgb;
}


static __device__ __inline__ optix::float3 max(const optix::float3 &value1, const optix::float3 &value2)
{
	return optix::make_float3(fmaxf(value1.x, value2.x), fmaxf(value1.y, value2.y), fmaxf(value1.z, value2.z));
}

static __device__ __inline__ optix::float3 min(const optix::float3 &value1, const optix::float3 &value2)
{
	return optix::make_float3(fminf(value1.x, value2.x), fminf(value1.y, value2.y), fminf(value1.z, value2.z));
}


static __device__ __inline__ optix::float3 exp(const optix::float3 &value1)
{
	return optix::make_float3(exp(value1.x), exp(value1.y), exp(value1.z));
}

static __device__ __inline__ optix::float3 sqrt(const optix::float3 &value1)
{
	return optix::make_float3(sqrt(value1.x), sqrt(value1.y), sqrt(value1.z));
}

static  __device__ __inline__ optix::float3 abs(const optix::float3 &value1)
{
	return optix::make_float3(abs(value1.x), abs(value1.y), abs(value1.z));
}

static __device__ __inline__ float step(const float &edge, const float &x)
{
	  return (x < edge)? 0.0f : 1.0f;
}

static  __device__ __inline__ optix::float3 fpowf(const optix::float3 & p, const float ex)
{
	return optix::make_float3(powf(p.x, ex), powf(p.y, ex), powf(p.z, ex));
}



// zeta1, zeta2 are two random uniform iid in [0,1], the function gives a uniform distributed point inside a triangle defined by v0,v1,v2
static __host__ __device__ __inline__ optix::float3 sample_point_triangle(float zeta1, float zeta2, optix::float3 v0, optix::float3 v1, optix::float3 v2)
{
	// As in Osada, Robert: Shape Distributions
	 zeta1 = sqrt(zeta1);
	 return (1-zeta1) * v0 + zeta1 * (1-zeta2) * v1 + zeta1 * zeta2 * v2;
}

//Transform direction wi to local coordinates around the normal
static __device__ __inline__ optix::float3 transformToLocal(const optix::float3 wi, const optix::float3 normal){
	optix::float3 u, v;
	create_onb(normal, u, v);
	float x = optix::dot(wi, u);
	float y = optix::dot(wi, v);
	float z = optix::dot(wi, normal);
	return  optix::normalize(optix::make_float3(x, y, z));
}

//Transform a local direction wi to world coordinates around the normal
static __device__ __inline__ optix::float3 transformToWorld(const optix::float3 wi, const optix::float3 normal){
	optix::float3 u, v;
	create_onb(normal, u, v);
	optix::float3 new_w = optix::normalize(u * wi.x + v * wi.y + normal * wi.z);
	return new_w;
}

static __host__ __inline__ __device__ double erfinv(double x)
{
	double w, p;

	w = -log((1.0 - x)*(1.0 + x));

	if (w < 6.250000) {
		w = w - 3.125000;
		p = -3.6444120640178196996e-21;
		p = -1.685059138182016589e-19 + p*w;
		p = 1.2858480715256400167e-18 + p*w;
		p = 1.115787767802518096e-17 + p*w;
		p = -1.333171662854620906e-16 + p*w;
		p = 2.0972767875968561637e-17 + p*w;
		p = 6.6376381343583238325e-15 + p*w;
		p = -4.0545662729752068639e-14 + p*w;
		p = -8.1519341976054721522e-14 + p*w;
		p = 2.6335093153082322977e-12 + p*w;
		p = -1.2975133253453532498e-11 + p*w;
		p = -5.4154120542946279317e-11 + p*w;
		p = 1.051212273321532285e-09 + p*w;
		p = -4.1126339803469836976e-09 + p*w;
		p = -2.9070369957882005086e-08 + p*w;
		p = 4.2347877827932403518e-07 + p*w;
		p = -1.3654692000834678645e-06 + p*w;
		p = -1.3882523362786468719e-05 + p*w;
		p = 0.0001867342080340571352 + p*w;
		p = -0.00074070253416626697512 + p*w;
		p = -0.0060336708714301490533 + p*w;
		p = 0.24015818242558961693 + p*w;
		p = 1.6536545626831027356 + p*w;
	}
	else if (w < 16.000000) {
		w = sqrt(w) - 3.250000;
		p = 2.2137376921775787049e-09;
		p = 9.0756561938885390979e-08 + p*w;
		p = -2.7517406297064545428e-07 + p*w;
		p = 1.8239629214389227755e-08 + p*w;
		p = 1.5027403968909827627e-06 + p*w;
		p = -4.013867526981545969e-06 + p*w;
		p = 2.9234449089955446044e-06 + p*w;
		p = 1.2475304481671778723e-05 + p*w;
		p = -4.7318229009055733981e-05 + p*w;
		p = 6.8284851459573175448e-05 + p*w;
		p = 2.4031110387097893999e-05 + p*w;
		p = -0.0003550375203628474796 + p*w;
		p = 0.00095328937973738049703 + p*w;
		p = -0.0016882755560235047313 + p*w;
		p = 0.0024914420961078508066 + p*w;
		p = -0.0037512085075692412107 + p*w;
		p = 0.005370914553590063617 + p*w;
		p = 1.0052589676941592334 + p*w;
		p = 3.0838856104922207635 + p*w;
	}
	else {
		w = sqrt(w) - 5.000000;
		p = -2.7109920616438573243e-11;
		p = -2.5556418169965252055e-10 + p*w;
		p = 1.5076572693500548083e-09 + p*w;
		p = -3.7894654401267369937e-09 + p*w;
		p = 7.6157012080783393804e-09 + p*w;
		p = -1.4960026627149240478e-08 + p*w;
		p = 2.9147953450901080826e-08 + p*w;
		p = -6.7711997758452339498e-08 + p*w;
		p = 2.2900482228026654717e-07 + p*w;
		p = -9.9298272942317002539e-07 + p*w;
		p = 4.5260625972231537039e-06 + p*w;
		p = -1.9681778105531670567e-05 + p*w;
		p = 7.5995277030017761139e-05 + p*w;
		p = -0.00021503011930044477347 + p*w;
		p = -0.00013871931833623122026 + p*w;
		p = 1.0103004648645343977 + p*w;
		p = 4.8499064014085844221 + p*w;
	}

	return p*x;
}

//Gaussian height inverse CDF
static __device__ __inline__ float GaussianHeightInvCDF(const float z){
	float h = M_SQRT2f * (float)erfinv(2.0f * z - 1.0f);
	return h;
}

//Gaussian height CDF
static __device__ __inline__ float GaussianHeightCDF(const float h){
	float cdf = 0.5f + 0.5f * (float)erf(M_SQRT1_2f * h);
	return cdf;
}

//Gaussian Height PDF
static __device__ __inline__ float  GaussianHeightPDF(const float h){

	float sqrt2pi_inv = 1.f / sqrtf(2 * M_PIf);
	float pdf = sqrt2pi_inv *expf(-0.5f * h * h);
	return pdf;
}

