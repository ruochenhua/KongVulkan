#pragma once
#include <memory>

#include "kv_camera.h"
#include "kv_game_object.h"
#include "kv_pipeline.h"
namespace kong
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(KongDevice& device, VkRenderPass renderPass);
        ~SimpleRenderSystem();
    
        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;
        void renderGameObjects(VkCommandBuffer commandBuffer,
            std::vector<KongGameObject>& gameObjects,
            const KongCamera& camera);
    
    private:
        
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
        
        KongDevice& m_device;
        
        std::unique_ptr<KongPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
         
    };
}
