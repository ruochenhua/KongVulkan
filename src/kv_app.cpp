#include "kv_app.h"

#include <array>
#include <stdexcept>

#include "assimp/code/AssetLib/MMD/MMDCpp14.h"

using namespace kong;

KongApp::KongApp()
{
    loadModels();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

KongApp::~KongApp()
{
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void KongApp::run()
{
    while (!m_window.ShouldClose())
    {
        glfwPollEvents();
        drawFrame();
    }

    // cpu等待所有gpu任务完成
    vkDeviceWaitIdle(m_device.device());
}

void KongApp::loadModels()
{
    std::vector<KongModel::Vertex> vertices {
        {{0.0, -0.5}, {1, 0, 0}},
        {{0.5, 0.5}, {0, 1, 0}},
        {{-0.5, 0.5}, {0, 0, 1}},
    };

    m_model = std::make_unique<KongModel>(m_device, vertices);
}

void KongApp::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // 用于传输vertex信息之外的信息（如texture， ubo之类的），目前先没有
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    // 用于将一些小量的数据送到shader中
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void KongApp::createPipeline()
{
    // 使用swapchain的大小而不是Windows的，因为这两个有可能不是一一对应
    PipelineConfigInfo pipelineConfig{};
    KongPipeline::defaultPipeLineConfigInfo(pipelineConfig, m_swapChain.width(), m_swapChain.height());
    pipelineConfig.renderPass = m_swapChain.getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<KongPipeline>(m_device,
        "../resource/shader/simple_shader.vert.spv",
        "../resource/shader/simple_shader.frag.spv",
        pipelineConfig);

}

void KongApp::createCommandBuffers()
{
    m_commandBuffers.resize(m_swapChain.imageCount());

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

    for (int i = 0; i < m_commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin command buffer operation!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain.getRenderPass();
        renderPassInfo.framebuffer = m_swapChain.getFrameBuffer(i);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        // 对应framebuffer的设定，attachment0是color，attachment1是depth，所以只需要设置对应的颜色和depthStencil的clear值
        clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // inline类型代表直接执行command buffer中的渲染指令，不存在引用其他command buffer
        // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS代表有引用的情况，两种不能混合使用
        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_pipeline->bind(m_commandBuffers[i]);

        m_model->bind(m_commandBuffers[i]);
        m_model->draw(m_commandBuffers[i]);
        
        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to end command buffer operation!");
        }
    }
}

void KongApp::drawFrame()
{
    uint32_t imageIndex = 0;
    auto result = m_swapChain.acquireNextImage(&imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    result = m_swapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer commands!");
    }
}
