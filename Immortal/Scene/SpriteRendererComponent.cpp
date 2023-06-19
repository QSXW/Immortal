/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Component.h"
#include "Vision/Common/SamplingFactor.h"

namespace Immortal
{

void SpriteRendererComponent::UpdateSprite(const Vision::Picture &picture)
{
    struct {
        Vision::ColorSpace colorSpace;
        int _10Bits;
    } properties {
        Vision::ColorSpace::YUV,
        0
    };

    Texture::Description desc = {
        Format::RGBA8,
        Wrap::Clamp,
        Filter::Bilinear,
        true
       };

    auto width = picture.desc.width;
    auto height = picture.desc.height;
    if (!Sprite || picture.desc.width != Sprite->Width() || picture.desc.height != Sprite->Height())
    {
        Sprite = Render::Create<Image>(picture.desc.width, picture.desc.height, nullptr, desc);
        Result = Render::Create<Image>(picture.desc.width, picture.desc.height, nullptr, desc);
    }
    
    desc.mipLevels = false;
    if (picture.desc.format == Format::RGBA8)
    {
        Sprite->Update(picture[0]);
        return;
    }

    if (picture.desc.format.IsType(Format::_10Bits))
    {
        desc.format = Format::R16;
        properties._10Bits = 1;
    }
    else
    {
        desc.format = Format::R8;
    }

    if (!pNext)
    {
        pNext = new Extension;
        pNext->chromaFormat = desc.format;

        SamplingFactor samplingFactor{ picture.desc.format };

        auto width = picture.shared->linesize[0] / desc.format.BytesPerPixel();
        auto height = picture.desc.height;

        if (picture.shared->type == Vision::PictureType::Device)
        {
            width = picture.desc.width;
        }

        pNext->input[0] = Render::Create<Image>(width, height, nullptr, desc);

        auto samplingWidth  = width >> samplingFactor.x;
        auto samplingHeight = height >> samplingFactor.y;
        if (picture.desc.format.IsType(Format::NV))
        {
            Texture::Description desc {
                Format::RG8,
                Wrap::Clamp,
                Filter::Bilinear,
                false
            };
            pNext->chromaFormat = (picture.desc.format & Format::_10Bits) != Format::None ? Format::RG16 : Format::RG8;
            desc.format = pNext->chromaFormat;
            pNext->input[1] = Render::Create<Image>(samplingWidth, samplingHeight, nullptr, desc);
        }
        else
        {
            pNext->input[1] = Render::Create<Image>(samplingWidth, samplingHeight, nullptr, desc);
            pNext->input[2] = Render::Create<Image>(samplingWidth, samplingHeight, nullptr, desc);
        }
    }

    Ref<ComputePipeline> colorSpacePipeline = picture.desc.format.IsType(Format::NV) ? Pipelines::ColorSpaceNV122RGBA8 : Pipelines::ColorSpace;

    properties.colorSpace = picture.GetProperty<Vision::ColorSpace>();
    colorSpacePipeline->AllocateDescriptorSet((uint64_t)this);

    /* Upload frame data for each plane to the texture */
    if (picture.shared->type == Vision::PictureType::Device)
    {
        pNext->input[0]->PlatformSpecifiedUpdate(picture[0], 0);
    }
    else
    {
        pNext->input[0]->Update(picture[0], picture.shared->linesize[0] / desc.format.BytesPerPixel());
    }
    colorSpacePipeline->Bind(pNext->input[0], 0);

    int samplingWidth = picture.shared->linesize[1];
    if (picture.desc.format.IsType(Format::NV))
    {
        samplingWidth >>= 1;
        colorSpacePipeline->Bind(Sprite, 2);
    }
    else
    {   
        pNext->input[2]->Update(picture[2], picture.shared->linesize[2] / pNext->chromaFormat.BytesPerPixel());
        colorSpacePipeline->Bind(pNext->input[2], 2);
        colorSpacePipeline->Bind(Sprite, 3);
    }

    if (picture.shared->type == Vision::PictureType::Device)
    {
        pNext->input[1]->PlatformSpecifiedUpdate(picture[0], 1);
    }
    else
    {
        pNext->input[1]->Update(picture[1], samplingWidth / pNext->chromaFormat.BytesPerPixel());
    }

    colorSpacePipeline->Bind(pNext->input[1], 1);

    Render::PushConstant(colorSpacePipeline, sizeof(properties), &properties, 0);
    Render::Dispatch(colorSpacePipeline, SLALIGN(picture.shared->linesize[0] / pNext->chromaFormat.BytesPerPixel(), 32) / 32, SLALIGN(height, 32) / 32, 1);
	Render::Blit(Sprite);
}

}
