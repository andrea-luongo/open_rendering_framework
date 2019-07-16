

#include "Microfacet.h"


__device__ __inline__ float3 ridge_plane_projection(const float3& w, const float3& u)
{
	float3 w_p = normalize(w - dot(w, u) * u);
	return w_p;
}

__device__ __inline__ void ridge_create_onb(const float3& n, float3& u, float3& v, const float& t_x, const float& t_y)
{
	//create_onb(n, u, v);
	//return;
	const int span = 10;
	const int vshift = int(t_y* span + 1) % 2;
	bool toggle = int(t_x * span + vshift) % 2 == 0;
	if (toggle){
		create_onb(n, u, v);
	}
	else{
		create_onb(n, v, u);
	}
}


__device__ __inline__ void ridge_create_rotate_onb(const float3& n, float3& u, float3& v, const float& rad_angle)
{
	float3 u_loc = make_float3(cosf(rad_angle), sinf(rad_angle), 0.0f);
	float3 v_loc = make_float3(-sinf(rad_angle), cosf(rad_angle), 0.0f);
	
	u = transformToWorld(u_loc, n);
	v = transformToWorld(v_loc, n);
	
}
__device__ __inline__ uint ridge_ridge_side(const float3& w_p, const float3& v, const float& theta_p, const float& slope, float& weight, const float& z)
{
	//ridge_side=1 if w_p hit the slope, ridge_side = 0 if w_p hit the edge
	uint ridge_side = dot(w_p, v) > 0;
	if (ridge_side == 0){
		float prob = fminf(1.0f, tan(theta_p) * tan(slope));
		if (z < prob){
			//hit edge
			ridge_side = 0;
		}
		else{
			//hit slope
			ridge_side = 1;
		}
	}
	return ridge_side;
}

__device__ __inline__ void ridge_get_weight_factors(const float3& w_i, const float3& n, const float3& m, const float3& w_p, const float& ior1_over_ior2, const uint& ridge_side, float& F_r, float& den)
{
	F_r = fmaxf(0.0f, fresnel_R(fmaxf(0.0f, (dot(w_i, m))), ior1_over_ior2));
	if (ridge_side){
		den = fabsf(dot(w_i, n) * dot(m, n) / dot(w_i, m));
	}
	else{
		den = fabsf(dot(w_i, n));
		den = 1.0f;
	}
}

__device__ __inline__ float ridged_G(const float3& w_p, const float3& n, const float3& m, const float& slope, const uint& ridge_side){

	float G;
	uint chi;
	if (ridge_side == 1){
		//hit the slope side
		chi = dot(w_p, n) > 0.0f &&  dot(w_p, m) > 0.0f &&  dot(n, m) > 0.0f;
		float theta = acos(dot(w_p, m));
		//float shadowed_slope = sin(theta + slope) / cos(theta) * sin(slope);
		float shadowed_slope = tan(theta)*tan(slope);
		G = clamp(1.0f - fminf(1.0f, shadowed_slope), 0.0f, 1.0f);
	}
	else{
		//hit the vertical edge
		chi = dot(w_p, n) > 0.0f &&  dot(w_p, m) > 0.0f;
		float theta = acos(dot(w_p, n));
		G = clamp(1.0f / (tan(theta) * tan(slope)), 0.0f, 1.0f);
		G = 0.0f;
	}
	return G * chi;
}

//Beckmann distribution normal sampling in object coordinates
__device__ __inline__ void ridge_sample_normal(const float3& n, const float3& u, const float3& v,
	float3 &sampled_n, const float slope, const uint& ridge_side, const float roughness, const float z)
{
	if (ridge_side == 0){
		//if hit the vertical edge
		sampled_n = -v;
		return;
	}
	//if hit the slope
	float phi = 0;
	float width = roughness;
	float theta;
	float tan_theta;
	tan_theta = -width*width * log(1 - z);
	theta = atan(sqrtf(tan_theta)) + slope;
	//theta = slope;
	clamp(theta, -M_PI_2f, M_PI_2f);
	float costheta = cosf(theta);
	float sintheta = sinf(theta);
	sampled_n = make_float3(sintheta * sin(phi), sintheta * cos(phi), costheta);
	//transform the sampled_normal coordinates from world to object
	sampled_n = normalize(u * sampled_n.x + v * sampled_n.y + n * sampled_n.z);
}

