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
    private:
        const int width;
        const int height;
        GLFWwindow* m_window;
    };
}
