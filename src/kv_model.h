#pragma once
#include "kv_device.h"

#define GLM_FORCE_RADIANS
// depth from 0 to 1, not -1 to 1 (opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <glm/glm.hpp>

#include "kv_buffer.h"

namespace kong
{
    class KongModel
    {
    public:
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};
            
            static std::vector<VkVertexInputBindingDescription> getBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescription();
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };
        
        KongModel(KongDevice& device, const Builder& builder);
        ~KongModel();
    
        KongModel(const KongModel&) = delete;
        KongModel& operator=(const KongModel&) = delete;

        static std::unique_ptr<KongModel> createModelFromFile(KongDevice& device, const std::string& filepath);
        
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        
    private:
        void createVertexBuffer(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
        
        KongDevice& m_kongDevice;

        std::unique_ptr<KongBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<KongBuffer> indexBuffer;
        uint32_t indexCount;
    };
}
