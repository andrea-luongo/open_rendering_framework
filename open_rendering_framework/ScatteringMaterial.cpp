#include <vector>
#include <complex>
//#include "../optprops/apple_juice.h"
#include "fresnel.h"
#include "ScatteringMaterial.h"

using namespace std;
using namespace optix;

void ScatteringMaterial::getDefaultMaterial(DefaultScatteringMaterial material)
{
  current = material;
  float3& sigma_a = properties.absorption;
  float3& sigma_s = properties.scattering;
  float3& g = properties.meancosine;
  float3& ior_real = properties.ior_real;
  float3& ior_imag = properties.ior_imag;
  float& ior = properties.relative_ior;
  sigma_a = make_float3(0.0f);
  sigma_s = make_float3(0.0f);
  g = make_float3(0.0f);
  ior_real = make_float3(1.3f);
  ior_imag = make_float3(0.0f);
  ior = 1.3f;
  switch(material)
  {
  case Chicken:
    sigma_a = make_float3(0.015f, 0.077f, 0.19f);
    sigma_s = make_float3(0.15f, 0.21f, 0.38f);
    g = make_float3(0.0f, 0.0f, 0.0f);
  break;
  case Skin:
    sigma_a = make_float3(0.032f, 0.17f, 0.48f);
    sigma_s = make_float3(0.74f, 0.88f, 1.01f);
    g = make_float3(0.0f, 0.0f, 0.0f);
  break;
  case Wholemilk:
    sigma_a = make_float3(0.0011f,0.0024f,0.014f);
    sigma_s = make_float3(2.55f,3.21f,3.77f);
    g = make_float3(0.0f, 0.0f, 0.0f);
  break;
  case Whitegrapefruit:
	  sigma_a = make_float3(0.096f, 0.131f, 0.395f);
	  sigma_s = make_float3(3.513f, 3.669f, 5.237f);
	  g = make_float3(0.548f, 0.545f, 0.565f);
  break;
  case Beer:
    sigma_a = make_float3(0.1449f,0.3141f,0.7286f);
    sigma_s = make_float3(0.0037f,0.0069f,0.0074f);
    g = make_float3(0.917f,0.956f,0.982f);
  break;
  case Soymilk:
    sigma_a = make_float3(0.0001f,0.0005f,0.0034f);
    sigma_s = make_float3(2.433f,2.714f,4.563f);
    g = make_float3(0.873f,0.858f,0.832f);
  break;
  case Coffee:
    sigma_a = make_float3(0.1669f,0.2287f,0.3078f);
    sigma_s = make_float3(0.2707f,0.2828f,0.297f);
    g = make_float3(0.907f,0.896f,0.88f);
  break;
  case Marble:
    ior_real = make_float3(1.5f);
    ior = 1.5f;
    sigma_a = make_float3(0.0021f,0.0041f,0.0071f);
    sigma_s = make_float3(2.19f,2.62f,3.00f);
    g = make_float3(0.0f,0.0f,0.0f);
  break;
  case Potato:
    sigma_a = make_float3(0.0024f,0.0090f,0.12f);
    sigma_s = make_float3( 0.68f,0.70f,0.55f);
    g = make_float3(0.0f, 0.0f, 0.0f);
  break;
  case Ketchup:
    sigma_a = make_float3(0.061f,0.97f,1.45f);
    sigma_s = make_float3(0.18f,0.07f,0.03f);
    g = make_float3(0.0f,0.0f,0.0f);
  break;
  case Apple:
    sigma_a = make_float3(0.0030f,0.0034f,0.0046f);
    sigma_s = make_float3(2.29f,2.39f,1.97f);
    g = make_float3(0.0f,0.0f,0.0f);
  break;
    case ChocolateMilk:
		sigma_s = make_float3(0.7352f, 0.9142f, 1.0588f);
		sigma_a = make_float3(0.7359f, 0.9172f, 1.0688f) - sigma_s;
		g = make_float3(0.862f, 0.838f, 0.806f);
  break;
    case ReducedMilk:
    sigma_a = make_float3(0.0001f, 0.0002f, 0.0005f);
    sigma_s = make_float3(10.748f, 12.209f, 13.931f);
    g = make_float3(0.819f, 0.797f, 0.746f);
  break;
    case Mustard:
    sigma_s = make_float3(16.447f,18.536f,6.457f);
    sigma_a = make_float3(0.057f,0.061f,0.451f);
    g = make_float3(0.155f,0.173f, 0.351f );
    break;
    case Shampoo:
    sigma_s = make_float3(8.111f,9.919f,10.575f);
    sigma_a = make_float3(0.178f,0.328f,0.439f);
    g = make_float3(0.907f,0.882f,0.874f);
    break;
    case MixedSoap:
    sigma_s = make_float3(3.923f, 4.018f, 4.351f);
    sigma_a = make_float3(0.003f, 0.005f, 0.013f);
    g = make_float3(0.330f, 0.322f, 0.316f);
    break;
    case GlycerineSoap:
    sigma_s = make_float3(0.201f, 0.202f, 0.221f);
    sigma_a = make_float3(0.001f, 0.001f, 0.002f);
    g = make_float3(0.955f, 0.949f, 0.943f);
    break;
  }
  sigma_a *= scale;
  sigma_s *= scale;
  computeCoefficients();
}

