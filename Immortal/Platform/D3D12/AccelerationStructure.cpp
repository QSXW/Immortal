#include "AccelerationStructure.h"

namespace Immortal
{
namespace D3D12
{

AccelerationStructure::AccelerationStructure()
{

}


AccelerationStructure::AccelerationStructure(RenderContext *context, const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer) :
    AccelerationStructure{}
{
    auto device = context->GetAddress<Device>();
    if (!device->IsRayTracingSupported())
    {
        return;
    }

    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {
        .Type      = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
        .Flags     = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
        .Triangles = {
                .Transform3x4 = pTranformBuffer ? *pTranformBuffer : D3D12_GPU_VIRTUAL_ADDRESS(0),
                .IndexFormat  = DXGI_FORMAT_R16_UINT,
                .VertexFormat = desc[0].format,
                .IndexCount   = pIndexBuffer->Size() >> 1,
                .VertexCount  = pVertexBuffer->Size() / desc.Stride,
                .IndexBuffer  = *pIndexBuffer,
                .VertexBuffer = {
                    .StartAddress  = *pVertexBuffer,
                    .StrideInBytes = desc.Stride,
                },
            },
    };

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
        .Type          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
        .Flags         = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
        .NumDescs      = 1,
        .DescsLayout   = D3D12_ELEMENTS_LAYOUT_ARRAY,
        .InstanceDescs = D3D12_GPU_VIRTUAL_ADDRESS_NULL,
    };

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    ThrowIf(topLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0, "Incorrect top level prebuild ResultDataMaxSizeInBytes");

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {
        .Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
        .Flags          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
        .NumDescs       = 1,
        .DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY,
        .pGeometryDescs = &geometryDesc,
    };

    device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
    ThrowIf(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0, "Incorrect bottom level prebuild ResultDataMaxSizeInBytes");

    URef<Buffer> scratchBuffer = new Buffer{
        context,
        std::max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes),
        Buffer::Type::Scratch };

    topLevelAS = new Buffer{
        context,
        topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
        Buffer::Type::AccelerationStructure };

    bottomLevelAS = new Buffer{
        context,
        bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
        Buffer::Type::AccelerationStructure };

    D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {
        .Transform = {
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
        },
        .InstanceID                          = 0,
        .InstanceMask                        = 1,
        .InstanceContributionToHitGroupIndex = 0,
        .Flags                               = 0,
        .AccelerationStructure               = *bottomLevelAS
    };

    URef<Buffer> instanceDescs = new Buffer(
        context,
        sizeof(instanceDesc),
        &instanceDesc, Buffer::Type::TransferSource);

#ifdef _DEBUG
    scratchBuffer->SetName("ScratchResource");
    topLevelAS->SetName("TopLevelAccelerationStructure");
    bottomLevelAS->SetName("BottomLevelAccelerationStructure");
    instanceDescs->SetName("InstanceDescs");
#endif

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
        .DestAccelerationStructureData    = *bottomLevelAS,
        .Inputs                           = bottomLevelInputs,
        .SourceAccelerationStructureData  = D3D12_GPU_VIRTUAL_ADDRESS_NULL,
        .ScratchAccelerationStructureData = *scratchBuffer,
    };

    topLevelInputs.InstanceDescs = *instanceDescs;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {
        .DestAccelerationStructureData    = *topLevelAS,
        .Inputs                           = topLevelInputs,
        .SourceAccelerationStructureData  = D3D12_GPU_VIRTUAL_ADDRESS_NULL,
        .ScratchAccelerationStructureData = *scratchBuffer,
    };

    ComPtr<ID3D12GraphicsCommandList4> pRayTracingCommandList;
    context->Submit([&](CommandList *cmdlist) {
        Check(cmdlist->QueryInterface(pRayTracingCommandList.GetAddressOf()));
        pRayTracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
        Barrier<BarrierType::UAV> barrier(*bottomLevelAS);
        pRayTracingCommandList->ResourceBarrier(1, &barrier);
        pRayTracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
     });
}

AccelerationStructure::~AccelerationStructure()
{

}

}
}
