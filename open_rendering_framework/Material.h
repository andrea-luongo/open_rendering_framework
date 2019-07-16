#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include <QVector3D>
#include <QJsonArray>
#include <qobject.h>
#include <QVector>
#include "structs.h"
#include <QtWidgets>
#include "MyComplex.h"

class MyMaterial : public QObject
{
public:
	MyMaterial() { };
	~MyMaterial() { };
	MaterialType getType() const { return type; };
	optix::Material getOptixMaterial() const { return mtl; };
	 QTableWidget* getParametersTable() const { return table; };
	 virtual void writeJSON(QJsonObject &json) const = 0;

protected:
	virtual void loadFromJSON(const QJsonObject &json) = 0;
	optix::Context context;
	MaterialType type;
	optix::Material mtl;
	QString shader_name;
	QTableWidget* table;
};



class DiffuseMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit DiffuseMaterial(optix::Context c);
	explicit DiffuseMaterial(optix::Context c, const QJsonObject &json);
	void setColor(optix::float3 c);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateColorFromTable(int row);
	void initTable();
	void initPrograms();

	//parameters
	optix::float3 color;
private:
	enum TableRows
	{
		color_row,
		table_rows
	};

public slots:
	void tableUpdate(int row, int column);
};

class FlatMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit FlatMaterial(optix::Context c);
	explicit FlatMaterial(optix::Context c, const QJsonObject &json);
	void setColor(optix::float3 c);
	void setFlatShadow(uint f);
	void setHighlightOn(uint f);
	void setHighlightColor(optix::float3 c);
	void setShininess(float s);
	void setRoughness(optix::float2 r);
	void setNormalDistribution(NormalsDistribution n);
	void setIor(float i);
	void setThreshold(float t);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateColorFromTable(int row);
	void updateHighlightColorFromTable(int row);
	void updateShininessFromTable(int row);
	void updateRoughnessFromTable(int row);
	void updateIORFromTable(int row);
	void updateThresholdFromTable(int row);
	void initTable();
	void initPrograms();

	//parameters
	optix::float3 color;
	uint flat_shadow;
	uint highlight;
	optix::float3 highlight_color;
	float shininess;
	float ior;
	float highlight_threshold;
	optix::float2 roughness; 
	NormalsDistribution n_distribution;

	enum TableRows
	{
		color_row,
		shadow_row,
		highlight_row,
		high_color_row ,
		shininess_row ,
		roughness_row,
		ior_row,
		n_dist_row ,
		threshold_row ,
		table_rows
	};

	//uint color_row = 0;
	//uint shadow_row = 1;
	//uint highlight_row = 2;
	//uint high_color_row = 3;
	//uint shininess_row = 4;
	//uint roughness_row = 5;
	//uint ior_row = 6;
	//uint n_dist_row = 7;
	//uint threshold_row = 8;

public slots:
	void tableUpdate(int row, int column);
	void updateFlatShadow(int row);
	void updateHighlightOn(int row);
	void updateNormalsDistributionFromTable(int new_value);
};

class LambertianInterfaceMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit LambertianInterfaceMaterial(optix::Context c);
	explicit LambertianInterfaceMaterial(optix::Context c, const QJsonObject &json);
	void setColor(optix::float3 c);
	void setIor(float i);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateColorFromTable(int row);
	void updateIORFromTable(int row);
	void initTable();
	void initPrograms();

	//parameters
	optix::float3 rho_d;
	float ior;
	public slots:
	void tableUpdate(int row, int column);

private:
	enum TableRows
	{
		rho_d_row,
		ior_row,
		table_rows
	};
	//uint rho_d_row = 0;
	//uint ior_row = 1;
};

class RoughDiffuseMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit RoughDiffuseMaterial(optix::Context c);
	explicit RoughDiffuseMaterial(optix::Context c, const QJsonObject &json);
	void setColor(optix::float3 c);
	void setIor(float i);
	void setRoughness(optix::float2 r);
	void setNormalDistribution(NormalsDistribution n);
	void setMicrofacetModel(MicrofacetModel m);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateColorFromTable(int row);
	void updateIORFromTable(int row);
	void updateRoughnessFromTable(int row);
	void initTable();
	void initPrograms();

	//parameters
	optix::float3 rho_d;
	float ior;
	optix::float2 roughness;
	NormalsDistribution n_distribution;
	MicrofacetModel m_model;

public slots:
	void tableUpdate(int row, int column);
	void updateNormalsDistributionFromTable(int new_value);
	void updateMicrofacetModelFromTable(int new_value);

