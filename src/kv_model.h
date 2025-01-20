#pragma once
#include "kv_device.h"

#define GLM_FORCE_RADIANS
// depth from 0 to 1, not -1 to 1 (opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace kong
{
    class KongModel
    {
    public:
        struct Vertex
        {
            glm::vec2 position;
            glm::vec3 color;
            static std::vector<VkVertexInputBindingDescription> getBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescription();
        };
        
        KongModel(KongDevice& device, const std::vector<Vertex>& vertices);
        ~KongModel();
    
        KongModel(const KongModel&) = delete;
        KongModel& operator=(const KongModel&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        
    private:
        void createVertexBuffer(const std::vector<Vertex>& vertices);
        
        KongDevice& m_kongDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}
