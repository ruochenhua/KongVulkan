#include "kv_renderer.h"

#include <array>
#include <stdexcept>

#include "glm/ext/matrix_transform.hpp"

using namespace kong;

VkCommandBuffer KongRenderer::getCurrentCommandBuffer() const
{
    assert(isFrameStarted && "cannot get command buffer when frame not in progress");
    return m_commandBuffers[currentFrameIndex];
}


KongRenderer::KongRenderer(KongWindow& window, KongDevice& device)
    : m_window(window), m_device(device)
{
    recreateSwapChain();
    createCommandBuffers();
}

KongRenderer::~KongRenderer()
{
    freeCommandBuffers();
}

int KongRenderer::getFrameIndex() const
{
    assert(isFrameStarted && "cannot get frame index when frame not in progress");
    return currentFrameIndex;
}

VkCommandBuffer KongRenderer::beginFrame()
{
    assert(!isFrameStarted && "cannot begin frame when frame already in progress");
    
    auto result = m_swapChain->acquireNextImage(&currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return nullptr;
    }
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin command buffer operation!");
    }
    return commandBuffer;
}

void KongRenderer::endFrame()
{
    assert(isFrameStarted && "cannot end frame when frame not in progress");
    auto commandBuffer = getCurrentCommandBuffer();

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to end command buffer operation!");
    }

    auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
   
    if (result == VK_ERROR_OUT_OF_DATE_KHR
        || result == VK_SUBOPTIMAL_KHR
        || m_window.wasWindowResized())
    {
        m_window.resetWindowResizedFlag();
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer commands!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % KongSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void KongRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "cannot beginSwapChainRenderPass when frame not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "cannot begin render pass on command buffer from a different frame");
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapChain->getRenderPass();
    renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(currentImageIndex);
    
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();
    
    std::array<VkClearValue, 2> clearValues{};
    // 对应framebuffer的设定，attachment0是color，attachment1是depth，所以只需要设置对应的颜色和depthStencil的clear值
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    // inline类型代表直接执行command buffer中的渲染指令，不存在引用其他command buffer
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS代表有引用的情况，两种不能混合使用
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0,0}, m_swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void KongRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "cannot endSwapChainRenderPass when frame not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "cannot end render pass on command buffer from a different frame");

    
    vkCmdEndRenderPass(commandBuffer);
}

void KongRenderer::createCommandBuffers()
{
    m_commandBuffers.resize(KongSwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    /* level有两种: primary和secondary
     * primary可以送到Device Graphics Queue执行，但是不能被其他Command Buffer引用
     * secondary不能送到Device Graphics Queue执行，但是可以被其他command buffer引用
     */
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = m_device.getCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device.device(), &allocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void KongRenderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(),
        static_cast<uint32_t>(m_commandBuffers.size()),
        m_commandBuffers.data());

    m_commandBuffers.clear();
}


void KongRenderer::recreateSwapChain()
{
    auto extent = m_window.getExtent();

    while (extent.height == 0 || extent.width == 0)    {
        extent = m_window.getExtent();
        glfwWaitEvents();
    }

    if (m_swapChain == nullptr)
    {
        m_swapChain = std::make_unique<KongSwapChain>(m_device, extent);
    }
    else
    {
        std::shared_ptr<KongSwapChain> oldSwapchain = std::move(m_swapChain);
        m_swapChain = std::make_unique<KongSwapChain>(m_device, extent, oldSwapchain);

        if (!oldSwapchain->compareSwapChainFormats(*m_swapChain.get()))
        {
            throw std::runtime_error("swapchain formats don't match!");
        }
    }
}
