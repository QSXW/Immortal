#include "Render2D.h"

#include "Graphics.h"
#include "OrthographicCamera.h"
#include "FileSystem/FileSystem.h"
#include <array>

namespace Immortal
{

Render2D::Render2D() :
    rectIndexCount{}
{
	Stream stream = { "Assets/Shaders/hlsl/Render2D.hlsl", StreamMode::Read };
    if (stream.Readable())
    {
		std::string source;
		stream.Read(source);

        auto device = Graphics::GetDevice();
		URef<Shader> vertexShader = device->CreateShader("render2d", ShaderStage::Vertex, source, "VSMain");
		URef<Shader> pixelShader  = device->CreateShader("render2d", ShaderStage::Pixel, source, "PSMain");

        Shader *shaders[] = { vertexShader, pixelShader };
		pipeline = device->CreateGraphicsPipeline();
		pipeline->Enable(Pipeline::State::Blend);
		pipeline->Construct(shaders,
            SL_ARRAY_LENGTH(shaders),
            {
                { Format::VECTOR3,  "POSITION"      },
                { Format::VECTOR4,  "COLOR"         },
                { Format::VECTOR2,  "TEXCOORD"      },
                { Format::FLOAT,    "INDEX"         },
                { Format::FLOAT,    "TILING_FACTOR" },
                { Format::R32_SINT, "OBJECT_ID"     }
            },
            {
                Format::RGBA8,
                Format::Depth24Stencil8
            }
        );

		descriptorSet = device->CreateDescriptorSet(pipeline);

		pointSampler  = device->CreateSampler(Filter::Nearest, AddressMode::Repeat);
		linearSampler = device->CreateSampler(Filter::Linear, AddressMode::Repeat);
		descriptorSet->Set(0, linearSampler);
		sampler = linearSampler;

        vertexBuffer = device->CreateBuffer(sizeof(RectVertex) * MaxVertices, BufferType::Vertex);
        indexBuffer  = device->CreateBuffer(sizeof(uint32_t) * MaxIndices, BufferType::Index);

        uint32_t *ptr = {};
		indexBuffer->Map((void **)&ptr, indexBuffer->GetSize(), 0);
        for (uint32_t i = 0, offset = 0; i < MaxIndices; i += 6)
        {
            ptr[i + 0] = offset + 0;
            ptr[i + 1] = offset + 1;
            ptr[i + 2] = offset + 2;

            ptr[i + 3] = offset + 2;
            ptr[i + 4] = offset + 3;
            ptr[i + 5] = offset + 0;

            offset += 4;
        }
		indexBuffer->Unmap();
    }
}

Render2D::~Render2D()
{

}

void Render2D::Flush()
{
    if (!rectIndexCount)
    {
        return;
    }

    if (pRectVertex)
	{
		vertexBuffer->Unmap();
	}

    uint32_t indexCount = rectIndexCount;
	Graphics::Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
		Buffer *buffers[] = { vertexBuffer };
		commandBuffer->SetPipeline(pipeline);
		commandBuffer->SetVertexBuffers(0, 1, buffers, sizeof(RectVertex));
		commandBuffer->SetIndexBuffer(indexBuffer, Format::R32_UINT);
		commandBuffer->SetDescriptorSet(descriptorSet);
		commandBuffer->PushConstants(ShaderStage::Vertex, &viewProjection, sizeof(viewProjection), 0);
		commandBuffer->DrawIndexedInstance(indexCount, 1, 0, 0, 0);
	});
}

void Render2D::StartBatch()
{
    rectIndexCount = 0;
	vertexBuffer->Map((void **)&pRectVertex, vertexBuffer->GetSize(), 0);
    textureIndex   = 0;
}

void Render2D::NextBatch()
{
    Flush();
    StartBatch();
}

void Render2D::BeginScene(const Camera &camera)
{
	const OrthographicCamera &orthographicCamera = (const OrthographicCamera &)camera;
    if (orthographicCamera.GetZoomLevel() <= 0.002)
    {
        if (sampler != pointSampler)
        {
			sampler = pointSampler;
			descriptorSet->Set(0, sampler);
        }
    }
    else
    {
		if (sampler != linearSampler)
		{
			sampler = linearSampler;
			descriptorSet->Set(0, sampler);
		}
    }

    viewProjection = camera.ViewProjection();
    NextBatch();
}

void Render2D::EndScene()
{
	NextBatch();
}

void Render2D::DrawRect(const Matrix4 &transform, const Vector4 &color, int object)
{
	DrawRect(transform, Graphics::Preset()->Textures.White, 1.0f, color, object);
}

void Render2D::DrawRect(const Matrix4 &transform, const Ref<Texture> &texture, float tilingFactor, const Vector4 &tintColor, int object)
{
    constexpr size_t RectVertexCount = 4;
    static Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    static Matrix4 RectVertexPosition = {
		{ -0.5f, -0.5f, 0.0f, 1.0f },
	    {  0.5f, -0.5f, 0.0f, 1.0f },
	    {  0.5f,  0.5f, 0.0f, 1.0f },
	    { -0.5f,  0.5f, 0.0f, 1.0f },
    };

    if (rectIndexCount >= MaxIndices || textureIndex >= MaxTextureSlots)
    {
        NextBatch();
    }

    int index = 0;
	for (; index < textureIndex; index++)
    {
		if (textures[index] == texture)
        {
			break;
        }
    }

    if (index == textureIndex)
    {
		index = textureIndex++;
		descriptorSet->Set(textureIndex, texture);
		textures[index] = texture;
    }
    else
    {
		index--;
    }

    for (size_t i = 0; i < RectVertexCount; i++, pRectVertex++)
    {
        pRectVertex->Position     = Vector4{ transform * RectVertexPosition[i] };
        pRectVertex->Color        = tintColor;
        pRectVertex->TexCoord     = textureCoords[i];
		pRectVertex->TexIndex     = index;
        pRectVertex->TilingFactor = tilingFactor;
        pRectVertex->Object       = object;
    }

    rectIndexCount += 6;
}

}