//Beckmann anisotropic distribution normal sampling in object coordinates
__device__ __inline__ void ridge_sample_anisotropic_beckmann_normal(const float3& ideal_m, const float3& u, 
	const float3& v, float3& m, const float a_u, const float a_v,
	const float z1, const float z2, const uint ridge_side)
{
	if (ridge_side == 0){
		//if hit the vertical edge
		m = -v;
		return;
	}
	//if hit the slope
	float3 u_m, v_m;
	u_m = normalize(cross(ideal_m, v));
	v_m = normalize(cross(ideal_m, u_m));

	float3 local_n = beckmann_sample_NDF(a_u, a_v, z1, z2);
	m = normalize(u_m * local_n.x + v_m * local_n.y + ideal_m * local_n.z);

}

//Beckmann anisotropic distribution normal evaluation in object coordinates
__device__ __inline__ float ridge_eval_anisotropic_beckmann_D( const float3& ideal_m, const float3& u,
	const float3& v, float3& m, const float a_u, const float a_v, const uint ridge_side)
{
	float D = 1.0f;
	if (ridge_side == 0){
		//if hit the vertical edge
		return D;
	}
	//if hit the slope
	float3 u_m, v_m;
	u_m = normalize(cross(ideal_m, v));
	v_m = normalize(cross(ideal_m, u_m));

	float3 local_n = make_float3(dot(m, u_m), dot(m, v_m), dot(m, ideal_m));
	D = beckmann_eval_NDF(local_n, a_u, a_v);
	return D;
}


//Gaussian random number
__device__ __inline__ float ridge_sample_gaussian(const float& std_dev, const float& mean, const float& z1, const float& z2)
{
	return  sqrtf(-2.0f * log(z1)) * cosf(2.0f * M_PIf *z2)*std_dev + mean;
}
//Gaussian  distribution evaluation
__device__ __inline__ float ridge_evaluate_gaussian(const float x, const float& std_dev, const float& mean)
{
	float std_dev_sqr = std_dev*std_dev;
	float x_mean_sqr = (x - mean)*(x - mean);
	return 1.0f / sqrtf(2.0f * M_PIf * std_dev_sqr) * exp(-x_mean_sqr / (2.0f * std_dev_sqr));
}
//Gaussian sample normal
__device__ __inline__ void ridge_sample_gaussian_normal(const float3& n, const float3& u, const float3& v,
	float3 &sampled_n, const float slope, const uint& ridge_side,const float roughness, uint& seed)
{
	if (ridge_side == 0){
		//if hit the vertical edge
		sampled_n = -v;
		return;
	}
	//if hit the slope

	float width = roughness;
	float theta;
	float phi = ridge_sample_gaussian(width/4.0f, 0, rnd(seed), rnd(seed));
	float tan_theta = ridge_sample_gaussian(width, slope, rnd(seed), rnd(seed));
	//theta = slope;
	clamp(theta, -M_PI_2f, M_PI_2f);
	clamp(phi, -M_PI_2f, M_PI_2f);
	float cos_theta = cosf(theta);
	float sin_theta = sinf(theta);
	float cos_phi = cosf(phi);
	float sin_phi = sinf(phi);
	float3 theta_n = make_float3(sin_theta * sinf(0), sin_theta * cosf(0), cos_theta);
	float3 phi_n = make_float3(sin_phi * cosf(0), sin_phi * sinf(0), cos_phi);
	sampled_n = normalize(theta_n + phi_n);
	//transform the sampled_normal coordinates from world to object
	sampled_n = normalize(u * sampled_n.x + v * sampled_n.y + n * sampled_n.z);
}
//evaluate gaussian distribution
__device__ __inline__ float ridge_evaluate_gaussian_D(const float3& n, const float3& u, const float3& v,
	float3 &sampled_n, const float slope, const float roughness,  const uint ridge_side )
{
	if (ridge_side == 0){
		//if hit the vertical edge
		return 1.0f;
	}
	//if hit the slope

	float width = roughness;
	float3 vn_proj = ridge_plane_projection(sampled_n, u);
	float3 un_proj = ridge_plane_projection(sampled_n, v);
	float cos_theta = fminf(1.0f, dot(n, vn_proj));
	float cos_phi = fminf(1.0f, dot(un_proj, n));
	float theta = acos(abs(cos_theta));
	float phi = acos(abs(cos_phi));
	if (dot(v, vn_proj) < 0.0f){
		theta = -theta;
	}
	if (dot(u, un_proj) < 0.0){
		phi = -phi;
	}
	float D_theta = ridge_evaluate_gaussian(theta, width, slope);
	float D_phi = ridge_evaluate_gaussian(phi, width/4.0f , 0.0f);
	float D = D_theta * D_phi;
	return D;
}

