#pragma once

#ifndef HEVC_H__
#define HEVC_H__

#include "Common/BitTracker.h"
#include "Common/NetworkAbstractionLayer.h"
#include "Media/Interface/Codec.h"

#ifdef _MSC_VER
#pragma warning(disable: 26495)
#endif

namespace Immortal
{
namespace Vision
{

enum H265
{
    MAX_DPB_SIZE = 16
};

enum Profiler
{
    Main             = 1,
    Main10           = 2,
    MainStillPicture = 3,
    RangeExtension   = 4
};

static const char *Stringify(Profiler profier)
{
#define XX(x) case Profiler::x: return #x;
    switch (profier)
    {
    XX(Main)
    XX(Main10)
    XX(MainStillPicture)
    XX(RangeExtension)
    default: return "Unknow";
    }
#undef XX
}

constexpr uint32_t MAX_SUB_LAYERS = 7;

#define EXTRACT(V, N) ((V >> (31 - N)) & 0x1)



struct ProfileTierLevel
{
    struct Description
    {
        uint32_t profile_space                    : 2;
        uint32_t tier_flag                        : 1;
        uint32_t profile_idc                      : 5;
        uint32_t profile_compatibility_flag       : 32;
        uint32_t progressive_source_flag          : 1;
        uint32_t interlaced_source_flag           : 1;
        uint32_t non_packed_constraint_flag       : 1;
        uint32_t frame_only_constraint_flag       : 1;
        uint32_t max_12bit_constraint_flag        : 1;
        uint32_t max_10bit_constraint_flag        : 1;
        uint32_t max_8bit_constraint_flag         : 1;
        uint32_t max_422chroma_constraint_flag    : 1;
        uint32_t max_420chroma_constraint_flag    : 1;
        uint32_t max_monochrome_constraint_flag   : 1;
        uint32_t intra_constraint_flag            : 1;
        uint32_t one_picture_only_constraint_flag : 1;
        uint32_t lower_bit_rate_constraint_flag   : 1;
        uint32_t max_14bit_constraint_flag        : 1;
        uint32_t inbld_flag                       : 1;
        uint32_t level_idc                        : 8;
    };

    ProfileTierLevel() = default;

    ProfileTierLevel(BitTracker &bitTracker, uint32_t profilePresentFlag, uint32_t maxNumSubLayersMinus1);

    Description general;
    Description sub_layer[8];
};

struct VideoParameterSet : public NAL::Payload
{
    VideoParameterSet()
    {
        CleanUpObject(this);
    }

    VideoParameterSet(BitTracker &bitTracker);

    ProfileTierLevel profile_tier_level;
    uint32_t vps_video_parameter_set_id    : 4;
    uint32_t vps_base_layer_internal_flag  : 1;
    uint32_t vps_base_layer_available_flag : 1;
    uint32_t vps_max_layers_minus1         : 6;
    uint32_t vps_max_sub_layers_minus1     : 3;
    uint32_t vps_temporal_id_nesting_flag  : 1;
    uint32_t vps_reserved_0xffff_16bits    : 16;
    uint32_t vps_sub_layer_ordering_info_present_flag : 1;
    uint32_t vps_max_layer_id              : 6;
    uint32_t vps_timing_info_present_flag  : 1;
    uint32_t vps_poc_proportional_to_timing_flag : 1;

    uint32_t vps_num_layer_sets_minus1;
    uint32_t vps_num_units_in_tick;
    uint32_t vps_time_scale;
    uint32_t vps_num_ticks_poc_diff_one_minus1;
    uint32_t vps_num_hrd_parameters;
    uint8_t  vps_max_dec_pic_buffering_minus1[8];
    uint8_t  vps_num_reorder_pics[8];
    uint8_t  vps_max_latency_increase[8];
    uint8_t  layer_id_included_flag[8][8];
};

struct NetworkAbstractionLayer : SuperNetworkAbstractionLayer
{
    using Super = SuperNetworkAbstractionLayer;

    struct Header : public Super::Header
    {
        uint32_t forbiddenZeroBit: 1;
        uint32_t type : 6;
        struct
        {
            uint32_t layer    : 6;
            uint32_t temporal : 3;
        } id;
    };

    NetworkAbstractionLayer(BitTracker &bitTracker)
    {
        header.forbiddenZeroBit = bitTracker.GetBit();
        ThrowIf(
            header.forbiddenZeroBit,
            "Corrupted NAL header"
        );

        header.type        = bitTracker.GetBits(6);
        header.id.layer    = bitTracker.GetBits(6);
        header.id.temporal = bitTracker.GetBits(3) - 1;
    }

    virtual int32_t GetType() const
    {
        return header.type;
    }

    ~NetworkAbstractionLayer()
    {

    }

    Header header;
};

using NAL = NetworkAbstractionLayer;

class HEVCCodec : public Interface::Codec
{
public:
    enum
    {
        MAX_SUB_LAYERS = 7
    };

public:
    virtual CodecError Decode(const std::vector<uint8_t> &rbsp) override;

    virtual CodecError Parse(const std::vector<uint8_t> &buffer) override;

    VideoParameterSet *vps;
};

}
}

#endif // !HEVC_H__
