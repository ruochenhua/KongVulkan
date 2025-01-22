#include "kv_model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>

#include "tiny_obj_loader.h"

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
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
    
    // // position
    // attributeDescriptions[0].binding = 0;
    // attributeDescriptions[0].location = 0;                      // location = 0
    // attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
    // attributeDescriptions[0].offset = offsetof(Vertex, position);
    // // color
    // attributeDescriptions[1].binding = 0;
    // attributeDescriptions[1].location = 1;
    // attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    // attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

void KongModel::Builder::loadModel(const std::string& filepath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};
            if (index.vertex_index >= 0)
            {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // color数据是可选的，接在vertex后面
                auto colorIndex = 3 * index.vertex_index + 2;
                if (colorIndex < attrib.colors.size())
                {
                    vertex.color = {
                        attrib.colors[colorIndex - 2],
                        attrib.colors[colorIndex - 1],
                        attrib.colors[colorIndex]
                    };  
                }
                else
                {
                    vertex.color = {0.7, 0.7, 0.7};
                }
            }

            if (index.normal_index >= 0)
            {
                vertex.normal = {
                    attrib.normals[3 * index.vertex_index + 0],
                    attrib.normals[3 * index.vertex_index + 1],
                    attrib.normals[3 * index.vertex_index + 2]
                };
            }

            if (index.texcoord_index >= 0)
            {
                vertex.uv = {
                    attrib.texcoords[2 * index.vertex_index + 0],
                    attrib.texcoords[2 * index.vertex_index + 1]
                };
            }

            vertices.push_back(vertex);
        }
    }
}

KongModel::KongModel(KongDevice& device, const Builder& builder)
    : m_kongDevice{device}
{
    createVertexBuffer(builder.vertices);
    createIndexBuffer(builder.indices);
}

KongModel::~KongModel()
{
    /*
     * buffer和memory分开，是由于分配内存是需要时间的，并且GPU有分配内存的操作数的上限（几千），在复杂场景下很容易遇到瓶颈
     * 所以正确的做法是请求大块内存并将内存的部分分配给不同资源。
     * 了解Vulkan memory allocator（VMA）库
     */
    // vkDestroyBuffer(m_kongDevice.device(), vertexBuffer, nullptr);
    // vkFreeMemory(m_kongDevice.device(), vertexBufferMemory, nullptr);
    //
    // if (hasIndexBuffer)
    // {
    //     vkDestroyBuffer(m_kongDevice.device(), indexBuffer, nullptr);
    //     vkFreeMemory(m_kongDevice.device(), indexBufferMemory, nullptr);
    //
    // }
}

void KongModel::draw(VkCommandBuffer commandBuffer)
{
    if (hasIndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);    
    }
}

std::unique_ptr<KongModel> KongModel::createModelFromFile(KongDevice& device, const std::string& filepath)
{
    Builder builder;
    builder.loadModel(filepath);

    std::cout << "vertex size:" << builder.vertices.size() << std::endl;
    return std::make_unique<KongModel>(device, builder);
}

void KongModel::bind(VkCommandBuffer commandBuffer)
{
    // 绑定command buffer和vertex buffer
    VkBuffer buffers[] = { vertexBuffer->getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (hasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void KongModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be greater than 3");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    KongBuffer staggingBuffer {
        m_kongDevice,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    staggingBuffer.map();
    staggingBuffer.writeToBuffer((void*)vertices.data());

    vertexBuffer = std::make_unique<KongBuffer>(
    m_kongDevice,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_kongDevice.copyBuffer(staggingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    
    // /*
    //  * staging buffer:
    //  * 原先使用的是buffer的property包含VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT，代表这段数据是cpu可以修改的
    //  * 但这对于GPU并不是最快的，性能最好的buffer需要开启VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT，这一般和HOST_VISIBLE_BIT不兼容
    //  * 因此使用staging buffer作为中间buffer，将CPU的数据传输到staging buffer，然后将staging buffer的数据拷贝到Device local的buffer上     
    //  */
    // VkBuffer stagingBuffer = VK_NULL_HANDLE;
    // VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    // /*
    //  * 映射host（CPU）和device（GPU）的memory
    //  * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT：表示memory数据能够在host上面可见（host写）
    //  * VK_MEMORY_PROPERTY_HOST_COHERENT_BIT：host和device的memory是连贯的
    //  */
    // m_kongDevice.createBuffer(bufferSize,
    //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,   // 告诉vulkan这个buffer的用处只是作为数据传输的来源
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //     stagingBuffer, stagingBufferMemory);

    // void* data;
    // // 指向host memory的地址
    // vkMapMemory(m_kongDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    // // 将顶点数据拷贝到host的分配内存中，由于前面设置了device和host的内存映射（COHERENT_BIT），顶点数据会自动同步到device的内存中
    // // 如果前面没有设置COHERENT_BIT，需要手动调用vkFlushMappedMemoryRanges来同步
    // memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    // vkUnmapMemory(m_kongDevice.device(), stagingBufferMemory);
    //
    // m_kongDevice.createBuffer(bufferSize,
    //     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,   // 作为vertex buffer，也作为数据传输的目标
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,    // 使用device local bit以获得更好的性能
    // vertexBuffer, vertexBufferMemory);
    //
    // // 将staging的数据传输到vertex buffer
    // m_kongDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    //    // 销毁staging buffer
    // vkDestroyBuffer(m_kongDevice.device(), stagingBuffer, nullptr);
    // vkFreeMemory(m_kongDevice.device(), stagingBufferMemory, nullptr);
}

void KongModel::createIndexBuffer(const std::vector<uint32_t>& indices)
{
    indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = indexCount > 0;

    if (!hasIndexBuffer)
    {
        return;
    }

    // // 和vertex buffer一样，index buffer也采用staging buffer
    // VkBuffer stagingBuffer = VK_NULL_HANDLE;
    // VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
    uint32_t indexSize = sizeof(indices[0]);
    KongBuffer staggingBuffer{
        m_kongDevice,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    staggingBuffer.map();
    staggingBuffer.writeToBuffer((void*)indices.data());
    
    
    // m_kongDevice.createBuffer(bufferSize,
    //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //     stagingBuffer, stagingBufferMemory);
    //
    // void* data;
    // // 指向host memory的地址
    // vkMapMemory(m_kongDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    // // 将顶点数据拷贝到host的分配内存中，由于前面设置了device和host的内存映射（COHERENT_BIT），顶点数据会自动同步到device的内存中
    // // 如果前面没有设置COHERENT_BIT，需要手动调用vkFlushMappedMemoryRanges来同步
    // memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    // vkUnmapMemory(m_kongDevice.device(), stagingBufferMemory);

    indexBuffer = std::make_unique<KongBuffer>(
        m_kongDevice,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //
    // m_kongDevice.createBuffer(bufferSize,
    //     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //     indexBuffer, indexBufferMemory);

    m_kongDevice.copyBuffer(staggingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    
    // vkDestroyBuffer(m_kongDevice.device(), stagingBuffer, nullptr);
    // vkFreeMemory(m_kongDevice.device(), stagingBufferMemory, nullptr);
}
