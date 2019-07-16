#include "OptixSceneLoader.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include "OptixScene.h"
#include "sampleConfig.h"

OptixSceneLoader::OptixSceneLoader(optix::Context c)
{
	context = c;
	SAMPLES_FRAME = 500;
	ss_samples = context->createBuffer(RT_BUFFER_OUTPUT);
	ss_samples->setFormat(RT_FORMAT_USER);
	ss_samples->setElementSize(sizeof(PositionSample));
	ss_samples->setSize(0);
	context["samples_output_buffer"]->set(ss_samples);
	context["samples"]->setUint(SAMPLES_FRAME);
}

OptixSceneLoader::~OptixSceneLoader()
{
	delete integrator, background, camera;
	foreach(const Light* light, lights) {
		delete light;
	}
	foreach(const Geometry* geometry, geometries) {
		delete geometry;
	}
}

bool OptixSceneLoader::loadJSONScene(const QString scene_path, uint& width, uint& height, QString& buffer_path, unsigned int& frame_count)
{
	QFile loadFile(scene_path);
	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning("Couldn't open scene file.");
		return false;
	}

	QByteArray saveData = loadFile.readAll();
	QJsonParseError err;
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData, &err));

	readJSON(loadDoc.object(), width, height, buffer_path, frame_count);
	loadFile.close();
	return true;
}

bool OptixSceneLoader::saveJSONScene(QString scene_path, uint frame_count)
{

	QFileInfo fileInfo = scene_path;
	QString pathWithoutExtension = fileInfo.absolutePath() + "/" + fileInfo.baseName();
	QString jsonPath = pathWithoutExtension + ".json";
	QString bufferPath = pathWithoutExtension + ".raw";
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	bufferPath = dir.relativeFilePath(bufferPath);
	QFile saveFile(jsonPath);
	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return false;
	}

	QJsonObject sceneObject;
	sceneObject["frame_count"] = (int) frame_count;
	sceneObject["buffer_path"] = bufferPath;
	writeJSON(sceneObject);
	QJsonDocument saveDoc(sceneObject);
	saveFile.write(saveDoc.toJson());
	saveFile.close();
	return true;

}

void OptixSceneLoader::writeJSON(QJsonObject &json)
{
	QJsonObject integratorObject;
	integrator->writeJSON(integratorObject);
	json["Integrator"] = integratorObject;

	QJsonObject backgroundObject;
	background->writeJSON(backgroundObject);
	json["Background"] = backgroundObject;

	QJsonObject cameraObject;
	camera->writeJSON(cameraObject);
	json["Camera"] = cameraObject;

	QJsonArray geometryArray;
	foreach(const Geometry* geometry, geometries) {
		QJsonObject geometryObject;
		geometry->writeJSON(geometryObject);
		geometryArray.append(geometryObject);
	}
	json["Geometries"] = geometryArray;

	QJsonArray lightArray;
	foreach(const Light* light, lights) {
		QJsonObject lightObject;
		light->writeJSON(lightObject);
		lightArray.append(lightObject);
	}
	json["Lights"] = lightArray;
}

