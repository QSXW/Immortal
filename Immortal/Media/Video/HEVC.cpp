#include "HEVC.h"

namespace Immortal
{
namespace Vision
{

static inline void rbsp_trailing_bits(BitTracker &bitTracker)
{
    auto rbsp_stop_one_bit = bitTracker.GetBits(1);
    ThrowIf(
        rbsp_stop_one_bit != 1,
        "Incorrect rbsp_stop_one_bit"
    );
    while (!bitTracker.ByteAligned())
    {
        auto rbsp_alignment_zero_bit = bitTracker.GetBits(1);
        ThrowIf(
            rbsp_alignment_zero_bit != 0,
            "Incorrect rbsp_alignment_zero_bit"
        );
    }
}

static inline int32_t FilterByteStream(std::vector<uint8_t> &dst, const uint8_t *src, const uint8_t *end)
{
    constexpr int32_t emulation_prevention_three_byte = 0x3;
    auto ptr = src;

    uint32_t start_code_prefix_one_4bytes = *rcast<const uint32_t *>(ptr);
    if (start_code_prefix_one_4bytes == 0x01000000)
    {
        ptr += 4;
        while (*rcast<const uint32_t *>(ptr) != 0x01000000 && ptr < end)
        {
            auto byte = *ptr;
            if (byte != emulation_prevention_three_byte)
            {
                dst.emplace_back(byte);
            }
            ptr++;
        }
    }

    return ptr - src;
}

CodecError HEVCCodec::Decode(const std::vector<uint8_t> &rbsp)
{
    std::vector<uint8_t> buffer;
    int32_t bytesConsumed = 0;
    int32_t bytes;
    const uint8_t *end = rbsp.data() + rbsp.size();

    while ((bytes = FilterByteStream(buffer, rbsp.data() + bytesConsumed, end)) > 0)
    {
        bytesConsumed += bytes;
        Parse(buffer);
        buffer.clear();
    }

    return CodecError::Succeed;
}

#define CHECK_PROFILE_IDC(IDC) general.profile_idc == IDC || EXTRACT(general.profile_compatibility_flag, IDC)
ProfileTierLevel::ProfileTierLevel(BitTracker & bitTracker, uint32_t profilePresentFlag, uint32_t maxNumSubLayersMinus1)
{
    CleanUpObject(this);
    if (profilePresentFlag)
    {
        general.profile_space = bitTracker.GetBits(2);
        general.tier_flag     = bitTracker.GetBits(1);
        general.profile_idc   = bitTracker.GetBits(5);

        LOG::DEBUG("HEVC profiler: {0}", Stringify(Profiler(general.profile_idc)));

        general.profile_compatibility_flag = 0;
        for (int j = 0; j < 32; j++)
        {
            auto bit = bitTracker.GetBit();
            general.profile_compatibility_flag |= bit << (31 - j);
        }

        general.progressive_source_flag    = bitTracker.GetBit();
        general.interlaced_source_flag     = bitTracker.GetBit();
        general.non_packed_constraint_flag = bitTracker.GetBit();
        general.frame_only_constraint_flag = bitTracker.GetBit();

        if (CHECK_PROFILE_IDC(4) || CHECK_PROFILE_IDC(5) || CHECK_PROFILE_IDC(6)  || CHECK_PROFILE_IDC(7) ||
            CHECK_PROFILE_IDC(8) || CHECK_PROFILE_IDC(9) || CHECK_PROFILE_IDC(10) || CHECK_PROFILE_IDC(11))
        {
            general.max_12bit_constraint_flag        = bitTracker.GetBit();
            general.max_10bit_constraint_flag        = bitTracker.GetBit();
            general.max_8bit_constraint_flag         = bitTracker.GetBit();
            general.max_422chroma_constraint_flag    = bitTracker.GetBit();
            general.max_420chroma_constraint_flag    = bitTracker.GetBit();
            general.max_monochrome_constraint_flag   = bitTracker.GetBit();
            general.intra_constraint_flag            = bitTracker.GetBit();
            general.one_picture_only_constraint_flag = bitTracker.GetBit();
            general.lower_bit_rate_constraint_flag   = bitTracker.GetBit();

            if (CHECK_PROFILE_IDC(5)  || CHECK_PROFILE_IDC(9) || CHECK_PROFILE_IDC(10) || CHECK_PROFILE_IDC(11))
            {
                general.max_14bit_constraint_flag = bitTracker.GetBit();
                bitTracker.SkipBits(33);
            }
            else
            {
                bitTracker.SkipBits(34);
            }
        }
        else if (CHECK_PROFILE_IDC(2))
        {
            bitTracker.SkipBits(7);
            general.one_picture_only_constraint_flag = bitTracker.GetBit();
            bitTracker.SkipBits(35);
        }
        else
        {
            bitTracker.SkipBits(43);
        }
        if (CHECK_PROFILE_IDC(1) || CHECK_PROFILE_IDC(2) || CHECK_PROFILE_IDC(3 ) || CHECK_PROFILE_IDC(4 ) ||
            CHECK_PROFILE_IDC(5) || CHECK_PROFILE_IDC(9) || CHECK_PROFILE_IDC(11))
        {
            general.inbld_flag = bitTracker.GetBit();
        }
        else
        {
            bitTracker.SkipBits(1);
        }
    }

    general.level_idc = bitTracker.GetBits(8);
    for (int i = 0; i < maxNumSubLayersMinus1; i++)
    {

    }
    if (maxNumSubLayersMinus1 > 0)
    {
        for (int i = maxNumSubLayersMinus1; i < 8; i++)
        {

        }
    }
    for (int i = 0; i < maxNumSubLayersMinus1; i++)
    {

    }
}

VideoParameterSet::VideoParameterSet(BitTracker &bitTracker) :
    VideoParameterSet{}
{
    vps_video_parameter_set_id    = bitTracker.GetBits(4);
    vps_base_layer_internal_flag  = bitTracker.GetBits(1);
    vps_base_layer_available_flag = bitTracker.GetBits(1); 
    vps_max_layers_minus1         = bitTracker.GetBits(6);
    vps_max_sub_layers_minus1     = bitTracker.GetBits(3);
    vps_temporal_id_nesting_flag  = bitTracker.GetBits(1);
    vps_reserved_0xffff_16bits    = bitTracker.GetBits(16);

    profile_tier_level = ProfileTierLevel{ bitTracker, 1, vps_max_sub_layers_minus1 };

    vps_sub_layer_ordering_info_present_flag = bitTracker.GetBits(1);
    for (int i = (vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1); i <= vps_max_sub_layers_minus1; i++)
    {
        vps_max_dec_pic_buffering_minus1[i] = bitTracker.ue();
        vps_num_reorder_pics[i]             = bitTracker.ue();
        vps_max_latency_increase[i]         = bitTracker.ue();
        ThrowIf(
            vps_max_dec_pic_buffering_minus1[i] + 1 > H265::MAX_DPB_SIZE ||
            vps_num_reorder_pics[i] > vps_max_dec_pic_buffering_minus1[i],
            SError::OutOfBound
        );
    }
    vps_max_layer_id = bitTracker.GetBits(6);
    vps_num_layer_sets_minus1 = bitTracker.ue();

    for (int i = 1; i <= vps_num_layer_sets_minus1; i++)
    {
        for (int j = 0; j <= vps_max_layer_id; j++)
        {
            layer_id_included_flag[i][j] = bitTracker.GetBits(1);
        }
    }
    vps_timing_info_present_flag = bitTracker.GetBits(1);
    if (vps_timing_info_present_flag)
    {
        vps_num_units_in_tick = bitTracker.GetBits(32);
        vps_time_scale        = bitTracker.GetBits(32);
        vps_poc_proportional_to_timing_flag = bitTracker.GetBits(1);
        if (vps_poc_proportional_to_timing_flag)
        {
            vps_num_ticks_poc_diff_one_minus1 = bitTracker.ue();
        }
        vps_num_hrd_parameters = bitTracker.ue();
        for (int i = 0; i < vps_num_hrd_parameters; i++)
        {

        }
    }
    auto vps_extension_flag = bitTracker.GetBit();
    if (vps_extension_flag) { }
    rbsp_trailing_bits(bitTracker);
}

CodecError HEVCCodec::Parse(const std::vector<uint8_t>& buffer)
{
    BitTracker bitTracker{ buffer.data(), buffer.size() };

    NAL nal{ bitTracker };
    switch (nal.GetType())
    {
    case NAL::Type::VPP:
        vps = new VideoParameterSet{ bitTracker };
        break;

    case NAL::Type::SPS:
        break;

    case NAL::Type::PPS:
        break;

    default:
        break;
    }

    while (!bitTracker.NoMoreData() &&
            bitTracker.Preview(24) != 0x000001 &&
            bitTracker.Preview(32) != 0x00000001)
    {
        auto traillingZeroBits = bitTracker.GetBits(8);
    }

    return CodecError::Succeed;
}

}
}
