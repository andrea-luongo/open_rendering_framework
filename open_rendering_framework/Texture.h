#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QVector3D>
#include <QMatrix4x4>


using optix::TextureSampler;
using optix::Buffer;
using optix::Context;
using optix::float3;
using optix::make_float3;




TextureSampler loadConstantTexture(float3 color, Context context)
{
	TextureSampler sampler = context->createTextureSampler();

	sampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
	sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
	sampler->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);
	sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
	sampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
	sampler->setMaxAnisotropy(1.0f);
	sampler->setMipLevelCount(1u);
	sampler->setArraySize(1u);

	Buffer buffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_BYTE4, 1u, 1u);
	unsigned char* buffer_data = static_cast<unsigned char*>(buffer->map());
	buffer_data[0] = (unsigned char)optix::clamp((int)(color.x * 255.0f), 0, 255);
	buffer_data[1] = (unsigned char)optix::clamp((int)(color.y * 255.0f), 0, 255);
	buffer_data[2] = (unsigned char)optix::clamp((int)(color.z * 255.0f), 0, 255);
	buffer_data[3] = 255;
	buffer->unmap();
	sampler->setBuffer(0u, 0u, buffer);
	sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);

	return sampler;
};


TextureSampler loadPNGTexture(QString texture_path, Context context)
{
	QImage texture(texture_path);
	TextureSampler sampler = context->createTextureSampler();

	QImage::Format format = texture.format();
	
	sampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
	sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
	sampler->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);
	sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
	sampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
	sampler->setMaxAnisotropy(1.0f);
	sampler->setMipLevelCount(1u);
	sampler->setArraySize(1u);

	//if (texture.isNull())
	//	return sampler;

	optix::Buffer buffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_BYTE4, texture.width(), texture.height());
	unsigned char* buffer_data = static_cast<unsigned char*>(buffer->map());
	unsigned char* texture_data = static_cast<unsigned char*>(texture.bits());
	for (int j = 0; j < texture.height(); ++j) {
		for (int i = 0; i < texture.width(); ++i) {
			unsigned char blue = *texture_data++;
			unsigned char green = *texture_data++;
			unsigned char red = *texture_data++;
			unsigned char alpha =  *texture_data++;		
			*buffer_data++ = red; /* R */
			*buffer_data++ = green;                   /* G */
			*buffer_data++ = blue;                  /* B */
			*buffer_data++ = 255;
		}
	}
	buffer->unmap();
	sampler->setBuffer(0u, 0u, buffer);
	sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
	return sampler;

}


TextureSampler loadTexture(QString texture_path, Context context)
{
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	QFileInfo textureInfo(dir, texture_path);
	QString suffix = textureInfo.suffix();

	if (suffix.compare(QString("png"), Qt::CaseInsensitive) == 0)
	{
		return loadPNGTexture(texture_path, context);
	}
	else
	{
		return loadConstantTexture(make_float3(1.0f), context);
	}

};