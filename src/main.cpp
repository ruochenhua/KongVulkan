#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include  "VkBootstrap.h"
bool init_vulkan()
{
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("example vulkan app")
                                        .request_validation_layers()
                                        .use_default_debug_messenger().build();

    if (!inst_ret)
    {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
        return false;
    }

    vkb::Instance vkb_inst = inst_ret.value();
    if (!glfwInit() || !glfwVulkanSupported())
    {
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // 这行命令让glfw不要默认将窗口和opengl关联，否则下面的surface创建过程会报错
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    VkSurfaceKHR surface_khr = nullptr;
    VkResult err = glfwCreateWindowSurface(vkb_inst.instance, window, nullptr, &surface_khr);
    if (err != VK_SUCCESS)
    {
        return false;
    }
    
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    auto phys_ret = selector.set_surface (surface_khr)
                        .set_minimum_version (1, 1) // require a vulkan 1.1 capable device
                        .require_dedicated_transfer_queue ()
                        .select ();
    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return false;
    }
    
    return true;
}

void main()
{
    init_vulkan();
}