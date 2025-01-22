#pragma once
 
#include "kv_device.h"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace kong {
 
class KongDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(KongDevice &lveDevice) : lveDevice{lveDevice} {}
 
    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<KongDescriptorSetLayout> build() const;
 
   private:
    KongDevice &lveDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
 
  KongDescriptorSetLayout(
      KongDevice &lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~KongDescriptorSetLayout();
  KongDescriptorSetLayout(const KongDescriptorSetLayout &) = delete;
  KongDescriptorSetLayout &operator=(const KongDescriptorSetLayout &) = delete;
 
  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
 
 private:
  KongDevice &lveDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
 
  friend class KongDescriptorWriter;
};
 
class KongDescriptorPool {
 public:
  class Builder {
   public:
    Builder(KongDevice &lveDevice) : lveDevice{lveDevice} {}
 
    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<KongDescriptorPool> build() const;
 
   private:
    KongDevice &lveDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
 
  KongDescriptorPool(
      KongDevice &lveDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~KongDescriptorPool();
  KongDescriptorPool(const KongDescriptorPool &) = delete;
  KongDescriptorPool &operator=(const KongDescriptorPool &) = delete;
 
  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
 
  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
 
  void resetPool();
 
 private:
  KongDevice &lveDevice;
  VkDescriptorPool descriptorPool;
 
  friend class KongDescriptorWriter;
};
 
class KongDescriptorWriter {
 public:
  KongDescriptorWriter(KongDescriptorSetLayout &setLayout, KongDescriptorPool &pool);
 
  KongDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  KongDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
 
  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);
 
 private:
  KongDescriptorSetLayout &setLayout;
  KongDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
 
}  // namespace lve