void OptixSceneLoader::readJSON(const QJsonObject &json, uint& width, uint& height, QString& buffer_path, unsigned int& frame_count)
{	
	if (json.contains("frame_count") && json["frame_count"].isDouble()) {
		frame_count = json["frame_count"].toInt();	
	}

	if (json.contains("buffer_path") && json["buffer_path"].isString()) {
		//buffer_path = json["buffer_path"].toString();
		QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
		QFileInfo fileInfo(dir, json["buffer_path"].toString());
		buffer_path = (fileInfo.absoluteFilePath());
	}

	if (json.contains("Integrator") && json["Integrator"].isObject()) {
		QJsonObject integratorObject = json["Integrator"].toObject();
		integrator = readIntegrator(integratorObject);
	}


	if (json.contains("Background") && json["Background"].isObject()) {
		QJsonObject backgroundObject = json["Background"].toObject();
		background = readBackground(backgroundObject);
	}

	if (json.contains("Camera") && json["Camera"].isObject()) {
		QJsonObject cameraObject = json["Camera"].toObject();
		camera = readCamera(cameraObject, width, height);
	}

	obj_group = context->createGroup();
	obj_group->setChildCount(0);
	optix::Acceleration acceleration = context->createAcceleration("Trbvh", "Bvh");
	obj_group->setAcceleration(acceleration);
	if (json.contains("Geometries") && json["Geometries"].isArray()) {	
		QJsonArray geometryArray = json["Geometries"].toArray();
		geometries.clear();
		translucentObjects.clear();
		geometries.reserve(geometryArray.size());

		for (int geometryIndex = 0; geometryIndex < geometryArray.size(); ++geometryIndex) {
			QJsonObject geometryObject = geometryArray[geometryIndex].toObject();
			Geometry* geometry = readGeometry(geometryObject);
			geometries.append(geometry);

		}
	}

	std::string ptx_path = OptixScene::ptxPath(SAMPLE_NAME, "sample_camera.cu");
	optix::Program ray_gen_program = context->createProgramFromPTXFile(ptx_path, "sample_camera");
	context->setRayGenerationProgram(1, ray_gen_program);
	computeTranslucentGeometries();


	initLightBuffers();
	if (json.contains("Lights") && json["Lights"].isArray()) {
		QJsonArray lightArray = json["Lights"].toArray();
		lights.clear();
		lights.reserve(lightArray.size());
		lightStructData.clear();
		lightStructData.reserve(lightArray.size());
		for (int lightIndex = 0; lightIndex < lightArray.size(); ++lightIndex) {
			QJsonObject lightObject = lightArray[lightIndex].toObject();
			LightStruct a;
			LightStruct* light_data = new LightStruct();
			readLight(lightObject, light_data);

		}
		updateLights(true);
	}
	obj_group->validate();
	context["top_object"]->set(obj_group);
	context["top_shadower"]->set(obj_group);
}

Integrator* OptixSceneLoader::readIntegrator(const QJsonObject &integratorObject) {

	QString integratorType;
	Integrator* integrator;
	if (integratorObject.contains("type") && integratorObject["type"].isString()) {
		integratorType = integratorObject["type"].toString();
	}

	if (integratorType.compare(QString("PathTracer"), Qt::CaseInsensitive) == 0) {
		integrator = new PathTracer(context);
		integrator->readJSON(integratorObject);
	}

	if (integratorType.compare(QString("DepthTracer"), Qt::CaseInsensitive) == 0) {
		integrator = new DepthTracer(context);
		integrator->readJSON(integratorObject);
	}
	return integrator;
}

Background* OptixSceneLoader::readBackground(const QJsonObject &backgroundObject) {

	QString backgroundType;
	Background* bg;
	if (backgroundObject.contains("type") && backgroundObject["type"].isString()) {
		backgroundType = backgroundObject["type"].toString();
	}

	if (backgroundType.compare(QString("ConstantBackground"), Qt::CaseInsensitive) == 0) {
		bg = new ConstantBackground(context);
		bg->readJSON(backgroundObject);
	}

	if (backgroundType.compare(QString("EnvMapBackground"), Qt::CaseInsensitive) == 0) {
		bg = new EnvMapBackground(context);
		bg->readJSON(backgroundObject);
	}
	return bg;
}

Camera* OptixSceneLoader::readCamera(const QJsonObject &cameraObject, uint& width, uint& height) {

	QString cameraType;
	Camera* c;
	if (cameraObject.contains("type") && cameraObject["type"].isString()) {
		cameraType = cameraObject["type"].toString();
	}

	if (cameraType.compare(QString("PinholeCamera"), Qt::CaseInsensitive) == 0) {
		c = new PinholeCamera(context);
		c->readJSON(cameraObject, width, height);
	}

	return c;
}

