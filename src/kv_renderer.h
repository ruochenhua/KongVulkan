#pragma once
#include <memory>

#include "kv_swap_chain.h"
#include "kv_window.h"

namespace kong
{
    class KongRenderer
    {
    public:
        KongRenderer(KongWindow& window, KongDevice& device);
        ~KongRenderer();
    
        KongRenderer(const KongRenderer&) = delete;
        KongRenderer& operator=(const KongRenderer&) = delete;

        int getFrameIndex() const;
        
        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        bool isFrameInProgress() const {return isFrameStarted;}
        VkCommandBuffer getCurrentCommandBuffer() const;

        VkRenderPass getSwapChainRenderPass() const {return m_swapChain->getRenderPass();}
    private:
        void createCommandBuffers();
        void freeCommandBuffers();

        void recreateSwapChain();
        
        KongWindow& m_window;
        KongDevice& m_device;

        std::unique_ptr<KongSwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t currentImageIndex = 0;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;
    };
}
