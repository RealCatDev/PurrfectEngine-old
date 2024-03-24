#include "PurrfectEngine/loaders.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs)

namespace PurrfectEngine {

  modelLoader::modelLoader(vkRenderer *renderer):
    mRenderer(renderer)
  { sInstance = this; }

  modelLoader::~modelLoader() {

  }

  vkMesh *modelLoader::load(const char *filename, vkCommandPool *cmdPool) {
    auto dis = sInstance;

    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(filename, ASSIMP_LOAD_FLAGS);
    if (!scene) return nullptr;
    vkMesh *mesh = new vkMesh(dis->mRenderer);
    {
      uint32_t vertexCount = 0;
      uint32_t indexCount  = 0;

      std::vector<MeshVertex> vertices{};
      std::vector<uint32_t> indices{};

      for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh *pMesh = scene->mMeshes[i];
        vkSubMesh smesh = {};
        smesh.NumIndices = pMesh->mNumFaces * 3;
        smesh.Vertex     = vertexCount;  vertexCount += pMesh->mNumVertices;
        smesh.Index      = indexCount;   indexCount  += smesh.NumIndices;
        mesh->addSubMesh(smesh);

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        for (unsigned int i = 0; i < pMesh->mNumVertices; i++) {
          MeshVertex vert{};
          const aiVector3D& pPos = pMesh->mVertices[i];
          vert.position = glm::vec3(pPos.x, pPos.y, pPos.z);

          if (pMesh->mNormals) {
            const aiVector3D& pNormal = pMesh->mNormals[i];
            vert.normal = glm::vec3(pNormal.x, pNormal.y, pNormal.z);
          } else {
            vert.normal = glm::vec3(0.0f, 1.0f, 0.0f);
          }

          const aiVector3D& uv = pMesh->HasTextureCoords(0) ? pMesh->mTextureCoords[0][i] : Zero3D;
          vert.uv = glm::vec2(uv.x, uv.y);

          vert.color = glm::vec3(1.0f);

          vertices.push_back(vert);
        }

        for (unsigned int i = 0; i < pMesh->mNumFaces; i++) {
          const aiFace& face = pMesh->mFaces[i];
          indices.push_back(face.mIndices[0]);
          indices.push_back(face.mIndices[1]);
          indices.push_back(face.mIndices[2]);
        }
      }

      mesh->setVertices(vertices);
      mesh->setIndices(indices);
    }
    mesh->initialize(cmdPool);

    return mesh;
  }

  void modelLoader::save(const char *filename) {

  }

}