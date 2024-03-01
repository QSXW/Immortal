#include "HEVC.h"
#include "Shared/Log.h"

namespace Immortal
{
namespace Vision
{

static inline void rbsp_trailing_bits(BitTracker &bitTracker)
{
    auto rbsp_stop_one_bit = bitTracker.GetBit();
    ThrowIf(
        rbsp_stop_one_bit != 1,
        "Incorrect rbsp_stop_one_bit"
    );
    while (!bitTracker.ByteAligned())
    {
        auto rbsp_alignment_zero_bit = bitTracker.GetBit();
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

    uint32_t start_code_prefix_one_4bytes = *(const uint32_t *)(ptr);
    if (start_code_prefix_one_4bytes == 0x01000000)
    {
        ptr += 4;
        for ( ; *(const uint32_t *)(ptr) != 0x01000000 && ptr < end; ptr++)
        {
            auto byte = *ptr;
            if (byte == emulation_prevention_three_byte && !ptr[-2] && !ptr[-1])
            {
                continue;
            }
            dst.emplace_back(byte);
        }
    }

    return ptr - src;
}

static inline Format SelectFormat(int32_t bitDepth, int32_t chromaFormatIDC)
{
    switch (bitDepth)
    {
    case 8:
        if (chromaFormatIDC == 0)
        {
            return Format::None;
        }
        if (chromaFormatIDC == 1)
        {
            return  Format::YUV420P;
        }
        if (chromaFormatIDC == 2)
        {
            return Format::YUV422P;
        }
        if (chromaFormatIDC == 2)
        {
            return Format::YUV444P;
        }

    default:
        return Format::None;
    }
}

CodecError HEVCCodec::Decode(const CodedFrame &codedFrame)
{
	const auto &rbsp = codedFrame.GetBuffer();

    std::vector<uint8_t> buffer;
    int32_t bytesConsumed = 0;
    int32_t bytes;
    const uint8_t *end = rbsp.data() + rbsp.size();

    while ((bytes = FilterByteStream(buffer, rbsp.data() + bytesConsumed, end)) > 0)
    {
        bytesConsumed += bytes;
        if (Parse(buffer) != CodecError::Preparing)
        {
            break;
        }
        buffer.clear();
    }

    return CodecError::Success;
}

#define EXTRACT(V, N) ((V >> (31 - N)) & 0x1)
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

void VideoParameterSet::Parse(BitTracker &bitTracker)
{
    LOG::DEBUG("HEVC: Paring Video Parameter Set");

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
        vps_max_dec_pic_buffering[i] = bitTracker.UnsignedExpGolomb() + 1;
        vps_num_reorder_pics[i]      = bitTracker.UnsignedExpGolomb();
        vps_max_latency_increase[i]  = bitTracker.UnsignedExpGolomb();
        ThrowIf(
            vps_max_dec_pic_buffering[i] + 1 > H265::MAX_DPB_SIZE ||
            vps_num_reorder_pics[i] > vps_max_dec_pic_buffering[i],
            SError::OutOfBound
        );
    }
    vps_max_layer_id = bitTracker.GetBits(6);
    vps_num_layer_sets_minus1 = bitTracker.UnsignedExpGolomb();

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
            vps_num_ticks_poc_diff_one_minus1 = bitTracker.UnsignedExpGolomb();
        }
        vps_num_hrd_parameters = bitTracker.UnsignedExpGolomb();
        for (int i = 0; i < vps_num_hrd_parameters; i++)
        {

        }
    }
    auto vps_extension_flag = bitTracker.GetBit();
    if (vps_extension_flag) { }
    rbsp_trailing_bits(bitTracker);
}


VideoParameterSet::VideoParameterSet(BitTracker &bitTracker)
{
    CleanUpObject(this);
    Parse(bitTracker);
}

