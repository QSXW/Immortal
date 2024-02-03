#include "Pipeline.h"
#include "Device.h"
#include "Shader.h"
#include "Types.h"
#include <cassert>
#include <cstring>

namespace Immortal
{
namespace Metal
{

MTL::VertexFormat CAST(Format format)
{
    switch (Format::ValueType(format))
    {
        case Format::FLOAT:
            return MTL::VertexFormatFloat;

        case Format::VECTOR2:
            return MTL::VertexFormatFloat2;

        case Format::VECTOR3:
            return MTL::VertexFormatFloat3;

        case Format::VECTOR4:
            return MTL::VertexFormatFloat4;

        case Format::R8_UNORM:
            return MTL::VertexFormatUCharNormalized;

        case Format::R8G8_UNORM:
            return MTL::VertexFormatUChar2Normalized;

        case Format::R8G8B8_UNORM:
            return MTL::VertexFormatUChar3Normalized;

        case Format::R8G8B8A8_UNORM:
            return MTL::VertexFormatUChar4Normalized;

        default:
            return MTL::VertexFormatInvalid;
    }
}

uint32_t GetPushConstantIndexFromBindings(NS::Array *bindings)
{
    uint32_t ret = 0;
    for (size_t i = 0; i < bindings->count(); i++)
    {
        MTL::Binding     *binding = bindings->object<MTL::Binding>(i);
        NS::UInteger      index   = binding->index();
        MTL::BindingType  type    = binding->type();
        NS::String       *nsName  = binding->name();
        const char *name = nsName->cString(NS::ASCIIStringEncoding);
        if (type == MTL::BindingTypeBuffer &&
            (!strcmp(name, "push_constant") ||
             !strcmp(name, "pushconstant")  ||
             !strcmp(name, "PushConstant")))
        {
            ret = index;
        }
        nsName->autorelease();
        binding->autorelease();
    }

    return ret;
}

Pipeline::Pipeline(Device *device, Type type) :
    device{ device },
    type{ type }
{

}

Pipeline::~Pipeline()
{

}

GraphicsPipeline::GraphicsPipeline(Device *device) :
    Pipeline{ device }
{

}

GraphicsPipeline::~GraphicsPipeline()
{

}

void GraphicsPipeline::Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
    MTL::RenderPipelineDescriptor *renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();

    for (size_t i = 0; i < shaderCount; i++)
    {
        Shader *shader = InterpretAs<Shader>(ppShader[i]);
        switch (shader->GetStage())
        {
            case ShaderStage::Vertex:
                renderPipelineDescriptor->setVertexFunction(shader->GetFunction());
                break;

            case ShaderStage::Fragment:
                renderPipelineDescriptor->setFragmentFunction(shader->GetFunction());
                break;

            default:
                break;
        }
    }

    renderPipelineDescriptor->setRasterSampleCount(1);
    renderPipelineDescriptor->setRasterizationEnabled(true);
    renderPipelineDescriptor->setInputPrimitiveTopology(MTL::PrimitiveTopologyClassTriangle);

    MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
    MTL::VertexAttributeDescriptorArray *vertexAttributeDescriptorArray = vertexDescriptor->attributes();
    MTL::VertexAttributeDescriptor *vertexAttributeDescriptor = MTL::VertexAttributeDescriptor::alloc()->init();
    MTL::VertexBufferLayoutDescriptor *vertexBufferLayoutDescriptor = MTL::VertexBufferLayoutDescriptor::alloc()->init();
    vertexBufferLayoutDescriptor->setStride(description.GetStride());
    MTL::VertexBufferLayoutDescriptorArray *vertexBufferLayoutDescriptorArray = vertexDescriptor->layouts();
    for (size_t i = 0; i < description.Size(); i++)
    {
        auto &desc = description[i];
        vertexAttributeDescriptor->setFormat(CAST(desc.GetFormat()));
        vertexAttributeDescriptor->setOffset(desc.GetOffset());
        vertexAttributeDescriptor->setBufferIndex(VertexBufferStartIndex);
        vertexAttributeDescriptorArray->setObject(vertexAttributeDescriptor, i);
    }
    vertexAttributeDescriptor->release();

    vertexBufferLayoutDescriptorArray->setObject(vertexBufferLayoutDescriptor, VertexBufferStartIndex);
    vertexBufferLayoutDescriptor->release();

    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    vertexDescriptor->release();

    for (size_t i = 0; i < outputDescription.size(); i++)
    {
        renderPipelineDescriptor->colorAttachments()->object(i)->setPixelFormat(outputDescription[i]);
        if (flags & Pipeline::State::Blend)
        {
            renderPipelineDescriptor->colorAttachments()->object(i)->setBlendingEnabled(true);
            renderPipelineDescriptor->colorAttachments()->object(i)->setRgbBlendOperation(MTL::BlendOperationAdd);
            renderPipelineDescriptor->colorAttachments()->object(i)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(i)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(i)->setAlphaBlendOperation(MTL::BlendOperationAdd);
            renderPipelineDescriptor->colorAttachments()->object(i)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            renderPipelineDescriptor->colorAttachments()->object(i)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        }
    }

    MTL::PipelineOption options = MTL::PipelineOptionArgumentInfo;
    MTL::RenderPipelineReflection *renderPipelineReflection;
    NS::Error *error = nullptr;
    handle = device->GetHandle()->newRenderPipelineState(renderPipelineDescriptor, options, &renderPipelineReflection, &error);
    CheckError(error);

    NS::Array *vertexBindings   = renderPipelineReflection->vertexBindings();
    SetPushConstantIndex(PushConstantIndexType::Vertex, GetPushConstantIndexFromBindings(vertexBindings));
    vertexBindings->release();

    NS::Array *fragmentBindings = renderPipelineReflection->fragmentBindings();
    SetPushConstantIndex(PushConstantIndexType::Fragment, GetPushConstantIndexFromBindings(fragmentBindings));
    fragmentBindings->autorelease();

    renderPipelineReflection->autorelease();
}

ComputePipeline::ComputePipeline(Device *device, SuperShader *shader) :
    ComputePipeline{ device, InterpretAs<Shader>(shader) }
{

}

ComputePipeline::ComputePipeline(Device *device, Shader *shader) :
    Pipeline{ device, Type::Compute },
    threadGroupSize{},
    pushConstantIndex{}
{
    assert(shader->GetStage() == ShaderStage::Compute);

    MTL::ComputePipelineDescriptor *computePipelineDescriptor = MTL::ComputePipelineDescriptor::alloc()->init();
    computePipelineDescriptor->setComputeFunction(shader->GetFunction());

    MTL::PipelineOption options = MTL::PipelineOptionArgumentInfo;
    MTL::ComputePipelineReflection *computePipelineReflection;
    NS::Error *error = nullptr;
    handle = device->GetHandle()->newComputePipelineState(computePipelineDescriptor, options, &computePipelineReflection, &error);
    CheckError(error);

    NS::Array *bindings = computePipelineReflection->bindings();
    pushConstantIndex = GetPushConstantIndexFromBindings(bindings);

    threadGroupSize = shader->GetThreadGroupSize();

    bindings->autorelease();
    computePipelineDescriptor->autorelease();
}

ComputePipeline::~ComputePipeline()
{

}

}
}
