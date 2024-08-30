#include "AccelerationStructure.h"
#include "Device.h"
#include "Buffer.h"

namespace Immortal
{
namespace D3D12
{

AccelerationStructure::AccelerationStructure() :
    NonDispatchableHandle{}
{

}


AccelerationStructure::AccelerationStructure(Device *device, const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer) :
    NonDispatchableHandle{ device }
{
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
                .IndexCount   = uint32_t(pIndexBuffer->GetSize() >> 1),
                .VertexCount  = uint32_t(pVertexBuffer->GetSize() / desc.GetStride()),
                .IndexBuffer  = *pIndexBuffer,
                .VertexBuffer = {
                    .StartAddress  = *pVertexBuffer,
                    .StrideInBytes = desc.GetStride(),
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
        device,
	    Buffer::Type::Scratch,
        std::max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes)
    };

    topLevelAS = new Buffer{
	    device,
	    Buffer::Type::AccelerationStructure,
        topLevelPrebuildInfo.ResultDataMaxSizeInBytes
    };

    bottomLevelAS = new Buffer{
	    device,
	    Buffer::Type::AccelerationStructure,
        bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes
    };

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
	    device,
	    Buffer::Type::TransferSource,
        sizeof(instanceDesc),
        Format::None,
        &instanceDesc);

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
}

AccelerationStructure::~AccelerationStructure()
{

}

}
}
