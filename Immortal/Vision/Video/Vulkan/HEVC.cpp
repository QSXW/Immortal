#include "HEVC.h"

namespace Immortal
{
namespace Vision
{
namespace Vulkan
{

static inline VkVideoComponentBitDepthFlagBitsKHR SelectBitDepth(int bitDepth)
{
    switch (bitDepth)
    {
    case 8:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;

    case 10:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;

    case 12:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR;

    default:
        return VK_VIDEO_COMPONENT_BIT_DEPTH_INVALID_KHR;
    }
}

static inline VkVideoChromaSubsamplingFlagBitsKHR SelectChromaSampling(Format format)
{
    switch (Format::ValueType{ format })
    {
    case Format::YUV420P:
        return VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;

    case Format::YUV422P:
        return VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR;

    case Format::YUV444P:
        return VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR;

    default:
        return VK_VIDEO_CHROMA_SUBSAMPLING_INVALID_KHR;
        break;
    }
}

CodecError HEVCCodec::Decode(const std::vector<uint8_t> &rbsp)
{
    Super::Decode(rbsp);

    auto &desc = picture.desc;
    if (!session)
    {
        desc.width  = sps->pic_width_in_luma_samples;
        desc.height = sps->pic_height_in_luma_samples;

        VkVideoProfileInfoKHR profile{};
        profile.pNext               = nullptr;
        profile.sType               = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR;
        profile.lumaBitDepth        = SelectBitDepth(sps->bit_depth_luma);
        profile.chromaBitDepth      = SelectBitDepth(sps->bit_depth_chroma);
        profile.chromaSubsampling   = SelectChromaSampling(desc.format);
        profile.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_EXT;

        session = new VideoSession{
            Device::That,
            desc.format,
            VkExtent2D{ U32(desc.width), U32(desc.height) },
            &profile,
            };
    }

    TransferParameterSet();

    VkVideoBeginCodingInfoKHR beginInfo = {
        .sType = VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR,
        .pNext                  = nullptr,
        .videoSession           = *session,
        .videoSessionParameters = session->GetParameters(),
        .referenceSlotCount     = 16,
    };
    VkVideoEndCodingInfoKHR endInfo{};

    VkVideoDecodeInfoKHR decodeInfo{};

    auto cmdbuf = Device::That->RequestCommandBuffer(Vulkan::Level::Primary);

    cmdbuf->Begin(&beginInfo);
    cmdbuf->DecodeVideoKHR(&decodeInfo);
    cmdbuf->End(&endInfo);

    return CodecError::Succeed;
}

void HEVCCodec::UpdateSequenceParameterSet()
{
    StdVideoH265ProfileTierLevel profileTierLevel = {
        .flags = {
            .general_progressive_source_flag    = sps->profile_tier_level.general.progressive_source_flag,
            .general_interlaced_source_flag     = sps->profile_tier_level.general.interlaced_source_flag,
            .general_non_packed_constraint_flag = sps->profile_tier_level.general.non_packed_constraint_flag,
            .general_frame_only_constraint_flag = sps->profile_tier_level.general.frame_only_constraint_flag,
        },
        .general_profile_idc = (StdVideoH265ProfileIdc)sps->profile_tier_level.general.profile_idc,
        .general_level_idc   = (StdVideoH265LevelIdc)sps->profile_tier_level.general.level_idc,
    };

    ps.sps.pProfileTierLevel                              = &profileTierLevel;
    ps.sps.pic_width_in_luma_samples                      = sps->pic_width_in_luma_samples;
    ps.sps.pic_height_in_luma_samples                     = sps->pic_height_in_luma_samples;
    ps.sps.sps_video_parameter_set_id                     = sps->sps_video_parameter_set_id;
    ps.sps.sps_max_sub_layers_minus1                      = sps->sps_max_sub_layers_minus1;
    ps.sps.sps_seq_parameter_set_id                       = sps->sps_seq_parameter_set_id;
    ps.sps.chroma_format_idc                              = (StdVideoH265ChromaFormatIdc)sps->chroma_format_idc;
    ps.sps.bit_depth_luma_minus8                          = sps->bit_depth_luma - 8;
    ps.sps.bit_depth_chroma_minus8                        = sps->bit_depth_chroma - 8;
    ps.sps.log2_max_pic_order_cnt_lsb_minus4              = sps->log2_max_pic_order_cnt_lsb_minus4;
    ps.sps.log2_min_luma_coding_block_size_minus3         = sps->log2_min_luma_coding_block_size_minus3;
    ps.sps.log2_diff_max_min_luma_coding_block_size       = sps->log2_diff_max_min_luma_coding_block_size;
    ps.sps.log2_min_luma_transform_block_size_minus2      = sps->log2_min_luma_transform_block_size_minus2;
    ps.sps.log2_diff_max_min_luma_transform_block_size    = sps->log2_diff_max_min_luma_transform_block_size;
    ps.sps.max_transform_hierarchy_depth_inter            = sps->max_transform_hierarchy_depth_inter;
    ps.sps.max_transform_hierarchy_depth_intra            = sps->max_transform_hierarchy_depth_intra;
    ps.sps.num_short_term_ref_pic_sets                    = sps->num_short_term_ref_pic_sets;
    ps.sps.num_long_term_ref_pics_sps                     = sps->num_long_term_ref_pics_sps;
    ps.sps.pcm_sample_bit_depth_luma_minus1               = sps->pcm_sample_bit_depth_luma_minus1;
    ps.sps.pcm_sample_bit_depth_chroma_minus1             = sps->pcm_sample_bit_depth_chroma_minus1;
    ps.sps.log2_min_pcm_luma_coding_block_size_minus3     = sps->log2_min_pcm_luma_coding_block_size_minus3;
    ps.sps.log2_diff_max_min_pcm_luma_coding_block_size   = sps->log2_diff_max_min_pcm_luma_coding_block_size;
    ps.sps.conf_win_left_offset                           = sps->conf_win_left_offset;
    ps.sps.conf_win_right_offset                          = sps->conf_win_right_offset;
    ps.sps.conf_win_top_offset                            = sps->conf_win_top_offset;
    ps.sps.conf_win_bottom_offset                         = sps->conf_win_bottom_offset;

    StdVideoH265SpsFlags *flags = &ps.sps.flags;
    flags->sps_temporal_id_nesting_flag        = sps->sps_temporal_id_nesting_flag;
    flags->separate_colour_plane_flag          = sps->separate_colour_plane_flag;
    flags->scaling_list_enabled_flag           = sps->scaling_list_enabled_flag;
    flags->sps_scaling_list_data_present_flag  = sps->sps_scaling_list_data_present_flag;
    flags->amp_enabled_flag                    = sps->amp_enabled_flag;
    flags->sample_adaptive_offset_enabled_flag = sps->sample_adaptive_offset_enabled_flag;
    flags->pcm_enabled_flag                    = sps->pcm_enabled_flag;
    flags->pcm_loop_filter_disabled_flag       = sps->pcm_loop_filter_disabled_flag;
    flags->long_term_ref_pics_present_flag     = sps->long_term_ref_pics_present_flag;
    flags->sps_temporal_mvp_enabled_flag       = sps->sps_temporal_mvp_enabled_flag;
    flags->strong_intra_smoothing_enabled_flag = sps->strong_intra_smoothing_enabled_flag;
    flags->vui_parameters_present_flag         = sps->vui_parameters_present_flag;
    flags->sps_extension_present_flag          = sps->sps_extension_present_flag;
    flags->sps_range_extension_flag            = sps->sps_range_extension_flag;

    ps.sps.pScalingLists            = &ps.spsScalingLists;
    ps.sps.pSequenceParameterSetVui = &ps.vui;
}

void HEVCCodec::UpdatePictureParameterSet()
{
    ps.pps.pps_pic_parameter_set_id              = pps->pps_pic_parameter_set_id;
    ps.pps.pps_seq_parameter_set_id              = pps->pps_seq_parameter_set_id;
    ps.pps.num_extra_slice_header_bits           = pps->num_extra_slice_header_bits;
    ps.pps.num_ref_idx_l0_default_active_minus1  = pps->num_ref_idx_l0_default_active_minus1;
    ps.pps.num_ref_idx_l1_default_active_minus1  = pps->num_ref_idx_l1_default_active_minus1;
    ps.pps.init_qp_minus26                       = pps->init_qp_minus26;
    ps.pps.diff_cu_qp_delta_depth                = pps->diff_cu_qp_delta_depth;
    ps.pps.pps_cb_qp_offset                      = pps->pps_cb_qp_offset;
    ps.pps.pps_cr_qp_offset                      = pps->pps_cr_qp_offset;
    ps.pps.num_tile_columns_minus1               = pps->num_tile_columns_minus1;
    ps.pps.num_tile_rows_minus1                  = pps->num_tile_rows_minus1;

    memcpy(ps.pps.column_width_minus1, pps->column_width_minus1, sizeof(ps.pps.column_width_minus1));
    memcpy(ps.pps.row_height_minus1,   pps->row_height_minus1,   sizeof(ps.pps.row_height_minus1));

    ps.pps.pps_beta_offset_div2                  = pps->pps_beta_offset_div2;
    ps.pps.pps_tc_offset_div2                    = pps->pps_tc_offset_div2;
    ps.pps.log2_parallel_merge_level_minus2      = pps->log2_parallel_merge_level_minus2;

    StdVideoH265PpsFlags *flags = &ps.pps.flags;
    flags->dependent_slice_segments_enabled_flag       = pps->dependent_slice_segments_enabled_flag;
    flags->output_flag_present_flag                    = pps->output_flag_present_flag;
    flags->sign_data_hiding_enabled_flag               = pps->sign_data_hiding_enabled_flag;
    flags->cabac_init_present_flag                     = pps->cabac_init_present_flag;
    flags->constrained_intra_pred_flag                 = pps->constrained_intra_pred_flag;
    flags->transform_skip_enabled_flag                 = pps->transform_skip_enabled_flag;
    flags->cu_qp_delta_enabled_flag                    = pps->cu_qp_delta_enabled_flag;
    flags->pps_slice_chroma_qp_offsets_present_flag    = pps->pps_slice_chroma_qp_offsets_present_flag;
    flags->weighted_pred_flag                          = pps->weighted_pred_flag;
    flags->weighted_bipred_flag                        = pps->weighted_bipred_flag;
    flags->transquant_bypass_enabled_flag              = pps->transquant_bypass_enabled_flag;
    flags->tiles_enabled_flag                          = pps->tiles_enabled_flag;
    flags->entropy_coding_sync_enabled_flag            = pps->entropy_coding_sync_enabled_flag;
    flags->uniform_spacing_flag                        = pps->uniform_spacing_flag;
    flags->loop_filter_across_tiles_enabled_flag       = pps->loop_filter_across_tiles_enabled_flag;
    flags->pps_loop_filter_across_slices_enabled_flag  = pps->pps_loop_filter_across_slices_enabled_flag;
    flags->deblocking_filter_control_present_flag      = pps->deblocking_filter_control_present_flag;
    flags->deblocking_filter_override_enabled_flag     = pps->deblocking_filter_override_enabled_flag;
    flags->pps_deblocking_filter_disabled_flag         = pps->pps_deblocking_filter_disabled_flag;
    flags->pps_scaling_list_data_present_flag          = pps->pps_scaling_list_data_present_flag;
    flags->lists_modification_present_flag             = pps->lists_modification_present_flag;
    flags->slice_segment_header_extension_present_flag = pps->slice_segment_header_extension_present_flag;
    flags->pps_extension_present_flag                  = pps->pps_extension_present_flag;

    ps.pps.pScalingLists = &ps.ppsScalingLists;
}

void HEVCCodec::TransferParameterSet()
{
    if (sps)
    {
        UpdateSequenceParameterSet();
    }
    if (pps)
    {
        UpdatePictureParameterSet();
    }
}

}
}
}
