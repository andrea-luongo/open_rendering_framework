#pragma once

#include <optix_world.h>
#include "../structs.h"
#include "../sampler.h"

using namespace optix;

const float M_4PIf = 4.0f*M_PIf;
const float M_1_4PIPIf = M_1_PIf/M_4PIf;

// Better dipole if z_r = 1/sigma_t_p and d_r = sqrt(z_r^2 + r^2)
__device__ float bdp_bssrdf(float d_r, float z_r, const float4& props, const float4& C)
{
  float sigma_s = props.x, sigma_a = props.y, g = props.z, A = C.w;
  float sigma_t = sigma_s + sigma_a;
  float sigma_s_p = sigma_s*(1.0f - g);
  float sigma_t_p = sigma_s_p + sigma_a;
  float alpha_p = sigma_s_p/sigma_t_p;
  float D = 1.0f/(3.0f*sigma_t_p);
  D *= (sigma_a*2.0f + sigma_s_p)/sigma_t_p; // Grosjean's approximation
  float d_e = 4.0f*A*D;
  float sigma_tr = sqrt(sigma_a/D);
  float d_v_sqr = d_r*d_r + 2.0f*z_r*d_e + d_e*d_e; // d_r^2 - z_r^2 + z_v^2
  float d_v = sqrt(d_v_sqr);
  float tr_r = sigma_tr*d_r;
  float d_r_c = max(d_r, 0.25f/sigma_t);
  float S_r = z_r*(1.0f + tr_r)/(d_r_c*d_r_c*d_r_c);
  float T_r = exp(-tr_r);
  S_r *= T_r;
  float tr_v = sigma_tr*d_v;
  float S_v = (z_r + d_e)*(1.0f + tr_v)/(d_v_sqr*d_v);
  float T_v = exp(-tr_v);
  S_v *= T_v;
  float phi = (T_r/d_r - T_v/d_v)/D;
  float S_d = phi*C.y + (S_r + S_v)*C.z;
  return alpha_p*M_1_4PIPIf*C.x*S_d;
}

// Henyey-Greenstein phase function
__device__ float phase_HG(float cos_theta, float g)
{
  float g_sqr = g*g;
  float demon = 1.0f + g_sqr - g*(2.0f*cos_theta);
  return (1.0f - g_sqr)/(powf(demon, 1.5f)*M_4PIf);
}