Geometry* OptixSceneLoader::readGeometry(const QJsonObject &geometryObject) 
{
	QString geometryType;
	Geometry* g;
	if (geometryObject.contains("type") && geometryObject["type"].isString()) {
		geometryType = geometryObject["type"].toString();
	}

	if (geometryType.compare(QString("obj"), Qt::CaseInsensitive) == 0) {
		g = new OBJGeometry(context, geometryObject);
	}
	optix::Transform& tt = g->getTransform();
	obj_group->addChild(tt);
	obj_group->getAcceleration()->markDirty();
	//QObject::connect(g, &Geometry::updated_translucent, this, &OptixSceneLoader::computeTranslucentGeometries);
	return g;
}

void OptixSceneLoader::readLight(const QJsonObject &lightObject, LightStruct* light_data)
{
	QString lightType;
	if (lightObject.contains("type") && lightObject["type"].isString()) {
		lightType = lightObject["type"].toString();
	}

	if (lightType.compare(QString("PointLight"), Qt::CaseInsensitive) == 0) {
		PointLight* pl = new PointLight(context);
		pl->readJSON(lightObject);
		addLight(pl);
	} else if (lightType.compare(QString("DirectionalLight"), Qt::CaseInsensitive) == 0) {
		DirectionalLight* dl = new DirectionalLight(context);
		dl->readJSON(lightObject);
		addLight(dl);
	} else if (lightType.compare(QString("TrianglesAreaLight"), Qt::CaseInsensitive) == 0) {
		TriangleAreaLight* tal = new TriangleAreaLight(context);
		tal->readJSON(lightObject);
		addLight(tal);
	} else if (lightType.compare(QString("DiskAreaLight"), Qt::CaseInsensitive) == 0) {
		DiskAreaLight* dal = new DiskAreaLight(context);
		dal->readJSON(lightObject);
		addLight(dal);
	} else if (lightType.compare(QString("SphericalLight"), Qt::CaseInsensitive) == 0) {
		SphericalLight* sal = new SphericalLight(context);
		sal->readJSON(lightObject);
		addLight(sal);
	}

}

void OptixSceneLoader::initLightBuffers()
{
	light_buffer = context->createBuffer(RT_BUFFER_INPUT);
	light_buffer->setFormat(RT_FORMAT_USER);
	light_buffer->setElementSize(sizeof(LightStruct));
	light_buffer->setSize(0);
	triangle_light_buffer = context->createBuffer(RT_BUFFER_INPUT);
	triangle_light_buffer->setFormat(RT_FORMAT_USER);
	triangle_light_buffer->setElementSize(sizeof(TriangleLight));
	triangle_light_buffer->setSize(0);
	triangle_light_count = 0;
}

void OptixSceneLoader::loadTriangleLightBuffer()
{
	triangle_light_buffer->setFormat(RT_FORMAT_USER);
	triangle_light_buffer->setElementSize(sizeof(TriangleLight));
	triangle_light_buffer->setSize(triangle_light_count);
	TriangleLight* triangle_light_data = static_cast<TriangleLight*>(triangle_light_buffer->map());
	int idx_offset = 0;
	for (int idx = 0; idx < lights.size(); idx++)
	{
		LightStruct* light_struct = &lightStructData[idx];
		if (light_struct->light_type == TRIANGLES_AREA_LIGHT)
		{
			TrianglesAreaLightStruct* triangle_light_struct = reinterpret_cast<TrianglesAreaLightStruct*>(light_struct);
			TriangleAreaLight* tal = reinterpret_cast<TriangleAreaLight*>(lights[idx]);
			triangle_light_struct->buffer_start_idx = idx_offset;
			triangle_light_struct->buffer_end_idx = idx_offset + triangle_light_struct->triangle_count - 1;	
			unsigned int number_of_elements = static_cast<unsigned int>(triangle_light_struct->triangle_count) * sizeof(TriangleLight);
			memcpy(triangle_light_data + idx_offset, tal->getLightBuffer()->map(), number_of_elements );
			tal->getLightBuffer()->unmap();
			idx_offset += triangle_light_struct->triangle_count;
		}
	}
	context["triangle_light_buffer"]->set(triangle_light_buffer);
	triangle_light_buffer->unmap();
}


void OptixSceneLoader::resizeCamera(uint width, uint height) {

	camera->resizeCamera(width, height);

}


optix::float3 OptixSceneLoader::qVector3DtoFloat3(QVector3D vec) {

	return optix::make_float3(vec.x(), vec.y(), vec.z());

}

