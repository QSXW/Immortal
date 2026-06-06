#include "MFXCodec.h"

#if HAVE_MFX
namespace Immortal
{
namespace Vision
{

#define XX(x) case x: return #x;
static const char *GetAdapterType(mfxIMPL impl)
{
    switch (impl & 15)
    {
        XX(MFX_IMPL_UNSUPPORTED )
        XX(MFX_IMPL_SOFTWARE    )
        XX(MFX_IMPL_HARDWARE    )
        XX(MFX_IMPL_AUTO_ANY    )
        XX(MFX_IMPL_HARDWARE_ANY)
        XX(MFX_IMPL_HARDWARE2   )
        XX(MFX_IMPL_HARDWARE3   )
        XX(MFX_IMPL_HARDWARE4   )
        XX(MFX_IMPL_RUNTIME     )
#if (MFX_VERSION >= MFX_VERSION_NEXT)
        XX(MFX_IMPL_SINGLE_THREAD) = 0x0009,
#endif
        default: return "MFX_IMPL_UNKNOWN";
    }
}

#define MFX_IMPL_VIA_MASK(x) (0x0f00 & (x))
static const char *GetApiType(mfxIMPL impl)
{
    switch (MFX_IMPL_VIA_MASK(impl))
    {
        XX(MFX_IMPL_VIA_ANY  )
        XX(MFX_IMPL_VIA_D3D9 )
        XX(MFX_IMPL_VIA_D3D11)
        XX(MFX_IMPL_VIA_VAAPI)
        XX(MFX_IMPL_AUDIO    )
        default: return "MFX_IMPL_UNKNOWN";
    }
}
#undef XX

MFXCodec::MFXCodec() :
    surface{},
    videoParam{}
{
    mfxInitParam initParam{};

    initParam.Implementation = MFX_IMPL_AUTO_ANY;
    initParam.Version.Major = 1;
    initParam.Version.Minor = 0;

    session = new MFXVideoSession{};
    if (session->InitEx(initParam) != MFX_ERR_NONE)
    {
        LOG::ERR("Failed to init MFX context!");
        return;
    }

    if (session->QueryVersion(&version) != MFX_ERR_NONE)
    {
        LOG::WARN("Failed to query MFX version!");
    }
    else
    {
        LOG::INFO("Using MFX version: {}-{}", version.Major, version.Minor);
    }

    if (session->QueryIMPL(&impl) != MFX_ERR_NONE)
    {
        LOG::WARN("Failed to query MFX implementation!");
    }
    else
    {
        LOG::INFO("Using MFX implementation: {} - {}", GetAdapterType(impl), GetApiType(impl));
    }

    handle = new MFXVideoDECODE(*session);
}

MFXCodec::~MFXCodec()
{
    handle.Release();
    if (session)
    {
        session->Close();
    }
}

CodecError MFXCodec::Decode(const CodedFrame &codedFrame)
{
   
    return CodecError::Succeed;
}

mfxStatus MFXCodec::CheckAdapterSupported()
{
    mfxStatus status = MFX_ERR_NONE;
    mfxU32 numAdapters = 0;

    if ((status = MFXQueryAdaptersNumber(&numAdapters)) != MFX_ERR_NONE)
    {
        LOG::ERR("Failed to available query adapter");
        return status;
    }

    std::vector<mfxAdapterInfo> adapterInfo{ numAdapters };
    mfxAdaptersInfo adapters = { adapterInfo.data(), mfxU32(adapterInfo.size()), 0 };

    status = MFXQueryAdaptersDecode(&bitstream, videoParam.mfx.CodecId, &adapters);
    if (status == MFX_ERR_NOT_FOUND)
    {
        LOG::ERR("Failed to find an available adapter");
    }
    return status;
}

}
}

#endif