void ScatteringMaterial::loadParameters(const char* name, Context& context) 
{
	context[name]->setUserData(sizeof(ScatteringMaterialProperties), &properties);
}

void ScatteringMaterial::loadParameters(const char* name, GeometryInstance& gi)
{
  float ior = properties.relative_ior;
  float3 ior_real = properties.ior_real;
  if(gi["ior"]->getFloat() < 1.0f)
    computeCoefficients(make_float3(1.52103f, 1.52494f, 1.53219f));
  else
    computeCoefficients();
  gi[name]->setUserData(sizeof(ScatteringMaterialProperties), &properties);
  properties.relative_ior = ior;
  properties.ior_real = ior_real;
}

void ScatteringMaterial::computeCoefficients(float3 ior_outside)
{
  float3 ior_rgb = properties.ior_real/ior_outside;
  float3 reducedScattering = properties.scattering*(1.0f - properties.meancosine);
  properties.relative_ior /= (ior_outside.x + ior_outside.y + ior_outside.z)/3.0f;
  properties.ior_real = ior_rgb;
  properties.extinction = properties.scattering + properties.absorption;
  properties.reducedExtinction = reducedScattering + properties.absorption;
  properties.deltaEddExtinction = properties.scattering*(1.0f - properties.meancosine*properties.meancosine) + properties.absorption;
  properties.D = make_float3(1.0f)/(3.0f*properties.reducedExtinction);
  properties.transport = sqrtf(properties.absorption/properties.D);
  properties.C_phi = make_float3(C_phi(ior_rgb.x), C_phi(ior_rgb.y), C_phi(ior_rgb.z));
  properties.C_phi_inv = make_float3(C_phi(1.0f/ior_rgb.x), C_phi(1.0f/ior_rgb.y), C_phi(1.0f/ior_rgb.z));
  properties.C_E = make_float3(C_E(ior_rgb.x), C_E(ior_rgb.y), C_E(ior_rgb.z));
  properties.albedo = properties.scattering/properties.extinction;
  properties.reducedAlbedo = reducedScattering/properties.reducedExtinction;
  properties.de = 2.131f*properties.D/sqrtf(properties.reducedAlbedo);
  properties.A = (1.0f - properties.C_E)/(2.0f*properties.C_phi);
  properties.three_D = 3.0f*properties.D;
  properties.two_A_de = 2.0f*properties.A*properties.de;
  properties.global_coeff = make_float3(1.0f)/(4.0f*properties.C_phi_inv) * 1.0f/(4.0f*M_PIf*M_PIf);
  properties.one_over_three_ext = make_float3(1.0)/(3.0f*properties.extinction);
  properties.mean_transport = (properties.transport.x + properties.transport.y + properties.transport.z)/3.0f;
  //properties.min_transport = fminf(fminf(properties.transport.x, properties.transport.y), properties.transport.z);
}