void OptixSceneLoader::updateLights(bool triangleAreaLightChanged) {


	if (triangleAreaLightChanged) {
		loadTriangleLightBuffer();
	}
	light_buffer->setSize(lights.size());
	memcpy(light_buffer->map(), lightStructData.data(), lightStructData.size() * sizeof(LightStruct));
	context["light_buffer"]->set(light_buffer);
	light_buffer->unmap();
}

void OptixSceneLoader::addLight(Light* light)
{
	LightStruct* lightStruct = new LightStruct();
	if(light->getType() == POINT_LIGHT) 
	{
		PointLight* pointLight = reinterpret_cast<PointLight*>(light);
		PointLightStruct* pointLightStruct = reinterpret_cast<PointLightStruct*>(lightStruct);
		pointLightStruct->light_type = pointLight->getType();
		pointLightStruct->emitted_radiance = qVector3DtoFloat3(pointLight->getRadiance());
		pointLightStruct->position = qVector3DtoFloat3(pointLight->getPosition());
	}
	else if(light->getType() == DIRECTIONAL_LIGHT) 
	{
		DirectionalLight* directionalLight = reinterpret_cast<DirectionalLight*>(light);
		DirectionalLightStruct* directionalLightStruct = reinterpret_cast<DirectionalLightStruct*>(lightStruct);
		directionalLightStruct->light_type = directionalLight->getType();
		directionalLightStruct->emitted_radiance = qVector3DtoFloat3(directionalLight->getRadiance());
		directionalLightStruct->direction = normalize(qVector3DtoFloat3(directionalLight->getDirection()));
	}
	else if(light->getType() == TRIANGLES_AREA_LIGHT) 
	{
		TriangleAreaLight* triangleAreaLight = reinterpret_cast<TriangleAreaLight*>(light);
		triangleAreaLight->loadLightGeometry();
		optix::Transform& transform = triangleAreaLight->getTransform();
		obj_group->addChild(transform);
		obj_group->getAcceleration()->markDirty();
		TrianglesAreaLightStruct* triangleAreaLightStruct = reinterpret_cast<TrianglesAreaLightStruct*>(lightStruct);
		triangleAreaLightStruct->light_type = triangleAreaLight->getType();
		triangleAreaLightStruct->emitted_radiance = qVector3DtoFloat3(triangleAreaLight->getRadiance());
		triangleAreaLightStruct->triangle_count = triangleAreaLight->getSize();
		triangle_light_count += triangleAreaLightStruct->triangle_count;
	}
	else if (light->getType() == DISK_LIGHT)
	{
		DiskAreaLight* diskLight = reinterpret_cast<DiskAreaLight*>(light);
		optix::GeometryGroup& gg = diskLight->getGeometryGroup();
		obj_group->addChild(gg);
		obj_group->getAcceleration()->markDirty();
		DiskLightStruct* diskLightStruct = reinterpret_cast<DiskLightStruct*>(lightStruct);
		diskLightStruct->light_type = diskLight->getType();
		diskLightStruct->emitted_radiance = qVector3DtoFloat3(diskLight->getRadiance());
		diskLightStruct->position = qVector3DtoFloat3(diskLight->getPosition());
		diskLightStruct->radius = diskLight->getRadius();
		diskLightStruct->phi = diskLight->getPhi();
		diskLightStruct->theta = diskLight->getTheta();
	}
	else if (light->getType() == SPHERICAL_LIGHT)
	{
		SphericalLight* sphereLight = reinterpret_cast<SphericalLight*>(light);
		optix::GeometryGroup& gg = sphereLight->getGeometryGroup();
		obj_group->addChild(gg);
		obj_group->getAcceleration()->markDirty();
		SphericalLightStruct* sphereLightStruct = reinterpret_cast<SphericalLightStruct*>(lightStruct);
		sphereLightStruct->light_type = sphereLight->getType();
		sphereLightStruct->emitted_radiance = qVector3DtoFloat3(sphereLight->getRadiance());
		sphereLightStruct->position = qVector3DtoFloat3(sphereLight->getPosition());
		sphereLightStruct->radius = sphereLight->getRadius();
	}
	lights.append(light);
	lightStructData.append(*lightStruct);
	delete lightStruct;
}