void ScalingListData::Parse(BitTracker &bitTracker)
{
    int nextCoef;

    for (int sizeId = 0; sizeId < 4; sizeId++)
    {
        for (int matrixId = 0; matrixId < 6; matrixId += (sizeId == 3) ? 3 : 1)
        {
            auto scaling_list_pred_mode_flag = bitTracker.GetBit();
            if (scaling_list_pred_mode_flag)
            {
                auto scaling_list_pred_matrix_id_delta = bitTracker.UnsignedExpGolomb();
            }
            else
            {
                nextCoef = 8;
                auto coefNum = std::min(64, (1 << (4 + (sizeId << 1))));
                if (sizeId > 1)
                {
                    scaling_list_dc_coef_minus8[sizeId - 2][matrixId] = bitTracker.SignedExpGolomb();
                    nextCoef = scaling_list_dc_coef_minus8[sizeId - 2][matrixId] + 8;
                }
                for (int i = 0; i < coefNum; i++)
                {
                    scaling_list_delta_coef = bitTracker.SignedExpGolomb();
                    nextCoef = (nextCoef + scaling_list_delta_coef + 256) % 256;
                    scaling_list[sizeId][matrixId][i] = nextCoef;
                }
            }
        }
    }
}

ScalingListData::ScalingListData(BitTracker &bitTracker)
{
    CleanUpObject(this);
    Parse(bitTracker);
}

void SequenceParameterSet::DecodeVuiParameters(BitTracker & bitTracker)
{
    aspect_ratio_info_present_flag = bitTracker.GetBit();
    if (aspect_ratio_info_present_flag)
    {
        aspect_ratio_idc = bitTracker.GetBits(8);
        if (aspect_ratio_idc == H265::EXTENDED_SAR)
        {
            sar = Rational{ bitTracker.GetBits(16), bitTracker.GetBits(16) };
        }
    }

    overscan_info_present_flag = bitTracker.GetBit();
    if (overscan_info_present_flag)
    {
        overscan_appropriate_flag = bitTracker.GetBit();
    }

    video_signal_type_present_flag = bitTracker.GetBit();
    if (video_signal_type_present_flag)
    {
        video_format                    = bitTracker.GetBits(3);
        video_full_range_flag           = bitTracker.GetBit();
        colour_description_present_flag = bitTracker.GetBit();
        if (colour_description_present_flag)
        {
            colour_primaries         = bitTracker.GetBits(8);
            transfer_characteristics = bitTracker.GetBits(8);
            matrix_coeffs = bitTracker.GetBits(8);
        }
    }

    chroma_loc_info_present_flag = bitTracker.GetBit();
    if (chroma_loc_info_present_flag)
    {
        chroma_sample_loc_type_top_field    = bitTracker.UnsignedExpGolomb();
        chroma_sample_loc_type_bottom_field = bitTracker.UnsignedExpGolomb();
    }

    neutral_chroma_indication_flag = bitTracker.GetBit();
    field_seq_flag                 = bitTracker.GetBit();
    frame_field_info_present_flag  = bitTracker.GetBit();
    default_display_window_flag    = bitTracker.GetBit();
    if (default_display_window_flag)
    {
        def_disp_win_left_offset   = bitTracker.UnsignedExpGolomb();
        def_disp_win_right_offset  = bitTracker.UnsignedExpGolomb();
        def_disp_win_top_offset    = bitTracker.UnsignedExpGolomb();
        def_disp_win_bottom_offset = bitTracker.UnsignedExpGolomb();
    }

    vui_timing_info_present_flag = bitTracker.GetBit();
    if (vui_timing_info_present_flag)
    {
        vui_num_units_in_tick               = bitTracker.GetBits(32);
        vui_time_scale                      = bitTracker.GetBits(32);
        vui_poc_proportional_to_timing_flag = bitTracker.GetBit();
        if (vui_poc_proportional_to_timing_flag)
        {
            vui_num_ticks_poc_diff_one_minus1 = bitTracker.UnsignedExpGolomb();
        }
        vui_hrd_parameters_present_flag = bitTracker.GetBit();
        if (vui_hrd_parameters_present_flag)
        {
            throw StaticException("Unsupport yet!");
        }
    }

    bitstream_restriction_flag = bitTracker.GetBit();
    if (bitstream_restriction_flag)
    {
        tiles_fixed_structure_flag              = bitTracker.GetBit();
        motion_vectors_over_pic_boundaries_flag = bitTracker.GetBit();
        restricted_ref_pic_lists_flag           = bitTracker.GetBit();
        min_spatial_segmentation_idc            = bitTracker.UnsignedExpGolomb();
        max_bytes_per_pic_denom                 = bitTracker.UnsignedExpGolomb();
        max_bits_per_min_cu_denom               = bitTracker.UnsignedExpGolomb();
        log2_max_mv_length_horizontal           = bitTracker.UnsignedExpGolomb();
        log2_max_mv_length_vertical             = bitTracker.UnsignedExpGolomb();
    }
}

