#pragma once
#include <memory>

#include "kv_camera.h"
#include "kv_frame_info.h"
#include "kv_game_object.h"
#include "kv_pipeline.h"
namespace kong
{
    /* 
     * 简单的渲染系统，包含pipeline信息
     * 不同的渲染阶段为一个系统，比如说pbr、后处理可以作为单独的rendersystem
     */ 
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(KongDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();
    
        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;
        void renderGameObjects(const FrameInfo& frameInfo, std::vector<KongGameObject>& gameObjects);
    
    private:
        
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        
        KongDevice& m_device;
        
        std::unique_ptr<KongPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
         
    };
}