void OptixSceneLoader::removeLight(unsigned int lightIdx)
{
	LightStruct* lightStruct = &getLightStructs()->data()[lightIdx];
	Light* light = getLights()->data()[lightIdx];
	lights.remove(lightIdx);
	lightStructData.remove(lightIdx);

	if (light->getType() == TRIANGLES_AREA_LIGHT)
	{
		TriangleAreaLight* triangleLight = reinterpret_cast<TriangleAreaLight*>(light);
		TrianglesAreaLightStruct* triangleLightStruct = reinterpret_cast<TrianglesAreaLightStruct*>(lightStruct);
		triangle_light_count -= triangleLightStruct->triangle_count;
		obj_group->removeChild(triangleLight->getTransform());
		obj_group->getAcceleration()->markDirty();
		delete triangleLight, triangleLightStruct;
		updateLights(true);
	}
	else 
	{
		if (light->getType() == DISK_LIGHT) {
			DiskAreaLight* diskLight = reinterpret_cast<DiskAreaLight*>(light);
			obj_group->removeChild(diskLight->getGeometryGroup());
			obj_group->getAcceleration()->markDirty();
		} else 	if (light->getType() == SPHERICAL_LIGHT) {
			SphericalLight* sphereLight = reinterpret_cast<SphericalLight*>(light);
			obj_group->removeChild(sphereLight->getGeometryGroup());
			obj_group->getAcceleration()->markDirty();
		}
		delete light, lightStruct;
		updateLights(false);
	}

	//
}

void OptixSceneLoader::addGeometry(Geometry* geometry)
{
	
	if (geometry->getType() == OBJ)
	{
		OBJGeometry* obj = reinterpret_cast<OBJGeometry*>(geometry);
		optix::Transform& transform = obj->getTransform();
		obj_group->addChild(transform);
		updateAcceleration();
	}
	geometries.append(geometry);
	computeTranslucentGeometries();
}

void OptixSceneLoader::removeGeometry(unsigned int geometryIdx)
{

	Geometry* geometry = geometries.data()[geometryIdx];
	geometries.remove(geometryIdx);
	if (geometry->getType() == OBJ)
	{
		obj_group->removeChild(geometry->getTransform());
		updateAcceleration();
		
	}

	delete geometry;
	computeTranslucentGeometries();
}

void OptixSceneLoader::updateAcceleration()
{
	obj_group->getAcceleration()->markDirty();
}

void OptixSceneLoader::computeTranslucentGeometries()
{	
	translucentObjects.clear();
	for (int geometryIndex = 0; geometryIndex < geometries.size(); ++geometryIndex) {
		Geometry *geometry = geometries.at(geometryIndex);
		if (geometry->getMaterial()->getType() == TRANSLUCENT_SHADER || geometry->getMaterial()->getType() == ROUGH_TRANSLUCENT_SHADER)
		{			
			optix::GeometryGroup gg = geometry->getGeometryGroup();
			optix::Matrix4x4 transform_matrix;
			optix::Matrix4x4 inverse_transform_matrix;
			ScatteringMaterialProperties properties;
			for (unsigned int j = 0; j < gg->getChildCount(); ++j)
			{		
				optix::GeometryInstance& gi = gg->getChild(j);
				geometry->getTransform()->getMatrix(0, transform_matrix.getData(), inverse_transform_matrix.getData());
				gi["transform_matrix"]->setMatrix4x4fv(0,transform_matrix.getData());
				gi["inverse_transform_matrix"]->setMatrix4x4fv(0, inverse_transform_matrix.getData());
				geometry->getMaterial()->getOptixMaterial()["scattering_properties"]->getUserData(sizeof(ScatteringMaterialProperties), &properties);
				gi["scattering_properties"]->setUserData(sizeof(ScatteringMaterialProperties), &properties);
				gi["material_type"]->setUint(geometry->getMaterial()->getType());
				if(geometry->getMaterial()->getType() == ROUGH_TRANSLUCENT_SHADER)
				{
					gi["roughness"]->setFloat(geometry->getMaterial()->getOptixMaterial()["roughness"]->getFloat2());
					optix::float2 test = gi["roughness"]->getFloat2();
					gi["normal_distribution"]->setUint(geometry->getMaterial()->getOptixMaterial()["normal_distribution"]->getUint());
					gi["microfacet_model"]->setUint(geometry->getMaterial()->getOptixMaterial()["microfacet_model"]->getUint());
				}
				translucentObjects.append(gi);
				geometry->getMaterial()->getOptixMaterial()["translucent_index"]->setUint(translucentObjects.size()-1);
			}
		}
	}

	context["samples_output_buffer"]->getBuffer()->setSize(SAMPLES_FRAME*getTranslucentObjects().size());

	if (getTranslucentObjects().size() > 0)
	{	
		loadTranslucentGeometry(0);
	}
	else {
		initTranslucentContext();
	}
}