void SequenceParameterSet::Parse(BitTracker & bitTracker)
{
    LOG::DEBUG("HEVC: Paring Sequence Parameter Set");
    sps_video_parameter_set_id   = bitTracker.GetBits(4);
    sps_max_sub_layers_minus1    = bitTracker.GetBits(3);
    sps_temporal_id_nesting_flag = bitTracker.GetBit();

    profile_tier_level = ProfileTierLevel{ bitTracker, 1, sps_max_sub_layers_minus1 };

    sps_seq_parameter_set_id = bitTracker.UnsignedExpGolomb();
    chroma_format_idc = bitTracker.UnsignedExpGolomb();
    if (chroma_format_idc == 3)
    {
        separate_colour_plane_flag = bitTracker.GetBit();
    }
    pic_width_in_luma_samples  = bitTracker.UnsignedExpGolomb();
    pic_height_in_luma_samples = bitTracker.UnsignedExpGolomb();

    if (/* conformance_window_flag = */ bitTracker.GetBit())
    {
        conf_win_left_offset   = bitTracker.UnsignedExpGolomb();
        conf_win_right_offset  = bitTracker.UnsignedExpGolomb();
        conf_win_top_offset    = bitTracker.UnsignedExpGolomb();
        conf_win_bottom_offset = bitTracker.UnsignedExpGolomb();
    }

    bit_depth_luma   = bitTracker.UnsignedExpGolomb() + 8;
    bit_depth_chroma = bitTracker.UnsignedExpGolomb() + 8;
    log2_max_pic_order_cnt_lsb_minus4 = bitTracker.UnsignedExpGolomb();
    sps_sub_layer_ordering_info_present_flag = bitTracker.GetBit();

    for (int i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1);
            i <= sps_max_sub_layers_minus1; i++)
    {
        subLayers[i].sps_max_dec_pic_buffering_minus1 = bitTracker.UnsignedExpGolomb();
        subLayers[i].sps_max_num_reorder_pics         = bitTracker.UnsignedExpGolomb();
        subLayers[i].sps_max_latency_increase_plus1   = bitTracker.UnsignedExpGolomb();
    }

    log2_min_luma_coding_block_size_minus3      = bitTracker.UnsignedExpGolomb();
    log2_diff_max_min_luma_coding_block_size    = bitTracker.UnsignedExpGolomb();
    log2_min_luma_transform_block_size_minus2   = bitTracker.UnsignedExpGolomb();
    log2_diff_max_min_luma_transform_block_size = bitTracker.UnsignedExpGolomb();
    max_transform_hierarchy_depth_inter         = bitTracker.UnsignedExpGolomb();
    max_transform_hierarchy_depth_intra         = bitTracker.UnsignedExpGolomb();

    scaling_list_enabled_flag = bitTracker.GetBit();
    if (scaling_list_enabled_flag)
    {
        sps_scaling_list_data_present_flag = bitTracker.GetBit();
        if (sps_scaling_list_data_present_flag)
        {
            scalingListData.Parse(bitTracker);
        }
    }

    amp_enabled_flag                    = bitTracker.GetBit();
    sample_adaptive_offset_enabled_flag = bitTracker.GetBit();
    pcm_enabled_flag                    = bitTracker.GetBit();
    if (pcm_enabled_flag)
    {
        pcm_sample_bit_depth_luma_minus1   = bitTracker.GetBits(4);
        pcm_sample_bit_depth_chroma_minus1 = bitTracker.GetBits(4);
        log2_min_pcm_luma_coding_block_size_minus3   = bitTracker.UnsignedExpGolomb();
        log2_diff_max_min_pcm_luma_coding_block_size = bitTracker.UnsignedExpGolomb();
        pcm_loop_filter_disabled_flag = bitTracker.GetBit();
    }

    num_short_term_ref_pic_sets = bitTracker.UnsignedExpGolomb();
    for (int i = 0; i < num_short_term_ref_pic_sets; i++)
    {
        // st_ref_pic_set(i);
    }
    long_term_ref_pics_present_flag = bitTracker.GetBit();
    if (long_term_ref_pics_present_flag)
    {
        num_long_term_ref_pics_sps = bitTracker.UnsignedExpGolomb();
        for (int i = 0; i < num_long_term_ref_pics_sps; i++)
        {
            lt_ref_pic_poc_lsb_sps[i] = bitTracker.GetBits(log2_max_pic_order_cnt_lsb_minus4 + 4);
            used_by_curr_pic_lt_sps_flag[i] = bitTracker.GetBit();
        }
    }

    sps_temporal_mvp_enabled_flag       = bitTracker.GetBit();
    strong_intra_smoothing_enabled_flag = bitTracker.GetBit();
    vui_parameters_present_flag         = bitTracker.GetBit();
    if (vui_parameters_present_flag)
    {
        DecodeVuiParameters(bitTracker);
    }

    sps_extension_present_flag = bitTracker.GetBit();
    if (sps_extension_present_flag)
    {
        sps_range_extension_flag      = bitTracker.GetBit();
        sps_multilayer_extension_flag = bitTracker.GetBit();
        sps_3d_extension_flag         = bitTracker.GetBit();
        sps_scc_extension_flag        = bitTracker.GetBit();
        sps_extension_4bits           = bitTracker.GetBits(4);
    }

    rbsp_trailing_bits(bitTracker);
}