//evaluate beckmann distribution
__device__ __inline__ float ridge_evaluate_beckmann_D(const float3& n, const float3& u,
	const float3& v, const float3& m,const float slope, const float new_slope, 
	const float roughness, const uint ridge_side)
{
	if (ridge_side == 0){
		//if hit the vertical edge

		return 1.0f;
	}
	//if hit the slope
	float D = 0.0f;
	float threshold = 1e-6f;
	if (dot(n, m) < 0.0f || abs(dot(m,u)) > threshold)
	{
		return D;
	}
	float rough_sqr = roughness * roughness;
	float theta = new_slope - slope;
	float tan_theta = tanf(theta);
	float tan_theta_sqr = tan_theta * tan_theta;
	float cos_theta = cosf(theta);
	float cos_theta_four = cos_theta * cos_theta * cos_theta * cos_theta;
	D = exp(-tan_theta_sqr / rough_sqr) / (M_PIf * rough_sqr * cos_theta_four);
	return D;
}


__device__ __inline__ void ridge_sample_BRDF(const float ridge_angle,const float3& u, const float3& v, 
	const float3& n, const float3& w_i, const float3& diffuse_color, const float eta, const float a_u, 
	const float a_v, uint& seed, float3& m, float3& w_o, float3& brdf)
{
	float slope = M_PIf / 180.0f * ridge_angle;
	uint ridge_side = 1;
	float3 ideal_m = normalize(u * 0.0f + v * sin(slope) + n *cos(slope));
	//incoming direction projected on the plane n-v
	float3 w_p_i = ridge_plane_projection(w_i, u);
	float theta_p = acos(dot(w_p_i, n));
	//if 1 is on the slope, 0 is on the vertical edge
	float z1;
	z1 = rnd(seed);
	float z2 = rnd(seed);


	ridge_sample_anisotropic_beckmann_normal(ideal_m, u, v, m, a_u, a_v, z1, z2, ridge_side);

	float new_slope = acos(dot(n, m));
	if (dot(m, v) < 0.0f)
	{
		new_slope = -new_slope;
	}

	float cos_i_m = dot(w_i, m);
	float cos_m_n = dot(m, n);
	float G_i = ridged_G(w_p_i, n, m, new_slope, ridge_side);

	//outgoing direction
	w_o = reflect(-w_i, m);
	float3 w_p_o = ridge_plane_projection(w_o, u);
	float G_o = ridged_G(w_p_o, n, m, new_slope, ridge_side);
	float F_r, den;
	//ridge_get_weight_factors(w_i, n, m, w_p_i, ior1_over_ior2, ridge_side, F_r, den);
	F_r = fmaxf(0.0f, fresnel_R(fmaxf(0.0f, (dot(w_i, m))), eta));
	den = fabsf(dot(w_i, n) * dot(m, n) / dot(w_i, m));
	float correction_factor = fabsf(dot(w_i, m)) / fmaxf(1.0e-4, fabsf((dot(w_i, n) * dot(m, n))));
	float spec_weight = 1.0f;
	spec_weight *= G_i * G_o * F_r * correction_factor;
	float diff_weight = 1.0f;
	float F_t_i_m = 1.0f - fresnel_R(fabsf(cos_i_m), eta);
	float F_t_o_m = F_t_i_m;
	diff_weight *= F_t_i_m * F_t_o_m * G_i * G_o * correction_factor ;
	brdf = spec_weight + diff_weight *diffuse_color;
	//rtPrintf("spec_weight %f  diff_weight %f \n", spec_weight, diff_weight);
	//rtPrintf("G %f %f angle %f\n", G_i, G_o, dot(w_i, m));
	//rtPrintf("F_r %f F_t %f  %f corr factor %f \n", F_r, F_t_i_m, F_t_o_m, correction_factor);
	//rtPrintf("new %f %f %f %f %f\n", spec_weight, diff_weight, diffuse_color.x, diffuse_color.y, diffuse_color.z);
}

