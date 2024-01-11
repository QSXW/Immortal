#pragma once

#ifndef HEVC_H__
#define HEVC_H__

#include "Core.h"
#include "Codec.h"
#include "Vision/Common/BitTracker.h"
#include "Vision/Common/NetworkAbstractionLayer.h"
#include "Math/Math.h"

#ifdef _MSC_VER
#pragma warning(disable: 26495)
#endif

namespace Immortal
{
namespace Vision
{

enum H265
{
    MAX_SUB_LAYERS         = 7,
    MAX_DPB_SIZE           = 16,
    MAX_LONG_TERM_REF_PICS = 32,
    EXTENDED_SAR           = 255,
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

struct ScalingListData
{
    ScalingListData() = default;

    ScalingListData(BitTracker &bitTracker);

    void Parse(BitTracker &bitTracker);

    uint32_t scaling_list_pred_matrix_id_delta;
    uint8_t scaling_list[4][6][64];
    uint8_t scaling_list_dc_coef_minus8[2][6];
    int32_t scaling_list_delta_coef;
};

struct VideoParameterSet : public NAL::Payload
{
    VideoParameterSet(BitTracker &bitTracker);

    void Parse(BitTracker &bitTracker);

    ProfileTierLevel profile_tier_level;
    uint32_t vps_video_parameter_set_id               : 4;
    uint32_t vps_base_layer_internal_flag             : 1;
    uint32_t vps_base_layer_available_flag            : 1;
    uint32_t vps_max_layers_minus1                    : 6;
    uint32_t vps_max_sub_layers_minus1                : 3;
    uint32_t vps_temporal_id_nesting_flag             : 1;
    uint32_t vps_reserved_0xffff_16bits               : 16;
    uint32_t vps_sub_layer_ordering_info_present_flag : 1;
    uint32_t vps_max_layer_id                         : 6;
    uint32_t vps_timing_info_present_flag             : 1;
    uint32_t vps_poc_proportional_to_timing_flag      : 1;

    uint32_t vps_num_layer_sets_minus1;
    uint32_t vps_num_units_in_tick;
    uint32_t vps_time_scale;
    uint32_t vps_num_ticks_poc_diff_one_minus1;
    uint32_t vps_num_hrd_parameters;
    uint8_t  vps_max_dec_pic_buffering[8];
    uint8_t  vps_num_reorder_pics[8];
    uint8_t  vps_max_latency_increase[8];
    uint8_t  layer_id_included_flag[8][8];
};

struct SequenceParameterSet : public NAL::Payload
{
    SequenceParameterSet(BitTracker &bitTracker);

    void Parse(BitTracker &bitTracker);

    void DecodeVuiParameters(BitTracker &bitTracker);

    ProfileTierLevel profile_tier_level;
    ScalingListData scalingListData;

    int32_t sps_seq_parameter_set_id;
    int32_t chroma_format_idc;
    int32_t pic_width_in_luma_samples;
    int32_t pic_height_in_luma_samples;
    int32_t conf_win_left_offset;
    int32_t conf_win_right_offset;
    int32_t conf_win_top_offset;
    int32_t conf_win_bottom_offset;
    int32_t bit_depth_luma;
    int32_t bit_depth_chroma;
    uint32_t log2_max_pic_order_cnt_lsb_minus4;

    struct {
        int sps_max_dec_pic_buffering_minus1;
        int sps_max_num_reorder_pics;
        int sps_max_latency_increase_plus1;
    } subLayers[H265::MAX_SUB_LAYERS];

    uint32_t log2_min_luma_coding_block_size_minus3;
    uint32_t log2_diff_max_min_luma_coding_block_size;
    uint32_t log2_min_luma_transform_block_size_minus2;
    uint32_t log2_diff_max_min_luma_transform_block_size;
    uint32_t max_transform_hierarchy_depth_inter;
    uint32_t max_transform_hierarchy_depth_intra;

    uint32_t log2_min_pcm_luma_coding_block_size_minus3;
    uint32_t log2_diff_max_min_pcm_luma_coding_block_size;

    uint32_t num_short_term_ref_pic_sets;
    uint32_t num_long_term_ref_pics_sps;
    uint16_t lt_ref_pic_poc_lsb_sps[MAX_LONG_TERM_REF_PICS];
    uint8_t used_by_curr_pic_lt_sps_flag[MAX_LONG_TERM_REF_PICS];

    Rational sar;
    uint8_t colour_primaries;
    uint8_t transfer_characteristics;
    uint8_t matrix_coeffs;

    uint32_t chroma_sample_loc_type_top_field;
    uint32_t chroma_sample_loc_type_bottom_field;

    uint32_t def_disp_win_left_offset;
    uint32_t def_disp_win_right_offset;
    uint32_t def_disp_win_top_offset;
    uint32_t def_disp_win_bottom_offset;

    uint32_t vui_num_ticks_poc_diff_one_minus1;
    uint32_t min_spatial_segmentation_idc;
    uint32_t max_bytes_per_pic_denom;
    uint32_t max_bits_per_min_cu_denom;
    uint32_t log2_max_mv_length_horizontal;
    uint32_t log2_max_mv_length_vertical;

