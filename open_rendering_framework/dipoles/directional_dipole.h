#pragma once

#include <optix_world.h>
#include "../structs.h"

using namespace optix;
  
__device__ float3 S_infinite(const float3& _r_sqr, const float x_dot_w12, const float no_dot_w12, const float x_dot_no,
                             const ScatteringMaterialProperties& properties)
{
  float3 _r = sqrt(_r_sqr);
  float3 _r_tr = properties.transport*_r;
  float3 _r_tr_p1 = _r_tr + 1.0f;
  float3 _T = exp(-_r_tr);
  float3 coeff = _T/(_r*_r_sqr);
  float3 first = properties.C_phi*(_r_sqr*properties.reducedExtinction + _r_tr_p1*x_dot_w12)*3.0f;
  float3 second = properties.C_E*(properties.three_D*_r_tr_p1*no_dot_w12 - (_r_tr_p1 + properties.three_D*(3.0f*_r_tr_p1 + _r_tr*_r_tr)/_r_sqr * x_dot_w12)*x_dot_no);
  float3 _S = coeff*(first - second);
  return _S;
}

__device__ float3 S_infinite_vec(const float3& _r_sqr, const float3& x_dot_w12, const float no_dot_w12, const float3& x_dot_no,
                                 const ScatteringMaterialProperties& properties)
{
  float3 _r = sqrt(_r_sqr);
  float3 _r_tr = properties.transport*_r;
  float3 _r_tr_p1 = _r_tr + 1.0f;
  float3 _T = exp(-_r_tr);
  float3 coeff = _T/(_r*_r_sqr);
  float3 first = properties.C_phi*(_r_sqr*properties.reducedExtinction + _r_tr_p1 * x_dot_w12)*3.0f;
  float3 second = properties.C_E*(properties.three_D*_r_tr_p1*no_dot_w12 - (_r_tr_p1 + properties.three_D*(3.0f*_r_tr_p1 + _r_tr*_r_tr)/_r_sqr * x_dot_w12)*x_dot_no);
  float3 _S = coeff*(first - second);
  return _S;
}


__device__ float3 dirpole_bssrdf(const float3& _xi, const float3& _ni, const float3& _w12,
                                 const float3& _xo, const float3& _no,
                                 const ScatteringMaterialProperties& properties)
{
  float3 _x = _xo - _xi;
  float _r_sqr = dot(_x, _x);

  // distance to the real source
  float3 _dr_sqr = make_float3(_r_sqr);
  float dot_x_w12 = dot(_x, _w12);
  float3 cos_beta = -sqrt(make_float3(_r_sqr - dot_x_w12*dot_x_w12)/(_r_sqr + properties.de*properties.de));
  float mu0 = -dot(_no, _w12);
  float edge = mu0 > 0.0f ? 1.0f : 0.0f;
  float3 _D_prime = properties.D*mu0*edge + properties.one_over_three_ext*(1.0f - edge);
  _dr_sqr += _D_prime*(_D_prime - 2.0f*properties.de*cos_beta*edge);

  // direction of the virtual source
  float3 _t = normalize(cross(_ni, _x));
  float3 _nistar = _r_sqr < 1.0e-12f ? _ni : cross(normalize(_x), _t);
  float3 _wv = _w12 - (2.0f*dot(_w12, _nistar))*_nistar;

  // distance to the virtual source
  float3 _xoxv_r = _x - properties.two_A_de.x*_nistar;
  float3 _xoxv_g = _x - properties.two_A_de.y*_nistar;
  float3 _xoxv_b = _x - properties.two_A_de.z*_nistar;
  float3 _dv_sqr = make_float3(dot(_xoxv_r, _xoxv_r), dot(_xoxv_g, _xoxv_g), dot(_xoxv_b, _xoxv_b));

  // cosines of the virtual source
  float3 _x_dot_wv = make_float3(dot(_xoxv_r, _wv), dot(_xoxv_g, _wv), dot(_xoxv_b, _wv));
  float3 _x_dot_no = make_float3(dot(_xoxv_r, _no), dot(_xoxv_g, _no), dot(_xoxv_b, _no));

  // compute source contributions and return BSSRDF result
  float3 _Sr = S_infinite(_dr_sqr, dot_x_w12, -mu0, dot(_x, _no), properties);
  float3 _Sv = S_infinite_vec(_dv_sqr, _x_dot_wv, dot(_no, _wv), _x_dot_no, properties);
  float3 _Sd = _Sr - _Sv;
  return max(_Sd, make_float3(0.0f));
}