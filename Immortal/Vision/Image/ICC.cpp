#include "ICC.h"

namespace Immortal
{

static void LogCallback(cmsContext ctx, cmsUInt32Number error, const char *str)
{
	ICC *icc = (ICC *)cmsGetContextUserData(ctx);
	LOG_ERROR("lcms2: [{}] {}\n", error, str);
}

ICC::ICC() :
    context{}
{
	context = cmsCreateContext(nullptr, this);
	if (!context)
	{
		cmsSetLogErrorHandlerTHR(context, LogCallback);
	}
}

ICC::~ICC()
{
	if (context)
	{
		cmsDeleteContext(context);
	}

}

ICCProfile::ICCProfile(const std::string &path) :
    profile{ !path.empty() ? cmsOpenProfileFromFile(path.c_str(), "r") : nullptr},
    description{}
{
	if (profile)
	{
		char buf[1024] = {};
		cmsGetProfileInfoASCII(profile, cmsInfoDescription, "en", "US", buf, sizeof(buf));
		description = buf;
	}
}

ICCProfile::~ICCProfile()
{
	if (profile)
	{
		cmsCloseProfile(profile);
		profile = nullptr;
	}
}

std::vector<uint8_t> ICCProfile::SaveProfileToMemory()
{
	cmsUInt32Number size = 0;
	cmsSaveProfileToMem(profile, nullptr, &size);
	if (size == 0)
	{
		return {};
	}

	std::vector<uint8_t> data;
	data.resize(size);
	cmsSaveProfileToMem(profile, data.data(), &size);

	return data;
}

}
