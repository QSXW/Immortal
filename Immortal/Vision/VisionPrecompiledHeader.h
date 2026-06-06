#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <list>

#include "Codec.h"
#include "CodedFrame.h"
#include "Demuxer.h"
#include "Image.h"
#include "Picture.h"
#include "Types.h"

#include "Audio/WAV.h"

#include "Common/Animator.h"
#include "Common/BitTracker.h"
#include "Common/Checksum.h"
#include "Common/Error.h"
#include "Common/NetworkAbstractionLayer.h"
#include "Common/SamplingFactor.h"

#include "Demux/FFDemuxer.h"
#include "Demux/IVFDemuxer.h"

#include "External/stb_image.h"

#include "Image/BMP.h"
#include "Image/Helper.h"
#include "Image/ImageCodec.h"
#include "Image/JPEG.h"
#include "Image/MFXJpegCodec.h"
#include "Image/OpenCVCodec.h"
#include "Image/PPM.h"
#include "Image/Raw.h"
#include "Image/STBCodec.h"

#include "Interface/MFXCodec.h"

#include "LookupTable/LookupTable.h"

#include "Processing/ColorSpace.h"

#include "Video/DAV1DCodec.h"
#include "Video/FFCodec.h"
#include "Video/HEVC.h"
#include "Video/Video.h"

#ifdef _WIN32
#include "Video/D3D12/HEVCCodec.h"
#endif

#include "Video/Vulkan/HEVC.h"
