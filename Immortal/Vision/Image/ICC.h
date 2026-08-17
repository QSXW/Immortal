#pragma once

#include <lcms2.h>

namespace Immortal
{

class ICC
{
public:
	ICC();

	~ICC();

public:
	operator bool() const
	{
		return !!context;
	}

private:
	cmsContext context;
};

class ICCProfile
{
public:
	SL_ENABLE_MOVE(ICCProfile)

public:
	ICCProfile(const std::string &path = {});

	~ICCProfile();

	std::vector<uint8_t> SaveProfileToMemory();

	void Swap(ICCProfile &other)
	{
		std::swap(profile,     other.profile);
		std::swap(description, other.description);
	}

public:
	operator bool() const
	{
		return !!profile;
	}

	const std::string &GetDescription() const
	{
		return description;
	}

private:
	cmsHPROFILE profile;

	std::string description;
};

}
