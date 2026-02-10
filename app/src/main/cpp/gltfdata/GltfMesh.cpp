//
// Created by vincentsyan on 2025/8/18.
//

#include "GltfMesh.h"
#include "../utils/utils.h"
#include <stdexcept>

namespace digitalhumans {

GltfMesh::GltfMesh()
    : GltfObject(), primitives(), name(""), weights() {
}

void GltfMesh::addPrimitive(std::shared_ptr<GltfPrimitive> primitive) {
  if (primitive) {
    primitives.push_back(primitive);
  }
}

} // namespace digitalhumans