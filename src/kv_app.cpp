#include "kv_app.h"

#include <chrono>

#include "keyboard_movement.h"
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
            /* 每个frame之间可以有多个render pass，比如
             * begin offscreen shadow pass
             * render shadow casting object
             * end offscreen shadow pass
             * // reflection
             * // post process
             */
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);
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
