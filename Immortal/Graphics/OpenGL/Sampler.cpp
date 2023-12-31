#include "Sampler.h"

namespace Immortal
{
namespace OpenGL
{

Sampler::Sampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod) :
    Handle{}
{
	glGenSamplers(1, &handle);

	GLuint addressModeParam = GL_SAMPLER_REPEAT;
	if (addressMode == AddressMode::Clamp)
	{
		addressModeParam = GL_SAMPLER_CLAMP_TO_EDGE;
	}
	if (addressMode == AddressMode::BorderColor)
	{
		addressModeParam = GL_SAMPLER_CLAMP_TO_BORDER;
	}

	GLuint filterParam = GL_FILTER_NEAREST;
	if (filter == Filter::Linear || filter == Filter::Bilinear)
	{
		filterParam = GL_FILTER_LINEAR;
	}

	glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, filterParam);
	glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, filterParam);

	glSamplerParameteri(handle, GL_TEXTURE_WRAP_S, addressModeParam);
	glSamplerParameteri(handle, GL_TEXTURE_WRAP_T, addressModeParam);
	glSamplerParameteri(handle, GL_TEXTURE_WRAP_R, addressModeParam);
}

Sampler::~Sampler()
{
	glDeleteSamplers(1, &handle);
}

}
}
