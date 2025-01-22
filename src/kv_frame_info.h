#pragma once
#include "kv_buffer.h"
#include "kv_camera.h"

namespace kong
{
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        KongCamera& camera;
        VkDescriptorSet globalDescriptorSet;
    };
}
