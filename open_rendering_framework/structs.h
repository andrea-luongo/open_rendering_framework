#pragma once
#include <optix_world.h>
#include "random.h"

enum RayTypes
{
	radiance_ray_type,
	shadow_ray_type,
	depth_ray_type,
	NUMBER_OF_RAYS
};

enum RayTracingEntries
{
	integrator_pass,
	sample_camera_pass,
	NUMBER_OR_ENTRIES
};
// Payload for radiance ray type
struct PerRayData_radiance
{
	optix::float3 result;
	int emit_light;
	int depth;
	unsigned int seed;
	Seed64 seed64;
};

// Payload for shadow ray type
struct PerRayData_shadow
{
	float attenuation;
};

struct PerRayData_depth
{
	optix::float3 normal;
	float ray_depth;
	unsigned int seed;
};

struct LightStruct
{
	unsigned int light_type;
	optix::float3 emitted_radiance;
	optix::float3 light_parameters;
	optix::float3 padding;
};

struct PointLightStruct
{
	unsigned int light_type;
	optix::float3 emitted_radiance;
	optix::float3 position;
	optix::float3 padding;
};

struct DirectionalLightStruct
{
	unsigned int light_type;
	optix::float3 emitted_radiance;
	optix::float3 direction;
	optix::float3 padding;
};

struct TrianglesAreaLightStruct
{
	unsigned int light_type;
	optix::float3 emitted_radiance;
	float triangle_count;
	float buffer_start_idx;
	float buffer_end_idx;
	optix::float3 padding;
};

struct DiskLightStruct
{
	unsigned int light_type;
	optix::float3 emitted_radiance;
	optix::float3 position;
	float radius;
	float phi;
	float theta;
};

struct SphericalLightStruct
{
	unsigned int light_type;
	optix::float3 emitted_radiance;
	optix::float3 position;
	float radius;
	optix::float2 padding;
};


struct TriangleLight
{
	optix::float3 v0, v1, v2;
	optix::float3 n0, n1, n2;
	unsigned int has_normals;
	optix::float3 emission;
	float area;
};

struct PositionSample
{
	optix::float3 pos;
	optix::float3 dir;
	optix::float3 normal;
	optix::float3 L;
	optix::float3 transmitted;
	optix::float3 weight;
	optix::float2 padding;
};

struct ScatteringMaterialProperties
{
	// base parameters
	float relative_ior;     // average reletive index of refraction (IOR inside divided by IOR outside)
	optix::float3 ior_real;
	optix::float3 ior_imag;
	optix::float3 absorption;
	optix::float3 scattering;
	optix::float3 meancosine;

	// derived parameters
	optix::float3 extinction;
	optix::float3 reducedExtinction;
	optix::float3 deltaEddExtinction;
	optix::float3 D;
	optix::float3 transport;
	optix::float3 C_phi;
	optix::float3 C_phi_inv;
	optix::float3 C_E;
	optix::float3 A;
	optix::float3 albedo;
	optix::float3 reducedAlbedo;
	optix::float3 de;
	optix::float3 three_D;
	optix::float3 two_A_de;
	optix::float3 global_coeff;
	optix::float3 one_over_three_ext;
	float mean_transport;
};

enum LightType
{
	POINT_LIGHT,
	DIRECTIONAL_LIGHT,
	TRIANGLES_AREA_LIGHT,
	DISK_LIGHT,
	SPHERICAL_LIGHT,
	NUMBER_OF_LIGHTS
};

static char *lightNames[] = {
	"PointLight",
	"DirectionalLight",
	"TrianglesAreaLight",
	"DiskAreaLight",
	"SphericalLight"
};

enum CameraType
{
	PINHOLE_CAMERA,
	NUMBER_OF_CAMERAS
};

static char *cameraNames[] = {
	"PinholeCamera"
};

enum GeometryType
{
	OBJ,
	NUMBER_OF_GEOMETRIES
};

static char *geometryNames[] = {
	"obj"
};

enum BackgroundType
{
	CONSTANT_BACKGROUND,
	ENVMAP_BACKGROUND,
	NUMBER_OF_BACKGROUND
};

static char *backgroundNames[] = {
	"ConstantBackground",
	"EnvMapBackground"
};


enum IntegratorType
{
	PATH_TRACER,
	DEPTH_TRACER,
	NUMBER_OF_INTEGRATOR
};

static char *integratorNames[] = {
	"PathTracer",
	"DepthTracer"
};

enum MaterialType
{

	DIFFUSE_SHADER,
	NORMAL_SHADER,
	TRANSPARENT_SHADER,
	ROUGH_TRANSPARENT_SHADER,
	METALLIC_SHADER,
	TRANSLUCENT_SHADER,
	ROUGH_TRANSLUCENT_SHADER,
	LAMBERTIAN_INTERFACE_SHADER,
	FLAT_SHADER,
	ROUGH_DIFFUSE_SHADER,
	ANISOTROPIC_MATERIAL,
	NUMBER_OF_MATERIALS

};

static char *materialNames[] = {
	"diffuse_shader",
	"normal_shader",
	"transparent_shader",
	"rough_transparent_shader",
	"metallic_shader",
	"translucent_shader",
	"rough_translucent_shader",
	"lambertian_interface_shader",
	"flat_shader",
	"rough_diffuse_shader",
	"anisotropic_structure_shader"
};


enum MicrofacetModel
{
	WALTER_MODEL,
	VISIBLE_NORMALS_MODEL,
	MULTISCATTERING_MODEL,
	NUMBER_OF_MICROFACET_MODELS
};

static char *microfacetModelNames[] = {
	"walter_model",
	"visible_normals_model",
	"multiscattering_model"
};

enum NormalsDistribution
{
	BECKMANN_DISTRIBUTION,
	GGX_DISTRIBUTION,
	NUMBER_OF_NORMALS_DISTRIBUTION
};

static char *normalDistributionNames[] = {
	"beckmann_distribution",
	"ggx_distribution"
};

static enum DefaultScatteringMaterial {
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
	GlycerineSoap,
	Custom,
	NUMBER_OF_DEFAULT_SCATTERING_MATERIALS
};

static char *DefaultScatteringMaterialNames[] = {
	"Apple",
	"Marble",
	"Potato",
	"Skin",
	"ChocolateMilk",
	"Soymilk",
	"Whitegrapefruit",
	"ReducedMilk",
	"Ketchup",
	"Wholemilk",
	"Chicken",
	"Beer",
	"Coffee",
	"Shampoo",
	"Mustard",
	"MixedSoap",
	"GlycerineSoap",
	"Custom"
};

enum DipoleModel
{
	STANDARD_DIPOLE,
	DIRECTIONAL_DIPOLE,
	NUMBER_OF_DIPOLE_MODELS
};

enum AnisotropicStructure
{
	RIDGED_STRUCTURE,
	SINUSOIDAL_STRUCTURE,
	TEXTURE_STRUCTURE,
	NUMBER_OF_STRUCTURES,
};

static char *anisotropicStructureNames[] = {
	"ridged_structure",
	"sinusoidal_structure",
	"texture_structure"
};