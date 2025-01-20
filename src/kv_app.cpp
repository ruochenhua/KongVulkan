#include "kv_app.h"

#include <array>
#include <stdexcept>

#include "kv_simple_render_system.h"
#include "glm/ext/matrix_transform.hpp"

using namespace kong;

KongApp::KongApp()
{
    loadGameobjects();
}

KongApp::~KongApp()
{}

void KongApp::run()
{
    SimpleRenderSystem simpleRenderSystem{m_device, m_renderer.getSwapChainRenderPass()};
    
    while (!m_window.ShouldClose())
    {
        glfwPollEvents();
        if (auto commandBuffer = m_renderer.beginFrame())
        {
            /* 每个frame之间可以有多个render pass，比如
             * begin offscreen shadow pass
             * render shadow casting object
             * end offscreen shadow pass
             * // reflection
             * // post process
             */
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects);
            m_renderer.endSwapChainRenderPass(commandBuffer);
            m_renderer.endFrame();
        }
    }

    // cpu等待所有gpu任务完成
    vkDeviceWaitIdle(m_device.device());
}

void KongApp::loadGameobjects()
{
    std::vector<KongModel::Vertex> vertices {
        {{0.0, -0.5}, {1, 0, 0}},
        {{0.5, 0.5}, {0, 1, 0}},
        {{-0.5, 0.5}, {0, 0, 1}},
    };

    auto model = std::make_shared<KongModel>(m_device, vertices);
    auto triangle = KongGameObject::CreateGameObject();
    triangle.model = model;
    triangle.color = glm::vec3(1.0f, 0.3f, 0.8f);
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.rotation = 0.5 * glm::two_pi<float>();

    m_gameObjects.push_back(std::move(triangle));
}