    uint32_t sps_video_parameter_set_id               : 4;
    uint32_t sps_max_sub_layers_minus1                : 3;
    uint32_t sps_temporal_id_nesting_flag             : 1;
    uint32_t separate_colour_plane_flag               : 1;
    uint32_t conformance_window_flag                  : 1;
    uint32_t sps_sub_layer_ordering_info_present_flag : 1;
    uint32_t scaling_list_enabled_flag                : 1;
    uint32_t sps_scaling_list_data_present_flag       : 1;
    uint32_t amp_enabled_flag                         : 1;
    uint32_t sample_adaptive_offset_enabled_flag      : 1;
    uint32_t pcm_enabled_flag                         : 1;
    uint32_t pcm_sample_bit_depth_luma_minus1         : 4;
    uint32_t pcm_sample_bit_depth_chroma_minus1       : 4;
    uint32_t pcm_loop_filter_disabled_flag            : 1;
    uint32_t long_term_ref_pics_present_flag          : 1;
    uint32_t sps_temporal_mvp_enabled_flag            : 1;
    uint32_t strong_intra_smoothing_enabled_flag      : 1;
    uint32_t vui_parameters_present_flag              : 1;
    uint32_t aspect_ratio_info_present_flag           : 1;
    uint32_t aspect_ratio_idc                         : 8;
    uint32_t overscan_info_present_flag               : 1;
    uint32_t overscan_appropriate_flag                : 1;
    uint32_t video_signal_type_present_flag           : 1;
    uint32_t video_format                             : 3;
    uint32_t video_full_range_flag                    : 1;
    uint32_t colour_description_present_flag          : 1;
    uint32_t chroma_loc_info_present_flag             : 1;
    uint32_t neutral_chroma_indication_flag           : 1;
    uint32_t field_seq_flag                           : 1;
    uint32_t frame_field_info_present_flag            : 1;
    uint32_t default_display_window_flag              : 1;
    uint32_t vui_timing_info_present_flag             : 1;
    uint32_t vui_num_units_in_tick                    : 32;
    uint32_t vui_time_scale                           : 32;
    uint32_t vui_poc_proportional_to_timing_flag      : 1;
    uint32_t vui_hrd_parameters_present_flag          : 1;
    uint32_t bitstream_restriction_flag               : 1;
    uint32_t tiles_fixed_structure_flag               : 1;
    uint32_t motion_vectors_over_pic_boundaries_flag  : 1;
    uint32_t restricted_ref_pic_lists_flag            : 1;
    uint32_t sps_extension_present_flag               : 1;
    uint32_t sps_range_extension_flag                 : 1;
    uint32_t sps_multilayer_extension_flag            : 1;
    uint32_t sps_3d_extension_flag                    : 1;
    uint32_t sps_scc_extension_flag                   : 1;
    uint32_t sps_extension_4bits                      : 4;
};

struct PictureParameterSet : public NAL::Payload
{
    PictureParameterSet(BitTracker &bitTracker);

    ~PictureParameterSet();

    void Parse(BitTracker &bitTracker);

    ScalingListData scalingListData;

    uint32_t pps_pic_parameter_set_id;
    uint32_t pps_seq_parameter_set_id;
    uint32_t num_ref_idx_l0_default_active_minus1;
    uint32_t num_ref_idx_l1_default_active_minus1;
    uint32_t diff_cu_qp_delta_depth;
    uint32_t num_tile_columns_minus1;
    uint32_t num_tile_rows_minus1;
    uint32_t log2_parallel_merge_level_minus2;

    uint16_t column_width_minus1[19];
    uint16_t row_height_minus1[21];

    int32_t init_qp_minus26;
    int32_t pps_cb_qp_offset;
    int32_t pps_cr_qp_offset;
    int32_t pps_beta_offset_div2;
    int32_t pps_tc_offset_div2;

    uint32_t dependent_slice_segments_enabled_flag       : 1;
    uint32_t output_flag_present_flag                    : 1;
    uint32_t num_extra_slice_header_bits                 : 3;
    uint32_t sign_data_hiding_enabled_flag               : 1;
    uint32_t cabac_init_present_flag                     : 1;
    uint32_t constrained_intra_pred_flag                 : 1;
    uint32_t transform_skip_enabled_flag                 : 1;
    uint32_t cu_qp_delta_enabled_flag                    : 1;
    uint32_t pps_slice_chroma_qp_offsets_present_flag    : 1;
    uint32_t weighted_pred_flag                          : 1;
    uint32_t weighted_bipred_flag                        : 1;
    uint32_t transquant_bypass_enabled_flag              : 1;
    uint32_t tiles_enabled_flag                          : 1;
    uint32_t entropy_coding_sync_enabled_flag            : 1;
    uint32_t uniform_spacing_flag                        : 1;
    uint32_t loop_filter_across_tiles_enabled_flag       : 1;
    uint32_t pps_loop_filter_across_slices_enabled_flag  : 1;
    uint32_t deblocking_filter_control_present_flag      : 1;
    uint32_t deblocking_filter_override_enabled_flag     : 1;
    uint32_t pps_deblocking_filter_disabled_flag         : 1;
    uint32_t pps_scaling_list_data_present_flag          : 1;
    uint32_t lists_modification_present_flag             : 1;
    uint32_t slice_segment_header_extension_present_flag : 1;
    uint32_t pps_extension_present_flag                  : 1;
    uint32_t pps_range_extension_flag                    : 1;
    uint32_t pps_multilayer_extension_flag               : 1;
    uint32_t pps_3d_extension_flag                       : 1;
    uint32_t pps_scc_extension_flag                      : 1;
    uint32_t pps_extension_4bits                         : 4;
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

class HEVCCodec : public VideoCodec
{
public:
    enum
    {
        MAX_SUB_LAYERS = 7
    };

public:
	virtual CodecError Decode(const CodedFrame &codedFrame) override;

    CodecError Parse(const std::vector<uint8_t> &buffer);

    CodecError Parse(const uint8_t *data, size_t size);

protected:
    VideoParameterSet *vps;

    SequenceParameterSet *sps;

    PictureParameterSet *pps;
};

using SuperHEVCCodec = HEVCCodec;

}
}

#endif // !HEVC_H__