__device__ __inline__ void ridge_eval_BRDF(const float ridge_angle, const float3& u, const float3& v,
	const float3& n, const float3& w_i, const float3& w_l, const float3& diffuse_color, const float eta, const float a_u,
	const float a_v, uint& seed,  float3& brdf)
{
	float slope = M_PIf / 180.0f * ridge_angle;
	uint ridge_side = 1;
	float3 ideal_m = normalize(u * 0.0f + v * sin(slope) + n *cos(slope));
	//incoming direction projected on the plane n-v
	float3 w_p_i = ridge_plane_projection(w_i, u);
	float3 h = normalize((w_i + w_l) / 2.0f);
	float h_slope = acos(dot(n, h));
	float cos_h_l = dot(h, w_l);
	float cos_i_h = dot(w_i, h);
	float F_r_i_h = fresnel_R(fabsf(cos_i_h), eta);
	float F_t_i_h = 1.0f - F_r_i_h;
	float F_t_l_h = 1.0f - F_r_i_h;
	
	float3 w_p_l = ridge_plane_projection(w_l, u);
	float G_i_h = ridged_G(w_p_i, n, h, h_slope, ridge_side);
	float G_l_h = ridged_G(w_p_l, n, h, h_slope, ridge_side);
	float D = ridge_eval_anisotropic_beckmann_D(ideal_m, u, v, h, a_u, a_v, ridge_side);
	float h_den = 4 * abs(dot(w_i, n)*dot(w_l, n));
	float correction_factor = h_den;
	brdf = (diffuse_color * F_t_i_h * F_t_l_h / M_PIf + F_r_i_h)*  D *  G_i_h*G_l_h / h_den  ;

}


__device__ __inline__ float3 sample_sinusoid_normal(const float& a, const float& b, const float& c, const float3& u, const float3& v, const float3& n, uint& seed)
{
	float r = rnd_tea(seed);
	float theta = 2 * M_PIf * rnd(seed);


	float x = r * cos(theta);
	float y =  r * sin(theta);
	float z = c * fmaxf(0.0f, sqrtf(1 - x*x - y*y));
	x = a * x;
	y = b * y;
	float3 normal_vector = 2 * make_float3(x / (a*a), y /(b*b), z / (c*c));
	normal_vector = normalize(normal_vector.x * u + normal_vector.y * v + normal_vector.z * n);
	return normal_vector;
}

__device__ __inline__ float sinusoid_G(const float3& w_i, const float3& n, const float3& m, const float& wavelength, const float& amplitude, const uint& iterations)
{
	float G;
	uint chi;
	chi = dot(w_i, n) > 0.0f &&  dot(w_i, m) > 0.0f &&  dot(n, m) > 0.0f;
	if (chi < 1.0f)
		return 0.0f;


	float k = 2 * M_PIf / wavelength;
	float theta = acos(dot(w_i, m));
	float theta_w = M_PI_2f - theta;
	float slope = tan(theta_w);
	
	if (fabsf(slope / (amplitude * k)) > 1.0f)
		return 1.0f;

	float X0 = asinf(-slope / (k * amplitude)) / k;
	if (X0 <= 0.0f)
		X0 = X0 + wavelength;

	float Y0 = amplitude * cosf(k * X0);

	float X1 = wavelength / 4.0f;
	for (int idx = 0; idx < iterations; idx++) {
		X1 = X1 - (Y0 + (X1-X0) * slope - amplitude * cosf(k*X1)) / (slope + amplitude * k * sinf(k*X1) );
	}
	G = 1.0f - fabsf(X1 - X0) / wavelength;
	//rtPrintf("theta_W %f, k %f, X0 %f ,Y0 %f, X1 %f, G %f \n", theta_w,k, X0, Y0, X1, G);
	return G;
}

__device__ __inline__ void sinusoid_sample_BRDF(const float2& wavelengths, const float& amplitude, const float3& u, const float3& v,
	const float3& n, const float3& w_i, const float3& diffuse_color, const float eta, const float a_u,
	const float a_v, uint& seed, float3& m, float3& w_o, float3& brdf)
{
	float u_wavelength = wavelengths.x;
	float v_wavelength = wavelengths.y;
	m = sample_sinusoid_normal(u_wavelength/4.0, v_wavelength/4.0, amplitude, u, v, n, seed);
	
	float3 w_i_u = normalize(w_i - dot(w_i, v)*v);
	float3 w_i_v = normalize(w_i - dot(w_i, u)*u);

	w_o = reflect(-w_i, m);
	float3 w_o_u = normalize(w_o - dot(w_o, v)*v);
	float3 w_o_v = normalize(w_o - dot(w_o, u)*u);

	float u_proj =  dot(u, w_i);
	float v_proj =  dot(v, w_i);
	float2 proj = make_float2(dot(u, w_i), dot(v, w_i));
	proj = normalize(proj) * make_float2(u_wavelength, v_wavelength);
	
	float proj_wavelength = length(proj);
	//rtPrintf("lambda %f %f, lambda_proj %f %f %f \n", u_wavelength, v_wavelength, proj.x, proj.y, proj_wavelength);
	//float G_i = sinusoid_G(w_i, n, m, proj_wavelength, amplitude, 5);
	//float G_o = sinusoid_G(w_o, n, m, proj_wavelength, amplitude, 5);
	float G_i = sinusoid_G(w_i_u, n, m, u_wavelength, amplitude, 5) * sinusoid_G(w_i_v, n, m, v_wavelength, amplitude, 5);
	float G_o = sinusoid_G(w_o_u, n, m, u_wavelength, amplitude, 5) * sinusoid_G(w_o_v, n, m, v_wavelength, amplitude, 5);
	float F_r = fmaxf(0.0f, fresnel_R(fmaxf(0.0f, (dot(w_i, m))), eta));
	float correction_factor = fabsf(dot(w_i, m)) / fmaxf(1.0e-4, fabsf((dot(w_i, n) * dot(m, n))));
	float spec_weight = 1.0f;
	spec_weight *= G_i * G_o * F_r * correction_factor;
	float diff_weight = 1.0f;
	float cos_i_m = dot(w_i, m);
	float F_t_i_m = 1.0f - fresnel_R(fabsf(cos_i_m), eta);
	float F_t_o_m = F_t_i_m;
	diff_weight *= F_t_i_m * F_t_o_m * G_i * G_o * correction_factor;
	brdf = spec_weight + diff_weight *diffuse_color;
	//rtPrintf("brdf %f %f %f \n", brdf.x, brdf.y, brdf.z);

	
}

