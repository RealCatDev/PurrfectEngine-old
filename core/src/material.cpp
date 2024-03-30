#include "PurrfectEngine/material.hpp"
#include "PurrfectEngine/renderer.hpp"

namespace PurrfectEngine {

  static vkDescriptorLayout *sMaterialLayout = nullptr;

  vkDescriptorLayout *getMaterialLayout(vkRenderer *renderer) {
    if (!sMaterialLayout) {
      PURR_ASSERT(renderer);
      sMaterialLayout = new vkDescriptorLayout(renderer);
      sMaterialLayout->addBinding({
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      sMaterialLayout->addBinding({
        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      sMaterialLayout->addBinding({
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      sMaterialLayout->addBinding({
        3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      sMaterialLayout->addBinding({
        4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      sMaterialLayout->initialize();
    }
    PURR_ASSERT(sMaterialLayout);
    return sMaterialLayout;
  }

  vkMaterial::vkMaterial(vkRenderer *renderer):
    mRenderer(renderer) {
  }

  vkMaterial::vkMaterial(vkRenderer *renderer, vkTexture *albedo):
    mRenderer(renderer), mAlbedo(albedo) {
  }
  
  vkMaterial::~vkMaterial() {
    cleanup();
  }

  void vkMaterial::initialize(vkDescriptorPool *pool) {
    mSet = pool->allocate(getMaterialLayout());
    PURR_ASSERT(mAlbedo && mNormal && mMetallic && mRoughness && mAmbientOcclusion);
    mSet->write(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mAlbedo->getView(),           mAlbedo->getSampler(),           0);
    mSet->write(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mNormal->getView(),           mNormal->getSampler(),           1);
    mSet->write(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mMetallic->getView(),         mMetallic->getSampler(),         2);
    mSet->write(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mRoughness->getView(),        mRoughness->getSampler(),        3);
    mSet->write(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mAmbientOcclusion->getView(), mAmbientOcclusion->getSampler(), 4);
  }

  void vkMaterial::recreate(vkDescriptorPool *pool) {
    cleanup();
    initialize(pool);
  }

  void vkMaterial::cleanup() {
    if (mSet) delete mSet;
  }

  void vkMaterial::bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set) {
    mSet->bind(cmdBuf, pipeline, set);
  }

}