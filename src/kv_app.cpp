#include "kv_app.h"

#include <chrono>

#include "keyboard_movement.h"
#include "kv_simple_render_system.h"
#include "glm/ext/matrix_transform.hpp"

using namespace kong;

struct GlobalUbo
{
    glm::mat4 projectionView {1.};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1., -3., -1.});
};

KongApp::KongApp()
{
    m_globalPool = KongDescriptorPool::Builder(m_device)
                    .setMaxSets(KongSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, KongSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();
    
    loadGameobjects();
}

KongApp::~KongApp()
{}

void KongApp::run()
{
    std::vector<std::unique_ptr<KongBuffer>> uboBuffers(KongSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] = std::make_unique<KongBuffer>(
            m_device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }
    // KongBuffer globalUboBuffer{
    // m_device,
    // sizeof(GlobalUbo),
    //     KongSwapChain::MAX_FRAMES_IN_FLIGHT,    //和swapchain同时在渲染的frame数量匹配，这样每个frame都可以使用不同的ubo，不需要进行同步操作
    //     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    //     m_device.properties.limits.minUniformBufferOffsetAlignment,
    // };
    //
    // globalUboBuffer.map();

    auto globalSetLayout = KongDescriptorSetLayout::Builder(m_device)
                    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                    .build();
    
    std::vector<VkDescriptorSet> globalDiscriptorSets{KongSwapChain::MAX_FRAMES_IN_FLIGHT};
    for (int i = 0; i < globalDiscriptorSets.size(); i++)
    {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        KongDescriptorWriter(*globalSetLayout, *m_globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDiscriptorSets[i]);
    }
    
    SimpleRenderSystem simpleRenderSystem{m_device, m_renderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout()};
    KongCamera camera{};
    camera.SetViewDirection(glm::vec3(0), glm::vec3(0.5, 0.1, 1));

    auto viewerObject = KongGameObject::CreateGameObject();
    KeyboardMovementController cameraController{};
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    while (!m_window.ShouldClose())
    {
        glfwPollEvents();
        // 在poll event之后，因为poll可能会pause（resize），需要记录这段时间的流逝
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(m_window.getGlfwWindow(), frameTime, viewerObject);
        camera.SetViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
        
        float aspect = m_renderer.getAspectRatio();
        //camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.0f);
        
        if (auto commandBuffer = m_renderer.beginFrame())
        {
            int frameIndex = m_renderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDiscriptorSets[frameIndex],
            };

            // 更新ubo数据
            GlobalUbo ubo{};
            ubo.projectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
            // globalUboBuffer.writeToBuffer(&ubo, frameIndex);
            // globalUboBuffer.flushIndex(frameIndex);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();
            
            // render
            /* 每个frame之间可以有多个render pass，比如
             * begin offscreen shadow pass
             * render shadow casting object
             * end offscreen shadow pass
             * // reflection
             * // post process
             */
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo, m_gameObjects);
            m_renderer.endSwapChainRenderPass(commandBuffer);
            m_renderer.endFrame();
        }
    }

    // cpu等待所有gpu任务完成
    vkDeviceWaitIdle(m_device.device());
}

std::unique_ptr<KongModel> createCubeModel(KongDevice& device, glm::vec3 offset) {
    KongModel::Builder modelBuilder{};
    modelBuilder.vertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
 
        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
 
        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
 
        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
 
        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
 
        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto& v : modelBuilder.vertices) {
        v.position += offset;
    }
 
    modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                            12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
 
    return std::make_unique<KongModel>(device, modelBuilder);
}

void KongApp::loadGameobjects()
{
 //   std::shared_ptr<KongModel> model = createCubeModel(m_device, {0.0, 0.0, 0.0});
    std::shared_ptr<KongModel> model = KongModel::createModelFromFile(m_device, "../resource/model/diablo3/diablo3_pose.obj");
    auto gameObject = KongGameObject::CreateGameObject();
    gameObject.model = model;
    gameObject.color = glm::vec3(1.0f, 0.3f, 0.8f);
    gameObject.transform.translation = {0.0, 0.0, 1.5};
    gameObject.transform.scale = {0.5f, 0.5f, 0.5f};
    //cube.transform.rotation = 0.5 * glm::two_pi<float>();

    m_gameObjects.push_back(std::move(gameObject));
}
