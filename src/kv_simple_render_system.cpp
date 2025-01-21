#include "kv_simple_render_system.h"

#include <array>
#include <stdexcept>

#include "glm/ext/matrix_transform.hpp"

using namespace kong;

struct SimplePushConstantData
{
    glm::mat4 transform {1.0f};
    alignas(16) glm::vec3 color;
};

SimpleRenderSystem::SimpleRenderSystem(KongDevice& device, VkRenderPass renderPass)
    : m_device(device)
{
    createPipelineLayout();
    createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem()
{
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // 用于传输vertex信息之外的信息（如texture， ubo之类的），目前先没有
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    // 用于将一些小量的数据送到shader中
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(m_pipelineLayout != nullptr && "pipelineLayout is null");
    
    // 使用swapchain的大小而不是Windows的，因为这两个有可能不是一一对应
    PipelineConfigInfo pipelineConfig{};
    KongPipeline::defaultPipeLineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<KongPipeline>(m_device,
        "../resource/shader/simple_shader.vert.spv",
        "../resource/shader/simple_shader.frag.spv",
        pipelineConfig);

}

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,
    std::vector<KongGameObject>& gameObjects,
    const KongCamera& camera)
{
    m_pipeline->bind(commandBuffer);

    auto projectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    
    for (auto& object : gameObjects)
    {
        SimplePushConstantData push{};
        push.color = object.color;
        push.transform = projectionView * object.transform.mat4();
        
        vkCmdPushConstants(commandBuffer, m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(SimplePushConstantData), &push);
        
        object.model->bind(commandBuffer);
        object.model->draw(commandBuffer);
    }
}

