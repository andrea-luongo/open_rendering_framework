#pragma once

#include <vector>
#include <optix_world.h>
//#include "../optprops/apple_juice.h"
#include "structs.h"
//#include "ObjScene.h"

class ScatteringMaterial
{
public:

  ScatteringMaterial(float indexOfRefraction = 1.0f,
                      optix::float3 absorption = optix::make_float3(1.0f),
                      optix::float3 scattering = optix::make_float3(1.0f),
                      optix::float3 meancosine = optix::make_float3(0.0f) )
    : current(static_cast<DefaultScatteringMaterial>(0)), scale(1.0f), concentration(0.5f)
  {
    properties.relative_ior = indexOfRefraction;
    properties.ior_real = optix::make_float3(indexOfRefraction);
    properties.ior_imag = absorption*optix::make_float3(700.0e-9f, 550.0e-9f, 425.0e-9f)/(4.0f*M_PIf);
    properties.absorption = absorption;
    properties.scattering = scattering;
    properties.meancosine = meancosine;
    computeCoefficients();
  }

	ScatteringMaterialProperties properties;

	static enum DefaultScatteringMaterial{
		Apple,
		Marble,
		Potato,
		Skin,
		ChocolateMilk,
		Soymilk,
		Whitegrapefruit,
		ReducedMilk,
		Ketchup,
		Wholemilk,
		Chicken,
		Beer,
		Coffee,
		Shampoo,
		Mustard,
		MixedSoap,
		GlycerineSoap
	};

  ScatteringMaterial(DefaultScatteringMaterial material, float prop_scale = 100.0f)
    : current(material),
      scale(prop_scale),
      concentration(1.0f)
  {
    getDefaultMaterial(material);
  }

  void getDefaultMaterial(DefaultScatteringMaterial material);
  void loadParameters(const char* name, optix::Context& context);
  void loadParameters(const char* name, optix::GeometryInstance& gi);
  void computeCoefficients(optix::float3 ior_outside = optix::make_float3(1.0f));
  void shrink() { scale /= 1.5; properties.absorption /= 1.5; properties.scattering /= 1.5; computeCoefficients(); }
  void grow() { scale *= 1.5; properties.absorption *= 1.5; properties.scattering *= 1.5; computeCoefficients(); }
  float get_scale() const { return scale; }

  void set_concentration(float C)
  {
    concentration = C;
    getDefaultMaterial(current);
  }
  float get_concentration() const { return concentration; }

 


private:
  DefaultScatteringMaterial current;
  float scale;
  float concentration;

};

