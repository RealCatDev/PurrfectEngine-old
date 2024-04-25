#ifndef PURRENGINE_PANELS_HPP_
#define PURRENGINE_PANELS_HPP_

#include <imgui.h>
#include <filesystem>
#include <iostream>
#include <PurrfectEngine/application.hpp>

namespace PurrfectEngine
{
  struct FileNode {
    std::string name;
    std::filesystem::path path;
    bool folder;
    std::vector<FileNode> children;
  };

  // MARK: Hierarchy
  class HierarchyPanel
  {
  public:
    static void Render()
    {
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
  static void Render() {
    ImGui::Begin("FileManager", &mOpen);

    for (const auto& node : mRootNode.children) {
      RenderNode(node);
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

  static void RenderNode(FileNode node) {
    if (node.folder) {
      if (ImGui::TreeNode(node.name.c_str())) {
        for (const auto& child : node.children) {
          
          RenderNode(child);
        }
        ImGui::TreePop();
      }
    } else {
      ImGui::BulletText("%s", node.name.c_str());
    }
  }


  inline static FileNode mRootNode; // Virtual root node for folder tree
  inline static bool mOpen = true;
};
}

#endif