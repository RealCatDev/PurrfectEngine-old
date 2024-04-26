#ifndef PURRENGINE_PANELS_HPP_
#define PURRENGINE_PANELS_HPP_

#include <imgui.h>
#include <filesystem>
#include <iostream>
#include <vulkan/vulkan.h>
#include <PurrfectEngine/application.hpp>
#include <PurrfectEngine/texture.hpp>
#include <PurrfectEngine/utils.hpp>

namespace PurrfectEngine {
  struct FileNode {
    std::string name;
    std::filesystem::path path;
    bool folder;
    std::vector<FileNode> children;
  };

  // MARK: Hierarchy
  class HierarchyPanel {
  public:
    static void Render() {
      ImGui::Begin("Hierarchy", &mOpen);
      // Render your hierarchy here
      ImGui::End();
    }
  private:
    inline static bool mOpen = true;
  };

  // MARK: FileView
  class FileManagerPanel {
  public:
    static void Initialize(vkRenderer *renderer, vkCommandPool *commandPool, vkDescriptorPool *descriptorPool)
      sTexture = new vkTexture(renderer, Asset("textures/texture.png"));
      sTexture->initialize(commandPool, descriptorPool, mSwapchain->getFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    
    static void Cleanup() {
      delete sTexture;
    }
    
    static void Render() {
      ImGui::Begin("FileManager", &mOpen);
      ImTextureID texId = (ImTextureID)sTexture.getSet();
      for (const auto& node : mRootNode.children) {
        RenderNode(node, texId, texId);
      }

      ImGui::End();
    }

    static void Refresh() {
      mRootNode.name = "<VIRTUAL_ROOT>";
      mRootNode.folder = true;
      mRootNode.children.clear();

      const std::filesystem::path projDir(application::getInstance()->getProjDir());

      for (auto const &entry : std::filesystem::directory_iterator(projDir)) {
        if (entry.is_directory()) {
          AddFileNode(mRootNode, entry);
        } else if (entry.is_regular_file()) {
          FileNode node{};
          node.name = entry.path().filename().string();
          node.path = entry.path();
          node.folder = false;
          node.children = {};

          mRootNode.children.push_back(node);

          // std::string out = std::string(node.name) + " : " + std::string(node.path);
          // fprintf(stderr, "%s\n", out.c_str());
        }
      }
    }
  private:
    static void AddFileNode(FileNode parent, std::filesystem::directory_entry entry) {
      FileNode node{};
      node.name = entry.path().filename().string();
      node.path = entry.path();
      node.folder = std::filesystem::is_directory(entry.path());
      node.children = {};

      if (node.folder) {
        for (auto const &entry : std::filesystem::directory_iterator(node.path)) {
          AddFileNode(node, entry);
        }
      }

      parent.children.push_back(node);
    }

    static void RenderNode(FileNode node, ImTextureID folderOpenTextureID, ImTextureID folderClosedTextureID) {
      if (node.folder) {
        bool nodeOpen = ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
        if (ImGui::IsItemClicked()) {
          nodeOpen = !nodeOpen;
        }

        if (nodeOpen) {
          for (const auto& child : node.children) {
            RenderNode(child, folderOpenTextureID, folderClosedTextureID);
          }
          ImGui::TreePop();
        }

        ImGui::SameLine();
        ImGui::Image(nodeOpen ? folderOpenTextureID : folderClosedTextureID, ImVec2(16, 16));
      } else {
        ImGui::BulletText("%s", node.name.c_str());
      }
    }
  private:
    static FileNode mRootNode; // Virtual root node for folder tree
    inline static bool mOpen = true;

    inline static vkTexture *sTexture = nullptr;
  };
}

#endif