SequenceParameterSet::SequenceParameterSet(BitTracker &bitTracker)
{
    CleanUpObject(this);
    Parse(bitTracker);
}

void PictureParameterSet::Parse(BitTracker &bitTracker)
{
    LOG::DEBUG("HEVC: Paring Picture Parameter Set");
    pps_pic_parameter_set_id = bitTracker.UnsignedExpGolomb();
    pps_seq_parameter_set_id = bitTracker.UnsignedExpGolomb();

    dependent_slice_segments_enabled_flag = bitTracker.GetBit();
    output_flag_present_flag              = bitTracker.GetBit();
    num_extra_slice_header_bits           = bitTracker.GetBits(3);
    sign_data_hiding_enabled_flag         = bitTracker.GetBit();
    cabac_init_present_flag               = bitTracker.GetBit();

    num_ref_idx_l0_default_active_minus1 = bitTracker.UnsignedExpGolomb();
    num_ref_idx_l1_default_active_minus1 = bitTracker.UnsignedExpGolomb();

    init_qp_minus26 = bitTracker.SignedExpGolomb();

    constrained_intra_pred_flag = bitTracker.GetBit();
    transform_skip_enabled_flag = bitTracker.GetBit();

    cu_qp_delta_enabled_flag    = bitTracker.GetBit();
    if (cu_qp_delta_enabled_flag)
    {
        diff_cu_qp_delta_depth = bitTracker.UnsignedExpGolomb();
    }

    pps_cb_qp_offset = bitTracker.SignedExpGolomb();
    pps_cr_qp_offset = bitTracker.SignedExpGolomb();

    pps_slice_chroma_qp_offsets_present_flag = bitTracker.GetBit();
    weighted_pred_flag                       = bitTracker.GetBit();
    weighted_bipred_flag                     = bitTracker.GetBit();
    transquant_bypass_enabled_flag           = bitTracker.GetBit();
    tiles_enabled_flag                       = bitTracker.GetBit();
    entropy_coding_sync_enabled_flag         = bitTracker.GetBit();

    if (tiles_enabled_flag)
    {
        num_tile_columns_minus1 = bitTracker.UnsignedExpGolomb() + 1;
        num_tile_rows_minus1 = bitTracker.UnsignedExpGolomb() + 1;
        uniform_spacing_flag    = bitTracker.GetBit();
        if (uniform_spacing_flag)
        {
            for (size_t i = 0; i < num_tile_columns_minus1; i++)
            {
                column_width_minus1[i] = bitTracker.UnsignedExpGolomb();
            }
            for (size_t i = 0; i < num_tile_rows_minus1; i++)
            {
                row_height_minus1[i] = bitTracker.UnsignedExpGolomb();
            }
        }
        loop_filter_across_tiles_enabled_flag = bitTracker.GetBit();
    }

    pps_loop_filter_across_slices_enabled_flag = bitTracker.GetBit();
    deblocking_filter_control_present_flag     = bitTracker.GetBit();
    if (deblocking_filter_control_present_flag)
    {
        deblocking_filter_override_enabled_flag = bitTracker.GetBit();
        pps_deblocking_filter_disabled_flag     = bitTracker.GetBit();
        if (!pps_deblocking_filter_disabled_flag)
        {
            pps_beta_offset_div2 = bitTracker.SignedExpGolomb();
            pps_tc_offset_div2   = bitTracker.SignedExpGolomb();
        }
    }

    pps_scaling_list_data_present_flag = bitTracker.GetBit();
    if (pps_scaling_list_data_present_flag)
    {
        scalingListData.Parse(bitTracker);
    }

    lists_modification_present_flag  = bitTracker.GetBit();
    log2_parallel_merge_level_minus2 = bitTracker.UnsignedExpGolomb();
    slice_segment_header_extension_present_flag = bitTracker.GetBit();
    pps_extension_present_flag = bitTracker.GetBit();

    if (pps_extension_present_flag)
    {
        pps_range_extension_flag      = bitTracker.GetBit();
        pps_multilayer_extension_flag = bitTracker.GetBit();
        pps_3d_extension_flag         = bitTracker.GetBit();
        pps_scc_extension_flag        = bitTracker.GetBit();
        pps_extension_4bits           = bitTracker.GetBits(4);

        ThrowIf(true, "Extension is not supported yet!");
    }

    rbsp_trailing_bits(bitTracker);
}

PictureParameterSet::PictureParameterSet(BitTracker &bitTracker)
{
    CleanUpObject(this);
    Parse(bitTracker);
}

PictureParameterSet::~PictureParameterSet()
{

}

CodecError HEVCCodec::Parse(const std::vector<uint8_t> &buffer)
{
	return Parse(buffer.data(), buffer.size());
}

CodecError HEVCCodec::Parse(const uint8_t *data, size_t size)
{
	BitTracker bitTracker{ data, size };

    NAL nal{ bitTracker };
    switch (nal.GetType())
    {
    case NAL::Type::VPP:
        vps = new VideoParameterSet{ bitTracker };
        break;

    case NAL::Type::SPS:
        sps = new SequenceParameterSet{ bitTracker };
        break;

    case NAL::Type::PPS:
        pps = new PictureParameterSet{ bitTracker };
        break;

    default:
        LOG::WARN("Unsupported NAL unit type {0}", nal.GetType());
        return CodecError::Success;
    }

    while (!bitTracker.NoMoreData() &&
        bitTracker.Preview(24) != 0x000001 &&
        bitTracker.Preview(32) != 0x00000001)
    {
        auto traillingZeroBits = bitTracker.GetBits(8);
    }

    return CodecError::Preparing;
}

}
}
