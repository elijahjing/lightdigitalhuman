//
// Created by vincentsyan on 2025/8/18.
//

#include "Gltf.h"
#include <algorithm>
#include "GltfObject.h"
#include "GltfCamera.h"
#include "GltfAnimation.h"
#include "GltfMesh.h"
#include "GltfSkin.h"
#include "GltfAsset.h"
#include "GltfImage.h"
#include "GltfAccessor.h"
#include "GltfBuffer.h"
#include "GltfBufferView.h"
#include "GltfScene.h"
#include "GltfMaterial.h"
#include "GltfSampler.h"
namespace digitalhumans {
 int Gltf::addAccessor(std::shared_ptr<GltfAccessor> accessor) {
        if (!accessor) return -1;
        accessors.push_back(std::move(accessor));
        return static_cast<int>(accessors.size() - 1);
    }

    int Gltf::addNode(std::shared_ptr<GltfNode> node) {
        if (!node) return -1;
        nodes.push_back(std::move(node));
        return static_cast<int>(nodes.size() - 1);
    }

    int Gltf::addScene(std::shared_ptr<GltfScene> scene) {
        if (!scene) return -1;
        scenes.push_back(std::move(scene));
        return static_cast<int>(scenes.size() - 1);
    }

    int Gltf::addCamera(const std::shared_ptr<GltfCamera>& camera) {
        if (!camera) return -1;
        cameras.push_back(camera);
        return static_cast<int>(cameras.size() - 1);
    }

    int Gltf::addLight(const std::shared_ptr<GltfLight>& light) {
        if (!light) return -1;
        lights.push_back(light);
        return static_cast<int>(lights.size() - 1);
    }

    int Gltf::addImageBasedLight(const std::shared_ptr<ImageBasedLight>& imageBasedLight) {
        if (!imageBasedLight) return -1;
        imageBasedLights.push_back(imageBasedLight);
        return static_cast<int>(imageBasedLights.size() - 1);
    }

    int Gltf::addTexture(const std::shared_ptr<GltfTexture>& texture) {
        if (!texture) return -1;
        textures.push_back(texture);
        return static_cast<int>(textures.size() - 1);
    }

    int Gltf::addImage(const std::shared_ptr<GltfImage>& image) {
        if (!image) return -1;
        images.push_back(image);
        return static_cast<int>(images.size() - 1);
    }

    int Gltf::addSampler(const std::shared_ptr<GltfSampler>& sampler) {
        if (!sampler) return -1;
        samplers.push_back(sampler);
        return static_cast<int>(samplers.size() - 1);
    }

    int Gltf::addMesh(const std::shared_ptr<GltfMesh>& mesh) {
        if (!mesh) return -1;
        meshes.push_back(mesh);
        return static_cast<int>(meshes.size() - 1);
    }

    int Gltf::addBuffer(const std::shared_ptr<GltfBuffer>& buffer) {
        if (!buffer) return -1;
        buffers.push_back(buffer);
        return static_cast<int>(buffers.size() - 1);
    }

    int Gltf::addBufferView(const std::shared_ptr<GltfBufferView>& bufferView) {
        if (!bufferView) return -1;
        bufferViews.push_back(bufferView);
        return static_cast<int>(bufferViews.size() - 1);
    }

    int Gltf::addMaterial(const std::shared_ptr<GltfMaterial>& material) {
        if (!material) return -1;
        materials.push_back(material);
        return static_cast<int>(materials.size() - 1);
    }

    int Gltf::addAnimation(const std::shared_ptr<GltfAnimation>& animation) {
        if (!animation) return -1;
        animations.push_back(animation);
        return static_cast<int>(animations.size() - 1);
    }

    int Gltf::addSkin(const std::shared_ptr<GltfSkin>& skin) {
        if (!skin) return -1;
        skins.push_back(skin);
        return static_cast<int>(skins.size() - 1);
    }

    int Gltf::addVariant(const std::shared_ptr<GltfVariant>& variant) {
        if (!variant) return -1;
        variants.push_back(variant);
        return static_cast<int>(variants.size() - 1);
    }

    void Gltf::addAccessors(const std::vector<std::shared_ptr<GltfAccessor>>& accessorList) {
        accessors.insert(accessors.end(), accessorList.begin(), accessorList.end());
    }

    void Gltf::addNodes(const std::vector<std::shared_ptr<GltfNode>>& nodeList) {
        nodes.insert(nodes.end(), nodeList.begin(), nodeList.end());
    }

    void Gltf::addImages(const std::vector<std::shared_ptr<GltfImage>>& imageList) {
        images.insert(images.end(), imageList.begin(), imageList.end());
    }

    void Gltf::addSamplers(const std::vector<std::shared_ptr<GltfSampler>>& samplerList) {
        samplers.insert(samplers.end(), samplerList.begin(), samplerList.end());
    }

    int Gltf::findNodeByName(const std::string& name) const {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i] && nodes[i]->getName() == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int Gltf::findMaterialByName(const std::string& name) const {
        for (size_t i = 0; i < materials.size(); ++i) {
            if (materials[i] && materials[i]->getName() == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int Gltf::findAnimationByName(const std::string& name) const {
        for (size_t i = 0; i < animations.size(); ++i) {
            if (animations[i] && animations[i]->getName() == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    bool Gltf::isValidAccessorIndex(int index) const {
        return index >= 0 && index < static_cast<int>(accessors.size());
    }

    bool Gltf::isValidNodeIndex(int index) const {
        return index >= 0 && index < static_cast<int>(nodes.size());
    }

    bool Gltf::isValidImageIndex(int index) const {
        return index >= 0 && index < static_cast<int>(images.size());
    }

    bool Gltf::isValidSamplerIndex(int index) const {
        return index >= 0 && index < static_cast<int>(samplers.size());
    }

} // namespace digitalhumans