private:
	enum TableRows
	{
		rho_d_row,
		ior_row,
		roughness_row, 
		n_dist_row,
		m_model_row,
		table_rows
	};
};

class NormalMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit NormalMaterial(optix::Context c);
	explicit NormalMaterial(optix::Context c, const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void initTable();
	void initPrograms();
	//parameters

public slots:
	void tableUpdate(int row, int column);
};

class TransparentMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit TransparentMaterial(optix::Context c);
	explicit TransparentMaterial(optix::Context c, const QJsonObject &json);
	void setAbsorption(optix::float3 c);
	void setIor(float ior);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateAbsorptionFromTable(int row);
	void updateIORFromTable(int row);
	void initTable();
	void initPrograms();

	//parameters
	float index_of_refraction;
	optix::float3 glass_absorption;
private:
	enum TableRows
	{
		absoprtion_row,
		ior_row,
		table_rows
	};

public slots:
	void tableUpdate(int row, int column);
};

class RoughTransparentMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit RoughTransparentMaterial(optix::Context c);
	explicit RoughTransparentMaterial(optix::Context c, const QJsonObject &json);
	void setAbsorption(optix::float3 c);
	void setIor(float ior);
	void setRoughness(float roughness);
	void setMicrofacetModel(MicrofacetModel m);
	void setNormalDistribution(NormalsDistribution n);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateAbsorptionFromTable(int row);
	void updateIORFromTable(int row);
	void updateRoughnessFromTable(int row);
	void initTable();
	void initPrograms();
	//parameters
	float index_of_refraction;
	float roughness;
	MicrofacetModel m_model;
	NormalsDistribution n_distribution;
	optix::float3 glass_absorption;

private:
	enum TableRows
	{
		absoprtion_row,
		ior_row,
		roughness_row,
		n_dist_row,
		m_model_row,
		table_rows
	};
	
public slots:
	void tableUpdate(int row, int column);
	void updateMicrofacetModelFromTable(int new_value);
	void updateNormalsDistributionFromTable(int new_value);
};

class MetallicMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit MetallicMaterial(optix::Context c);
	explicit MetallicMaterial(optix::Context c, const QJsonObject &json);
	void setIor(MyComplex3 ior);
	void setRoughness(optix::float2 roughness);
	void setMicrofacetModel(MicrofacetModel m);
	void setNormalDistribution(NormalsDistribution n);
	void writeJSON(QJsonObject &json) const;

protected:
	void loadFromJSON(const QJsonObject &json);
	void updateIORFromTable(int row);
	void updateRoughnessFromTable(int row);

	void initTable();
	void initPrograms();

	//parameters
	MyComplex3 index_of_refraction;
	MyComplex3 gold = { MyComplex{ 0.170265, 3.01893 }, MyComplex{ 0.511516, 2.44734 }, MyComplex{ 1.47544, 1.87263 } };
	MyComplex3 silver = { MyComplex{ 0.12614, 3.80301 }, MyComplex{ 0.126421, 3.24418 }, MyComplex{ 0.150691, 2.45234 } };
	MyComplex3 copper = { MyComplex{ 0.407567, 3.03073 }, MyComplex{ 0.944315, 2.62346 }, MyComplex{ 1.16966, 2.38102 } };
	MyComplex3 alum = { MyComplex{ 1.24134, 7.35646 }, MyComplex{ 0.918662, 6.54687 }, MyComplex{ 0.614598, 5.45122 } };

	optix::float2 roughness;
	MicrofacetModel m_model;
	NormalsDistribution n_distribution;

private:
	enum TableRows
	{
		default_ior_row,
		x_ior_row,
		y_ior_row,
		z_ior_row,
		roughness_row,
		m_model_row,
		n_dist_row,
		table_rows
	};
	
	public slots:
	void tableUpdate(int row, int column);
	void updateMicrofacetModelFromTable(int new_value);
	void updateNormalsDistributionFromTable(int new_value);
	void updateCustomIOR(int new_value);
};

class TranslucentMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit TranslucentMaterial(optix::Context c);
	explicit TranslucentMaterial(optix::Context c, const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	
	void getDefaultMaterial(DefaultScatteringMaterial material);
	void computeCoefficients(optix::float3 ior_outside = optix::make_float3(1.0f));
	void loadParameters(const char* name);
	float getScale() const { return scale; };
	float getIor() const { return ior; };
	void setIor(float i);
	void setScale(float s);
	void setAbsorption(optix::float3 a);
	void setScattering(optix::float3 s);
	void setMeanCosine(optix::float3 m);

protected:
	void loadFromJSON(const QJsonObject &json);
	void initTable();
	void initPrograms();
	void updateIORFromTable(int row);
	void updateScaleFromTable(int row);
	void updateAbsorptionFromTable(int row);
	void updateScatteringFromTable(int row);
	void updateMeanCosineFromTable(int row);
	
	//parameters
	ScatteringMaterialProperties properties;
	float scale;
	//float concentration;
	float ior;
	optix::float3 absorption;
	optix::float3 scattering;
	optix::float3 meancosine;
	DefaultScatteringMaterial current;
	DipoleModel dipole_model;

private:
	enum TableRows
	{
		def_material_row,
		ior_row,
		scale_row,
		absorption_row,
		scattering_row,
		meancosine_row,
		dipole_row,
		table_rows
	};

public slots:
	void tableUpdate(int row, int column);
	void updateDefaultMaterial(int material);
	void updateDipoleModel(int model);
};

class RoughTranslucentMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit RoughTranslucentMaterial(optix::Context c);
	explicit RoughTranslucentMaterial(optix::Context c, const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;

	void getDefaultMaterial(DefaultScatteringMaterial material);
	void computeCoefficients(optix::float3 ior_outside = optix::make_float3(1.0f));
	void loadParameters(const char* name);
	float getScale() const { return scale; };
	float getIor() const { return ior; };
	optix::float2 getRoughness() const { return roughness; };
	void setIor(float i);
	void setScale(float s);
	void setAbsorption(optix::float3 a);
	void setScattering(optix::float3 s);
	void setMeanCosine(optix::float3 m);
	void setRoughness(optix::float2 roughness);
	void setMicrofacetModel(MicrofacetModel m);
	void setNormalDistribution(NormalsDistribution n);

protected:
	void loadFromJSON(const QJsonObject &json);
	void initTable();
	void initPrograms();
	void updateIORFromTable(int row);
	void updateScaleFromTable(int row);
	void updateAbsorptionFromTable(int row);
	void updateScatteringFromTable(int row);
	void updateMeanCosineFromTable(int row);
	void updateRoughnessFromTable(int row);
	//parameters
	ScatteringMaterialProperties properties;
	float scale;
	//float concentration;
	float ior;
	optix::float3 absorption;
	optix::float3 scattering;
	optix::float3 meancosine;
	DefaultScatteringMaterial current;
	DipoleModel dipole_model;
	optix::float2 roughness;
	MicrofacetModel m_model;
	NormalsDistribution n_distribution;

private:
	enum TableRows
	{
		def_material_row,
		ior_row,
		scale_row,
		absorption_row,
		scattering_row,
		meancosine_row,
		dipole_row,
		roughness_row,
		m_model_row,
		n_dist_row,
		table_rows
	};

	public slots:
	void tableUpdate(int row, int column);
	void updateDefaultMaterial(int material);
	void updateDipoleModel(int model);
	void updateMicrofacetModelFromTable(int new_value);
	void updateNormalsDistributionFromTable(int new_value);
};

class AnisotropicMaterial : public MyMaterial
{
	Q_OBJECT
public:
	explicit AnisotropicMaterial(optix::Context c);
	explicit AnisotropicMaterial(optix::Context c, const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	void setIor(float i);
	void setRoughness(optix::float2 roughness);
	void setType(AnisotropicStructure type);
	void setRidgeAngle(float angle);
	void setColor(optix::float3 c);
	void setSinusoidWavelengths(optix::float2 f);
	void setSinusoidAmplitude(float a);
	void setTexture(QString t);
	void setPatternAngle(float angle);
protected:
	void loadFromJSON(const QJsonObject &json);//protected:
	void initTable();
	void initPrograms();
	void updateIORFromTable(int row);
	void updateRoughnessFromTable(int row);
	void updateRidgeAngleFromTable(int row);
	void updateColorFromTable(int row);
	void updateSinusoidAmplitudeFromTable(int row);
	void updateSinusoidWavelengthsFromTable(int row);
	void updatePatternAngleFromTable(int row);

	AnisotropicStructure structure_type;
	optix::float2 roughness;
	optix::float3 rho_d;
	float ior;
	float ridge_angle;
	optix::float2 sinusoid_wavelengths;
	float sinusoid_amplitude;
	QString texture_path;
	optix::TextureSampler sampler;
	float pattern_angle;
private:
	enum TableRows
	{
		structure_row,
		texture_row,
		rho_d_row,
		ior_row,
		roughness_row,
		ridge_angle_row,
		sinusoid_amplitude_row,
		sinusoid_wavelengths_row,
		pattern_angle_row,
		table_rows
	};

public slots:
	void tableUpdate(int row, int column);
	void updateTypeFromTable(int type);
	void updateTextureFromTable();

};