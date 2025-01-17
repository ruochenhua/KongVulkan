#include "kv_model.h"

using namespace kong;

std::vector<VkVertexInputBindingDescription> KongModel::Vertex::getBindingDescription()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> KongModel::Vertex::getAttributeDescription()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    // position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;                      // location = 0
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;  // vec2
    attributeDescriptions[0].offset = offsetof(Vertex, position);
    // color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

KongModel::KongModel(KongDevice& device, const std::vector<Vertex>& vertices)
    : m_kongDevice{device}
{
    createVertexBuffer(vertices);
}

KongModel::~KongModel()
{
    /*
     * buffer和memory分开，是由于分配内存是需要时间的，并且GPU有分配内存的操作数的上限（几千），在复杂场景下很容易遇到瓶颈
     * 所以正确的做法是请求大块内存并将内存的部分分配给不同资源。
     * 了解Vulkan memory allocator（VMA）库
     */
    vkDestroyBuffer(m_kongDevice.device(), vertexBuffer, nullptr);
    vkFreeMemory(m_kongDevice.device(), vertexBufferMemory, nullptr);
}

void KongModel::draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

void KongModel::bind(VkCommandBuffer commandBuffer)
{
    // 绑定command buffer和vertex buffer
    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void KongModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be greater than 3");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    /*
     * 映射host（CPU）和device（GPU）的memory
     * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT：表示memory数据能够在host上面可见（host写）
     * VK_MEMORY_PROPERTY_HOST_COHERENT_BIT：host和device的memory是连贯的
     */
    m_kongDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer, vertexBufferMemory);

    void* data;
    // 指向host memory的地址
    vkMapMemory(m_kongDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);

    // 将顶点数据拷贝到host的分配内存中，由于前面设置了device和host的内存映射（COHERENT_BIT），顶点数据会自动同步到device的内存中
    // 如果前面没有设置COHERENT_BIT，需要手动调用vkFlushMappedMemoryRanges来同步
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    
    vkUnmapMemory(m_kongDevice.device(), vertexBufferMemory);
}
