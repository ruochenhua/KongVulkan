#include "kv_window.h"

#include <iostream>
#include <ostream>

using namespace kong;

KongWindow::KongWindow(int w, int h, const std::string& title)
    : width(w), height(h)
{
    glfwInit();
    // 取消window的自动关联，否则vulkan会报错
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // vulkan的窗口resize需要特殊处理，所以要在这里先关掉
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    // glfw设定，当窗口重设大小的时候，调用callback传入Windows的指针和重设大小后的长宽数据
    glfwSetWindowSizeCallback(m_window, framebufferResizeCallback);
}

KongWindow::~KongWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool KongWindow::ShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void KongWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

VkExtent2D KongWindow::getExtent() const
{
    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
}

void KongWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto kongWindow = reinterpret_cast<KongWindow*>(glfwGetWindowUserPointer(window));
    kongWindow->frameBufferResized = true;
    kongWindow->width = width;
    kongWindow->height = height;
}
