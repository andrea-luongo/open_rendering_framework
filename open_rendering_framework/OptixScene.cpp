#include "OptixScene.h"
#include <iostream>
#include <fstream>
#include <climits>
#include "sampleConfig.h"

OptixScene::OptixScene(GLuint w, GLuint h)
{
	WIDTH = w;
	HEIGHT = h;
	//SAMPLES_FRAME = 500;
	frame = 0;
	max_frame = -1;
	quit_and_save = false;
}

OptixScene::~OptixScene()
{
	destroyContext();
	delete sceneLoader;
}

void OptixScene::resizeScene(GLuint w, GLuint h)
{
	if (w == WIDTH && h == HEIGHT) return;

	WIDTH = w;
	HEIGHT = h;
	frame = 0;
	
	optix::Buffer buffer = getOutputBuffer();
	buffer->setSize(WIDTH, HEIGHT);
	optix::Buffer position_buffer = getPositionBuffer();
	position_buffer->setSize(WIDTH, HEIGHT);
	optix::Buffer normal_buffer = getNormalBuffer();
	normal_buffer->setSize(WIDTH, HEIGHT);

	sceneLoader->resizeCamera(WIDTH, HEIGHT);
}


optix::Buffer OptixScene::getOutputBuffer()
{
	return optix_context["output_buffer"]->getBuffer();
}

optix::Buffer OptixScene::getPositionBuffer()
{
	return optix_context["positions_buffer"]->getBuffer();
}


optix::Buffer OptixScene::getNormalBuffer()
{
	return optix_context["normals_buffer"]->getBuffer();
}


std::string OptixScene::ptxPath(const std::string& target, const std::string& base)
{
	return
		std::string(SAMPLES_PTX_DIR) + "/" + target + "_generated_" + base + ".ptx";
}



//
void OptixScene::destroyContext()
{
	if (optix_context)
	{
		optix_context->destroy();
		optix_context = 0;
	}
}
//
void OptixScene::initContext(GLuint b_id, uint& width, uint& height)
{
	QString path = SAMPLES_DIR + QString("/scenes/jsonscene.json");

	for (int idx = 0; idx < QCoreApplication::arguments().count(); idx++)
	{
		if (QFileInfo(QCoreApplication::arguments().at(idx)).suffix().compare(QString("json")) == 0) 
		{
			path = QCoreApplication::arguments().at(idx);
		}
		if (QCoreApplication::arguments().at(idx).compare(QString("-q")) == 0 || QCoreApplication::arguments().at(idx).compare(QString("--quit")) == 0)
		{
			quit_and_save = true;
		}
	}
	
	initContext(b_id, path, width, height);
}

void OptixScene::initContext(GLuint b_id, QString path, uint& width, uint& height)
{
	scene_path = path;
	optix_context = optix::Context::create();
	
	sceneLoader = new OptixSceneLoader(optix_context);
	buffer_id = b_id;
	optix_context->setRayTypeCount(NUMBER_OF_RAYS);
	optix_context->setEntryPointCount(NUMBER_OR_ENTRIES);
	optix_context->setStackSize(30000);
	
	//optix_context["radiance_ray_type"]->setUint(0);
	//optix_context["shadow_ray_type"]->setUint(1);

	loadScene(WIDTH, HEIGHT);
	height = HEIGHT;
	width = WIDTH;
	optix_context->setPrintEnabled(true);
	optix_context->setPrintBufferSize(1024);
	optix_context->setPrintLaunchIndex(WIDTH / 2, HEIGHT / 2);
	//optix_context->setPrintLaunchIndex(sceneLoader->getSamplesFrame() / 2);
}

void OptixScene::unregisterOutputBuffer() 
{
	optix::Buffer buffer = getOutputBuffer();
	const GLuint pboId = buffer->getGLBOId();
	if (pboId)
	{
		buffer->unregisterGLBuffer();
	}
}

void OptixScene::registerOutputBuffer() 
{
	optix::Buffer buffer = getOutputBuffer();
	const GLuint pboId = buffer->getGLBOId();
	if (pboId)
	{
		buffer->registerGLBuffer();
	}
}

void OptixScene::renderScene()
{
	// Launch the ray tracer
	if (max_frame < 0 || frame < max_frame )
	{
		optix_context["frame"]->setUint(frame++);

		// Generate surface position samples
		if (sceneLoader->getTranslucentObjects().size() == 1)
			optix_context->launch(sample_camera_pass, sceneLoader->getSamplesFrame());
		else if (sceneLoader->getTranslucentObjects().size() > 1)
		{
			
			for (unsigned int i = 0; i < sceneLoader->getTranslucentObjects().size(); ++i)
			{
				sceneLoader->loadTranslucentGeometry(i);
				optix_context->launch(sample_camera_pass, sceneLoader->getSamplesFrame());
			}
		}

		optix_context->launch(integrator_pass, WIDTH, HEIGHT);
	}
	if (frame == max_frame && quit_and_save)
	{
		QFileInfo fileInfo = scene_path;
		QString pathWithoutExtension = fileInfo.path() + "/" + fileInfo.baseName();
		saveScene(scene_path);
		//saveScreenshot(pathWithoutExtension + ".png", false);
	}
}