__device__ __inline__ void sinusoid_eval_BRDF(const float2& wavelengths, const float& amplitude, const float3& u, const float3& v,
	const float3& n, const float3& w_i, const float3& w_l, const float3& diffuse_color, const float eta, const float a_u,
	const float a_v, uint& seed, float3& brdf)
{
	float u_wavelength = wavelengths.x;
	float v_wavelength = wavelengths.y;
	float3 h = normalize((w_i + w_l) / 2.0f);

	float u_proj = dot(u, w_i);
	float v_proj = dot(v, w_i);
	float2 proj = make_float2(dot(u, w_i), dot(v, w_i));
	proj = normalize(proj) * make_float2(u_wavelength, v_wavelength);

	float proj_wavelength = length(proj);


	float3 w_i_u = normalize(w_i - dot(w_i, v)*v);
	float3 w_i_v = normalize(w_i - dot(w_i, u)*u);

	float3 w_l_u = normalize(w_i - dot(w_l, v)*v);
	float3 w_l_v = normalize(w_i - dot(w_l, u)*u);

	float cos_h_l = dot(h, w_l);
	float cos_i_h = dot(w_i, h);
	float F_r_i_h = fresnel_R(fabsf(cos_i_h), eta);
	float F_t_i_h = 1.0f - F_r_i_h;
	float F_t_l_h = 1.0f - F_r_i_h;
	//rtPrintf("lambda %f %f, lambda_proj %f %f %f \n", u_wavelength, v_wavelength, proj.x, proj.y, proj_wavelength);
	//float G_i = sinusoid_G(w_i, n, h, proj_wavelength, amplitude, 5);
	//float G_l = sinusoid_G(w_l, n, h, proj_wavelength, amplitude, 5);
	float G_i = sinusoid_G(w_i_u, n, h, u_wavelength, amplitude, 5) * sinusoid_G(w_i_v, n, h, v_wavelength, amplitude, 5);
	float G_l = sinusoid_G(w_l_u, n,h, u_wavelength, amplitude, 5) * sinusoid_G(w_l_v, n, h, v_wavelength, amplitude, 5);
	float F_r = fmaxf(0.0f, fresnel_R(fmaxf(0.0f, (dot(w_i, h))), eta));
	float correction_factor = fabsf(dot(w_i, h)) / fmaxf(1.0e-4, fabsf((dot(w_i, n) * dot(h, n))));
	float spec_weight = 1.0f;
	spec_weight *= G_i * G_l * F_r * correction_factor;
	float diff_weight = 1.0f;
	float cos_i_m = dot(w_i, h);
	float F_t_i_m = 1.0f - fresnel_R(fabsf(cos_i_m), eta);
	float F_t_o_m = F_t_i_m;
	diff_weight *= F_t_i_m * F_t_o_m * G_i * G_l * correction_factor;
	float h_den = 4 * abs(dot(w_i, n)*dot(w_l, n));
	brdf = (diffuse_color * F_t_i_h * F_t_l_h / M_PIf + F_r_i_h) *  dot(n, h) / M_PIf *  G_i*G_l / h_den;
	//rtPrintf("brdf %f %f %f \n", brdf.x, brdf.y, brdf.z);
}