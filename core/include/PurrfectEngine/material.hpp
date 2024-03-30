#ifndef   PURRENGINE_MATERIAL_HPP_
#define   PURRENGINE_MATERIAL_HPP_

#include "PurrfectEngine/texture.hpp"

namespace PurrfectEngine {
    
  vkDescriptorLayout *getMaterialLayout(vkRenderer *renderer = nullptr);

  class vkMaterial {
  public:
    vkMaterial(vkRenderer *renderer);
    vkMaterial(vkRenderer *renderer, vkTexture *albedo);
    ~vkMaterial();

    void initialize(vkDescriptorPool *pool);
    void recreate(vkDescriptorPool *pool);
    void cleanup();

    void setAlbedo(vkTexture *albedo)                     { mAlbedo           = albedo; }
    void setNormal(vkTexture *normal)                     { mNormal           = normal; }
    void setMetallic(vkTexture *metallic)                 { mMetallic          = metallic; }
    void setRoughness(vkTexture *roughness)               { mRoughness        = roughness; }
    void setAmbientOcclusion(vkTexture *ambientOcclusion) { mAmbientOcclusion = ambientOcclusion; }

    void bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set = 1);
  private:
    vkRenderer *mRenderer = nullptr;

    // NOTE(CatDev): Has to be set less (`set` = false)
    vkTexture  *mAlbedo           = nullptr;
    // NOTE(CatDev): Has to be set less (`set` = false)
    vkTexture  *mNormal           = nullptr;
    // NOTE(CatDev): Has to be set less (`set` = false)
    vkTexture  *mMetallic          = nullptr;
    // NOTE(CatDev): Has to be set less (`set` = false)
    vkTexture  *mRoughness        = nullptr;
    // NOTE(CatDev): Has to be set less (`set` = false)
    vkTexture  *mAmbientOcclusion = nullptr;

    vkDescriptorSet *mSet = nullptr;
  };

}

#endif // PURRENGINE_MATERIAL_HPP_