void OptixScene::drawOutputBuffer() 
{
	optix::Buffer buffer = getOutputBuffer();
	GLvoid* imageData = buffer->map(0, RT_BUFFER_MAP_READ);
	GLenum gl_data_type = GL_FALSE;
	GLenum gl_format = GL_FALSE;

	gl_data_type = GL_FLOAT;
	gl_format = GL_RGBA;

	RTsize elmt_size = buffer->getElementSize();
	int align = 1;
	if ((elmt_size % 8) == 0) align = 8;
	else if ((elmt_size % 4) == 0) align = 4;
	else if ((elmt_size % 2) == 0) align = 2;
	glPixelStorei(GL_UNPACK_ALIGNMENT, align);

	glDrawPixels(
	static_cast<GLsizei>(WIDTH),
	static_cast<GLsizei>(HEIGHT),
	gl_format,
	gl_data_type,
	imageData
	);
	buffer->unmap();

}


void OptixScene::saveScene(QString filename)
{
	//TODO write json scen
	QFileInfo fileInfo = filename;
	QString pathWithoutExtension = fileInfo.path() + "/" + fileInfo.baseName();
	sceneLoader->saveJSONScene(filename, frame);
	//Write buffer to raw file
	optix::Buffer buffer = getOutputBuffer();
	int size_buffer = WIDTH*HEIGHT * 4;
	float* mapped = new float[size_buffer];
	memcpy(mapped, buffer->map(0, RT_BUFFER_MAP_READ), size_buffer * sizeof(float));
	buffer->unmap();
	//Convert buffer to float3
	int size_image = WIDTH*HEIGHT * 4;
	//float* converted = new float[size_image];
	//for (int i = 0; i < size_image / 3; ++i)
	//{
	//	for (int j = 0; j < 3; ++j)
	//	{
	//		float value = mapped[i * 4 + j];
	//		converted[i * 3 + j] = value;
	//	}
	//}
	//Write buffer to file
	uchar* image_data = bufferToImage(RT_FORMAT_FLOAT4, WIDTH, HEIGHT, reinterpret_cast<void*>(mapped));
	QImage image(image_data, WIDTH, HEIGHT, QImage::Format_RGBA8888);
	image.save(pathWithoutExtension + ".png", "PNG");

	QFile file(pathWithoutExtension + ".raw");
	unsigned long datasize;
	if (file.open(QIODevice::WriteOnly)) {
		qint64 bytesWritten = file.write(reinterpret_cast<const char*>(mapped), size_image * sizeof(float));
		file.close();
	}

	delete[] mapped,  image_data;
	
}


void OptixScene::loadScene(uint& width, uint& height)
{
	//TODO load json scene
	restartFrame();
	buffer_path;
	sceneLoader->loadJSONScene(scene_path, WIDTH, HEIGHT, buffer_path, frame);
	height = HEIGHT;
	width = WIDTH;
	

}

void OptixScene::loadBuffer()
{
	optix::Buffer buffer;
	if (buffer_id) {
		buffer = optix_context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, buffer_id);
		buffer->setFormat(RT_FORMAT_FLOAT4);
		buffer->setSize(WIDTH, HEIGHT);
	}
	else {
		buffer = optix_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, WIDTH, HEIGHT);
	}
	optix_context["output_buffer"]->set(buffer);

	optix::Buffer positions_buffer;
	optix::Buffer normals_buffer;

	QFile file(buffer_path);
	int size_image = WIDTH*HEIGHT * 4;
	float* image = new float[size_image];
	if (!quit_and_save && file.open(QIODevice::ReadOnly)) {
		file.read(reinterpret_cast<char*>(image), size_image * sizeof(float));
		setMaxFrame(frame);
		file.close();
	}
	else {
		if (frame > 0) {
			setMaxFrame(frame);
			frame = 0;
		}
		return;
	}
	//convert data to float4
	int size_buffer = WIDTH * HEIGHT * 4;
	float* converted = new float[size_buffer];
	for (int i = 0; i < size_buffer / 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			converted[i * 4 + j] = image[i * 4 + j];
		//converted[i * 4 + 3] = 1.0f;
	}
	//write to buffer
	optix::Buffer out = getOutputBuffer();
	memcpy(out->map(), converted, size_buffer * sizeof(float));
	out->unmap();
	delete[] converted;
	delete[] image;
}


void OptixScene::saveScreenshot(QString filename, bool add_frames)
{

	//TODO write json scen
	QFileInfo fileInfo = filename;
	QString pathWithoutExtension = fileInfo.path() + "/" + fileInfo.baseName();
	QString extension = fileInfo.suffix();
	//Write buffer to png file
	optix::Buffer buffer = getOutputBuffer();
	int size_buffer = WIDTH*HEIGHT * 4;
	float* mapped = new float[size_buffer];
	memcpy(mapped, buffer->map(0, RT_BUFFER_MAP_READ), size_buffer * sizeof(float));
	buffer->unmap();
	
	//Write buffer to file
	uchar* image_data = bufferToImage(RT_FORMAT_FLOAT4, WIDTH, HEIGHT, reinterpret_cast<void*>(mapped));
	QImage image(image_data, WIDTH, HEIGHT, QImage::Format_RGBA8888);
	if(add_frames)
		image.save(pathWithoutExtension + "_" + QString::number(frame) + "." + extension, extension.toStdString().c_str());
	else
		image.save(fileInfo.absoluteFilePath(), extension.toStdString().c_str());
	
	delete[] mapped, image_data;

}

