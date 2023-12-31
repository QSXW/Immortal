#include "AccelerationStructure.h"

namespace Immortal
{
namespace Vulkan
{

AccelerationStructureLevel::AccelerationStructureLevel() :
    handle{},
    buffer{}
{

}

AccelerationStructureLevel::AccelerationStructureLevel(Device *device, VkAccelerationStructureTypeKHR type, const VkAccelerationStructureGeometryKHR *pGeometry, uint32_t geometryCount, const uint32_t *pPrimitiveCount) :
    AccelerationStructureLevel{}
{
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {
        .sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext                    = nullptr,
        .type                     = type,
        .flags                    = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .srcAccelerationStructure = VK_NULL_HANDLE,
        .dstAccelerationStructure = VK_NULL_HANDLE,
        .geometryCount            = geometryCount,
        .pGeometries              = pGeometry,
    };

    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
        .pNext = nullptr,
    };

    device->GetAccelerationStructureBuildSizesKHR(
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildGeometryInfo,
        pPrimitiveCount,
        &buildSizeInfo
    );

    buffer = new Buffer{
        device,
	    Buffer::Type::AccelerationStructure,
        buildSizeInfo.accelerationStructureSize,
    };

    VkAccelerationStructureCreateInfoKHR createInfo = {
        .sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext         = nullptr,
        .createFlags   = 0,
        .buffer        = *buffer,
        .offset        = buffer->GetOffset(),
        .size          = buildSizeInfo.accelerationStructureSize,
        .type          = type,
        .deviceAddress = 0,
    };

    Check(device->Create(&createInfo, &handle));

    buildGeometryInfo.dstAccelerationStructure = handle;

    URef<Buffer> scratchBuffer = new Buffer{
        device,
	    Buffer::Type::Scratch,
        buildSizeInfo.buildScratchSize,
    };

    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

    VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .pNext = nullptr,
        .accelerationStructure = handle,
    };

    deviceAddress = device->GetAccelerationStructureAddressKHR(&deviceAddressInfo);
}

AccelerationStructureLevel::~AccelerationStructureLevel()
{
    Destroy();
}

AccelerationStructureLevel::AccelerationStructureLevel(AccelerationStructureLevel &&other) :
    AccelerationStructureLevel{}
{
    other.Swap(*this);
}

AccelerationStructureLevel &AccelerationStructureLevel::operator =(AccelerationStructureLevel &&other)
{
    AccelerationStructureLevel(std::move(other)).Swap(*this);
    return *this;
}

void AccelerationStructureLevel::Swap(AccelerationStructureLevel &other)
{
    std::swap(handle, other.handle);
    std::swap(buffer, other.buffer);
}

void AccelerationStructureLevel::Destroy()
{
    if (handle)
    {
        auto device = buffer->GetAddress<Device>();
        device->DestroyAsync(handle);
        handle = nullptr;
    }
}

AccelerationStructure::AccelerationStructure(Device *device) :
    device{ device },
    bottomLevelAS{},
    topLevelAS{}
{

}

AccelerationStructure::AccelerationStructure(Device *device, const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer) :
    AccelerationStructure{ device }
{
    {
        std::array<VkAccelerationStructureGeometryKHR, 1> geomerty{
            VkAccelerationStructureGeometryKHR{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .pNext = nullptr,
                .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                .geometry = {
                    .triangles = {
                        .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                        .pNext        = nullptr,
                        .vertexFormat = desc[0].format,
                        .vertexData   = {
                            .deviceAddress = pVertexBuffer->GetDeviceAddress(),
                        },
		                .vertexStride = desc.GetStride(),
                        .maxVertex    = uint32_t(pVertexBuffer->GetSize() / 3),
                        .indexType    = VK_INDEX_TYPE_UINT16,
                        .indexData = {
                            .deviceAddress = pIndexBuffer->GetDeviceAddress(),
                        },
                        .transformData = {
                            .deviceAddress = pTranformBuffer ? pTranformBuffer->GetDeviceAddress() : 0,
                        },
                    },
                },
                .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
            }
        };

        uint32_t numTriangles = 1;
        bottomLevelAS = AccelerationStructureLevel{
            device,
            VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            geomerty.data(),
            geomerty.size(),
            &numTriangles
        };
    }

    {
        VkTransformMatrixKHR transformMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        };

        VkAccelerationStructureInstanceKHR instance = {
            .transform                              = transformMatrix,
            .instanceCustomIndex                    = 0,
            .mask                                   = 0xFF,
            .instanceShaderBindingTableRecordOffset = 0,
            .flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
            .accelerationStructureReference         = bottomLevelAS,
        };

        URef<Buffer> instanceBuffer = new Buffer{
            device,
		    Buffer::Type::AccelerationStructureSource,
            sizeof(VkAccelerationStructureInstanceKHR),
            &instance,
        };

        std::array<VkAccelerationStructureGeometryKHR, 1> geometry = {
            VkAccelerationStructureGeometryKHR{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .pNext = nullptr,
                .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
                .geometry = {
                    .instances = {
                        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                        .arrayOfPointers = VK_FALSE,
                        .data = {
                            .deviceAddress = instanceBuffer->GetDeviceAddress(),
                        },
                    },
                 },
                .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
            }
        };

        uint32_t primitiveCount = 1;
        bottomLevelAS = AccelerationStructureLevel{
            device,
            VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            geometry.data(),
            geometry.size(),
            &primitiveCount
        };
    }
}

AccelerationStructure::~AccelerationStructure()
{
    if (bottomLevelAS)
    {
        bottomLevelAS.Destroy();
    }
    if (topLevelAS)
    {
        topLevelAS.Destroy();
    }
}

}
}