void OptixSceneLoader::loadTranslucentGeometry(uint idx)
{
		context["current_translucent_obj"]->setUint(idx);
		optix::GeometryInstance gi = getTranslucentObjects().at(idx);
		optix::Geometry& g = gi->getGeometry();
		context["vertex_buffer"]->setBuffer(g["vertex_buffer"]->getBuffer());
		context["normal_buffer"]->setBuffer(g["normal_buffer"]->getBuffer());
		context["vindex_buffer"]->setBuffer(g["vindex_buffer"]->getBuffer());
		context["nindex_buffer"]->setBuffer(g["nindex_buffer"]->getBuffer());
		optix::Matrix4x4 transform_matrix;
		optix::Matrix4x4 inverse_transform_matrix;
		ScatteringMaterialProperties properties;
		optix::float2 roughness;
		uint normal_distribution;
		uint microfacet_model;
		uint material_type;
		gi["transform_matrix"]->getMatrix4x4(0, transform_matrix.getData());
		gi["inverse_transform_matrix"]->getMatrix4x4(0, inverse_transform_matrix.getData());
		gi["scattering_properties"]->getUserData(sizeof(ScatteringMaterialProperties), &properties);
		material_type = gi["material_type"]->getUint();
		if (material_type == ROUGH_TRANSLUCENT_SHADER) {
			roughness = gi["roughness"]->getFloat2();
			normal_distribution = gi["normal_distribution"]->getUint();
			microfacet_model = gi["microfacet_model"]->getUint();
			context["current_roughness"]->setFloat(roughness);
			context["current_microfacet_model"]->setUint(microfacet_model);
			context["current_normal_distribution"]->setUint(normal_distribution);
		}

		
	
		context["transform_matrix"]->setMatrix4x4fv(0, transform_matrix.getData());
		context["normal_matrix"]->setMatrix4x4fv(1, inverse_transform_matrix.getData());
		context["current_scattering_properties"]->setUserData(sizeof(ScatteringMaterialProperties), &properties);

		context["current_material_type"]->setUint(material_type);
}

void OptixSceneLoader::initTranslucentContext()
{
	ScatteringMaterialProperties properties;
	optix::Buffer b = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, 0);
	context["current_translucent_obj"]->setUint(0);
	context["vertex_buffer"]->setBuffer( b);
	context["normal_buffer"]->setBuffer(b);
	context["vindex_buffer"]->setBuffer(b);
	context["nindex_buffer"]->setBuffer(b);
	context["transform_matrix"]->setMatrix4x4fv(0, optix::Matrix4x4::identity().getData());
	context["normal_matrix"]->setMatrix4x4fv(1, optix::Matrix4x4::identity().getData());
	context["current_scattering_properties"]->setUserData(sizeof(ScatteringMaterialProperties), &properties);
	context["current_material_type"]->setUint(TRANSLUCENT_SHADER);
	context["current_roughness"]->setFloat(optix::make_float2(0.0f));
	context["current_microfacet_model"]->setUint(WALTER_MODEL);
	context["current_normal_distribution"]->setUint(BECKMANN_DISTRIBUTION);
	
}