// Photon Beam Diffusion [Habel et al. 2013]
__device__ float single_diffuse(float t, float d_r, const float3& w_i, const float3& w_o, const float3& n_o, const float4& props) {
  float sigma_s = props.x, sigma_a = props.y, g = props.z;
  float sigma_t = sigma_s + sigma_a;
  float cos_theta_o = abs(dot(w_o, n_o));
  float d_r_c = max(d_r, 0.25f/sigma_t);
  return sigma_s*phase_HG(dot(w_i, w_o), g)*exp(-sigma_t*(t + d_r))*cos_theta_o/(d_r_c*d_r_c);
}
__device__ float3 pbd_bssrdf(const float3& x, const float3& ni, const float3& wi, const float3& no, const float4& props, const float4& C)
{
  const float N = 5.0f;
  float sigma_s = props.x, sigma_a = props.y, g = props.z, eta = props.w;
  float sigma_t = sigma_s + sigma_a;
  float sigma_s_p = sigma_s*(1.0f - g);
  float sigma_t_p = sigma_s_p + sigma_a;
  float alpha_p = sigma_s_p/sigma_t_p;
  float a = 0.9f/sigma_t_p;
  float b = 1.1f/sigma_t_p;
  float w_exp = clamp((length(x) - a)/(b - a), 0.0f, 1.0f);
  float w_equ = 1.0f - w_exp;
  float3 wt = refract(-wi, ni, 1.0f/eta);
  float cos_theta_t = dot(wt, -ni);
  float Delta = dot(x, wt);                           // signed distance to perpendicular
  float h = length(Delta*wt - x);                     // perpendicular distance to beam
  float theta_a = atan2(-Delta, h);
  float theta_b = 0.5f*M_PIf;
  float3 S_d = make_float3(0.0f);
  for(unsigned int k = 0; k < 3; ++k)
  {
    for(float i = 1.0f; i <= N; ++i)
    {
      float xi_i = (i - 0.5)/N;                         // deterministic regular sequence
      float t_i = -log(1.0f - xi_i)/sigma_t_p;          // exponential sampling
      float3 xr_xo = x - t_i*wt;
      float d_r = length(xr_xo);
      float z_r = t_i*cos_theta_t;
      float kappa = 1.0f - exp(-2.0f*sigma_t*(d_r + t_i));
      float pdf_exp = sigma_t_p*exp(-sigma_t_p*t_i);
      float Q = alpha_p*pdf_exp;
      float f = bdp_bssrdf(d_r, z_r, props, C)*Q*kappa;
      float s = single_diffuse(t_i, d_r, wt, normalize(xr_xo), no, props)*M_1_PIf*C.x*kappa;
      float theta_j = lerp(theta_a, theta_b, xi_i);
      float t_j = h*tan(theta_j) + Delta;               // equiangular sampling
      float t_equ = t_i - Delta;                        // multiple importance sampling
      float pdf_equ = h/((theta_b - theta_a)*(h*h + t_equ*t_equ));
      *(&S_d.x + k) += (f + s)*w_exp/(w_exp*pdf_exp + w_equ*pdf_equ);  // exponential sampling part (t_i)
      t_equ = t_j - Delta;
      pdf_equ = h/((theta_b - theta_a)*(h*h + t_equ*t_equ));
      pdf_exp = sigma_t_p*exp(-sigma_t_p*t_j);
      xr_xo = x - t_j*wt;
      d_r = length(xr_xo);
      z_r = t_j*cos_theta_t;
      kappa = 1.0f - exp(-2.0f*sigma_t*(d_r + t_j));
      Q = alpha_p*pdf_exp;
      f = bdp_bssrdf(d_r, z_r, props, C)*Q*kappa;
      s = single_diffuse(t_j, d_r, wt, normalize(xr_xo), no, props)*M_1_PIf*C.x*kappa;
      *(&S_d.x + k) += (f + s)*w_equ/(w_exp*pdf_exp + w_equ*pdf_equ);  // equiangular sampling part (t_j)
    }
  }
  return fmaxf(S_d, make_float3(0.0f))/N;
}

__device__ float3 bssrdf(const float3& _xi, const float3& _ni, const float3& _w12,
  const float3& _xo, const float3& _no,
  const ScatteringMaterialProperties& properties, uint& t)
{
  float3 _x = _xo - _xi;
  float3 _r_sqr = make_float3(dot(_x, _x));

  // distance to the real source
  float3 _dr_sqr = _r_sqr;
  float dot_x_w12 = dot(_x, w12);
  float3 cos_beta = -sqrt(make_float3(_r_sqr.x - dot_x_w12 * dot_x_w12)/(_r_sqr + properties.de_sqr));
  float mu0 = -dot(_no, w12);
  float edge = mu0 > 0.0f ? 1.0f : 0.0f;
  float3 _D_prime = mu0 * properties.D * edge + properties.one_over_three_ext * (1.0f - edge);
  _dr_sqr += _D_prime * (_D_prime - properties.two_de * cos_beta * edge);

  // direction of the virtual source
  float3 _t = normalize(cross(_ni, _x));
  float3 _nistar = _r_sqr.x < 1.0e-12f ? _ni : cross(normalize(_x), _t);
  float3 _wv = w12 - (2.0f * dot(w12, _nistar)) * _nistar;

  // distance to the virtual source
  float3 _xoxv_r = _x - properties.two_a_de.x * _nistar;
  float3 _xoxv_g = _x - properties.two_a_de.y * _nistar;
  float3 _xoxv_b = _x - properties.two_a_de.z * _nistar;
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