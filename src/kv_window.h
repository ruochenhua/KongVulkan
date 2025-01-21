#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace kong
{
    class KongWindow
    {
    public:
        KongWindow(int width, int height, const std::string& title);
        ~KongWindow();

        KongWindow(const KongWindow&) = delete;
        KongWindow& operator=(const KongWindow&) = delete;

        bool ShouldClose() const;
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        VkExtent2D getExtent() const;

        bool wasWindowResized() const { return frameBufferResized; }

        void resetWindowResizedFlag() { frameBufferResized = false; }
        GLFWwindow* getGlfwWindow() const {return m_window;}
    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        
        int width;
        int height;
        bool frameBufferResized = false;
        GLFWwindow* m_window;
    };
}
