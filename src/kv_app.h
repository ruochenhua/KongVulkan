#pragma once
#include <memory>

#include "kv_pipeline.h"
#include "kv_swap_chain.h"
#include "kv_window.h"

namespace kong
{
    class KongApp
    {
    public:
        KongApp();
        ~KongApp();
    
        KongApp(const KongApp&) = delete;
        KongApp& operator=(const KongApp&) = delete;
        
        void run();

        static constexpr int window_width = 800;
        static constexpr int window_height = 600;
        
    private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();
        
        KongWindow m_window {window_width, window_height, "kong vulkan"};
        KongDevice m_device{m_window};
        KongSwapChain m_swapChain{m_device, m_window.getExtent()};
        std::unique_ptr<KongPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkCommandBuffer> m_commandBuffers;
    };
}
