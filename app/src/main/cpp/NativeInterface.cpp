//
// Created by vincentsyan on 2025/8/11.
//

#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/asset_manager_jni.h>
#include "utils/LogUtils.h"
#include "engine/Engine.h"
#include "gltfdata/GltfRenderer.h"
#include "gltfdata/converter/GltfLoader.h"
#include "gltfdata/converter/ShaderManager.h"
#include "gltfdata/ibl/HDRImageLoader.h"
#include "gltfdata/ibl_sampler.h"


namespace digitalhumans {
static GltfLoader loader;

static std::unordered_map<jlong, std::shared_ptr<UserCamera>> g_camera_map;
static std::mutex g_camera_mutex;

jlong createCamera();

std::shared_ptr<UserCamera> getCameraShared(jlong camera_ptr);

jlong createCamera() {
  auto camera = std::make_shared<UserCamera>();

  std::lock_guard<std::mutex> lock(g_camera_mutex);
  jlong ptr = reinterpret_cast<jlong>(camera.get());
  g_camera_map[ptr] = camera;

  return ptr;
}

std::shared_ptr<UserCamera> getCameraShared(jlong camera_ptr) {
  std::lock_guard<std::mutex> lock(g_camera_mutex);
  auto it = g_camera_map.find(camera_ptr);
  return (it != g_camera_map.end()) ? it->second : nullptr;
}

std::string readGLSLFile(JNIEnv *env, jobject context, const char *filename);


void loadShaders(JNIEnv *env, jobject thiz);

Engine *getEngine(jlong enginePtr);

Engine *getEngine(jlong enginePtr);

void loadShaders(JNIEnv *env, jobject thiz) {

  auto animation = readGLSLFile(env, thiz, "pbrshader/animation.glsl");
  auto brdf = readGLSLFile(env, thiz, "pbrshader/brdf.glsl");
  auto cubemap = readGLSLFile(env, thiz, "pbrshader/cubemap.frag");
  auto cubemap_vert = readGLSLFile(env, thiz, "pbrshader/cubemap.vert");

  auto functions = readGLSLFile(env, thiz, "pbrshader/functions.glsl");
  auto ibl = readGLSLFile(env, thiz, "pbrshader/ibl.glsl");
  auto iridescence = readGLSLFile(env, thiz, "pbrshader/iridescence.glsl");

  auto material_info = readGLSLFile(env, thiz, "pbrshader/material_info.glsl");
  auto pbr = readGLSLFile(env, thiz, "pbrshader/pbr.frag");
  auto primitive = readGLSLFile(env, thiz, "pbrshader/primitive.vert");
  auto punctual = readGLSLFile(env, thiz, "pbrshader/punctual.glsl");
  auto specular_glossiness =
      readGLSLFile(env, thiz, "pbrshader/specular_glossiness.frag");
  auto textures = readGLSLFile(env, thiz, "pbrshader/textures.glsl");
  auto tonemapping = readGLSLFile(env, thiz, "pbrshader/tonemapping.glsl");
  auto fullscreen = readGLSLFile(env, thiz, "iblshader/fullscreen.vert");
  auto panorama_to_cubemap =
      readGLSLFile(env, thiz, "iblshader/panorama_to_cubemap.frag");
  auto ibl_filtering = readGLSLFile(env, thiz, "iblshader/ibl_filtering.frag");
  auto debug = readGLSLFile(env, thiz, "iblshader/debug.frag");
  GLSLStringFiles glslStringShaders;
  glslStringShaders.animation = animation;
  glslStringShaders.brdf = brdf;
  glslStringShaders.cubemap = cubemap;
  glslStringShaders.cubemap_vert = cubemap_vert;
  glslStringShaders.functions = functions;
  glslStringShaders.ibl = ibl;
  glslStringShaders.iridescence = iridescence;
  glslStringShaders.material_info = material_info;
  glslStringShaders.pbr = pbr;
  glslStringShaders.primitive = primitive;
  glslStringShaders.punctual = punctual;
  glslStringShaders.specular_glossiness = specular_glossiness;
  glslStringShaders.textures = textures;
  glslStringShaders.tonemapping = tonemapping;
  glslStringShaders.fullscreen = fullscreen;
  glslStringShaders.panorama_to_cubemap = panorama_to_cubemap;
  glslStringShaders.ibl_filtering = ibl_filtering;
  glslStringShaders.debug = debug;
  ShaderManager::getInstance().setShaderFiles(glslStringShaders);
}


std::string readGLSLFile(JNIEnv *env, jobject context, const char *filename) {
  jclass contextClass = env->GetObjectClass(context);
  jmethodID getAssetsMethod = env->GetMethodID(contextClass,
                                               "getAssets",
                                               "()Landroid/content/res/AssetManager;");
  jobject assetManagerObj = env->CallObjectMethod(context, getAssetsMethod);
  AAssetManager *assetManager = AAssetManager_fromJava(env, assetManagerObj);
  AAsset
      *asset = AAssetManager_open(assetManager, filename, AASSET_MODE_UNKNOWN);
  if (!asset) return "";
  size_t size = AAsset_getLength(asset);
  std::string content(size, '\0');
  AAsset_read(asset, &content[0], size);
  AAsset_close(asset);
  return content;
}

Engine *getEngine(jlong enginePtr) {
  if (enginePtr == 0) {
    return nullptr;
  }
  return reinterpret_cast<Engine *>(enginePtr);
}


UserCamera *getCamera(jlong userCameraPtr) {
  auto camera = getCameraShared(userCameraPtr);
  return camera.get();
}

}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativeCreate(JNIEnv *env,
                                                                     jobject thiz) {
  try {
    // 创建Engine对象
    auto *engine = new digitalhumans::Engine();
    auto enginePtr = reinterpret_cast<jlong>(engine);
    return enginePtr;

  } catch (const std::exception &e) {
    return 0;
  } catch (...) {
    return 0;
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativeDestroy(JNIEnv *env,
                                                                      jobject thiz,
                                                                      jlong engine_ptr) {
  auto *engine = digitalhumans::getEngine(engine_ptr);
  if (engine) {
    delete engine;
    digitalhumans::loader.clearModelCache();
  }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_Engine_renderFrame(JNIEnv *env,
                                                                    jobject thiz,
                                                                    jlong engine_ptr,
                                                                    jint
                                                                    width,
                                                                    jint height) {
  auto *engine = digitalhumans::getEngine(engine_ptr);
  if (engine) {
    engine->renderFrame(width, height);
  }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_lightdigitalhuman_render_Engine_loadModel(JNIEnv *env,
                                                                  jobject thiz,
                                                                  jlong engine_ptr,
                                                                  jobject asset_manager,
                                                                  jstring patch) {
  AAssetManager *assetManager = AAssetManager_fromJava(env, asset_manager);
  if (!assetManager) {
    LOGE("Failed to get native AssetManager from Java object");
    return JNI_FALSE;  // 使用JNI_FALSE而不是false
  }

  auto *engine = digitalhumans::getEngine(engine_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
    return JNI_FALSE;
  }

  const char *filenameStr = env->GetStringUTFChars(patch, nullptr);
  if (!filenameStr) {
    LOGE("Failed to get filename string");
    return JNI_FALSE;
  }

  // 4. 执行加载（使用RAII确保资源释放）
  bool success = false;
  try {
    LOGI("📁 Loading GLTF file: %s", filenameStr);
    success = digitalhumans::loader.loadGltfFromAssets(assetManager,
                                                       filenameStr,
                                                       *engine);

    if (success) {
      LOGI("Model loaded successfully: %s", filenameStr);
    } else {
      LOGE("Failed to load model: %s", filenameStr);
    }

  } catch (const std::exception &e) {
    LOGE("Exception during model loading: %s", e.what());
    success = false;
  } catch (...) {
    LOGE("Unknown exception during model loading");
    success = false;
  }
  env->ReleaseStringUTFChars(patch, filenameStr);
  return success ? JNI_TRUE : JNI_FALSE;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_Engine_initializeOpenGLResources(
    JNIEnv *env,
    jobject thiz,
    jobject context) {
  digitalhumans::loadShaders(env, context);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeCreate(JNIEnv *env,
                                                                         jobject thiz) {
  try {
    // 创建Engine对象
    return digitalhumans::createCamera();
  } catch (...) {
    LOGE("pUserCamera error");
    return 0;
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeDestroy(JNIEnv *env,
                                                                          jobject thiz,
                                                                          jlong camera_ptr) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    delete engine;
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeSetVerticalFoV(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat fov) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->setVerticalFoV(fov);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeLookAt(JNIEnv *env,
                                                                         jobject thiz,
                                                                         jlong camera_ptr,
                                                                         jfloat from_x,
                                                                         jfloat from_y,
                                                                         jfloat from_z,
                                                                         jfloat to_x,
                                                                         jfloat to_y,
                                                                         jfloat to_z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    const glm::vec3 from(from_x, from_y, from_z);
    const glm::vec3 to(to_x, to_y, to_z);
    engine->lookAt(from, to);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeSetPosition(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat x,
    jfloat y,
    jfloat z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    const glm::vec3 position(x, y, z);
    engine->setPosition(position);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeSetTarget(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat x,
    jfloat y,
    jfloat z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    const glm::vec3 target(x, y, z);
    engine->setTarget(target);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeSetRotation(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat yaw,
    jfloat pitch) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->setRotation(yaw, pitch);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeSetDistanceFromTarget(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat distance,
    jfloat target_x,
    jfloat target_y,
    jfloat target_z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    const glm::vec3 target(target_x, target_y, target_z);
    engine->setDistanceFromTarget(distance, target);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeZoomBy(JNIEnv *env,
                                                                         jobject thiz,
                                                                         jlong camera_ptr,
                                                                         jfloat value) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->zoomBy(value);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeOrbit(JNIEnv *env,
                                                                        jobject thiz,
                                                                        jlong camera_ptr,
                                                                        jfloat x,
                                                                        jfloat y) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->orbit(x, y);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativePan(JNIEnv *env,
                                                                      jobject thiz,
                                                                      jlong camera_ptr,
                                                                      jfloat x,
                                                                      jfloat y) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->pan(x, y);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeFitPanSpeedToScene(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat min_x,
    jfloat min_y,
    jfloat min_z,
    jfloat max_x,
    jfloat max_y,
    jfloat max_z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    const glm::vec3 min(min_x, min_y, min_z);
    const glm::vec3 max(max_x, max_y, max_z);
    engine->fitPanSpeedToScene(min, max);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeReset(JNIEnv *env,
                                                                        jobject thiz,
                                                                        jlong camera_ptr) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->reset();
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeResetView(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jlong gltf_ptr,
    jint scene_index) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    auto *mainEngine = digitalhumans::getEngine(gltf_ptr);
    if (!mainEngine) {
      LOGE("Invalid engine pointer: %lld", static_cast<long long>(gltf_ptr));
      return;
    }
    auto gltf = mainEngine->state->getGltf();
    engine->resetView(gltf, scene_index);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeFitViewToScene(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jlong gltf_ptr,
    jint scene_index) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    auto *mainEngine = digitalhumans::getEngine(gltf_ptr);
    if (!mainEngine) {
      LOGE("Invalid engine pointer: %lld", static_cast<long long>(gltf_ptr));
      return;
    }
    auto gltf = mainEngine->state->getGltf();
    engine->fitViewToScene(gltf, scene_index);
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeFitDistanceToExtents(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat min_x,
    jfloat min_y,
    jfloat min_z,
    jfloat max_x,
    jfloat max_y,
    jfloat max_z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->fitDistanceToExtents({min_x, min_y, min_z}, {max_x, max_y, max_z});
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeFitCameraTargetToExtents(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat min_x,
    jfloat min_y,
    jfloat min_z,
    jfloat max_x,
    jfloat max_y,
    jfloat max_z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->fitCameraTargetToExtents({min_x, min_y, min_z},
                                     {max_x, max_y, max_z});
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_UserCamera_nativeFitCameraPlanesToExtents(
    JNIEnv *env,
    jobject thiz,
    jlong camera_ptr,
    jfloat min_x,
    jfloat min_y,
    jfloat min_z,
    jfloat max_x,
    jfloat max_y,
    jfloat max_z) {
  auto *engine = digitalhumans::getCamera(camera_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(camera_ptr));
    return;
  } else {
    engine->fitCameraPlanesToExtents({min_x, min_y, min_z},
                                     {max_x, max_y, max_z});
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_Engine_setUserCamera(JNIEnv *env,
                                                                      jobject thiz,
                                                                      jlong engine_ptr,
                                                                      jlong camera_data_ptr) {
  auto camera = digitalhumans::getCameraShared(camera_data_ptr);
  if (!camera) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
    return;
  } else {
    auto *mainEngine = digitalhumans::getEngine(engine_ptr);
    if (!mainEngine) {
      LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
      return;
    }
    mainEngine->setUserCamera(camera);
  }
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativeGetAnimationAllName(
    JNIEnv *env,
    jobject thiz,
    jlong engine_ptr) {


  try {
    auto *mainEngine = digitalhumans::getEngine(engine_ptr);
    if (!mainEngine) {
      LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
      return nullptr;
    }
    std::vector<std::string> animationNames = mainEngine->getAnimationAllName();
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    if (arrayListClass == nullptr) {
      return nullptr;
    }

    jmethodID arrayListConstructor =
        env->GetMethodID(arrayListClass, "<init>", "(I)V");
    if (arrayListConstructor == nullptr) {
      return nullptr;
    }

    jmethodID addMethod =
        env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    if (addMethod == nullptr) {
      return nullptr;
    }
    jobject arrayList = env->NewObject(arrayListClass, arrayListConstructor,
                                       static_cast<jint>(animationNames.size()));
    if (arrayList == nullptr) {
      return nullptr;
    }
    for (const auto &name: animationNames) {
      jstring javaString = env->NewStringUTF(name.c_str());
      if (javaString == nullptr) {
        return nullptr;
      }

      env->CallBooleanMethod(arrayList, addMethod, javaString);
      env->DeleteLocalRef(javaString);
    }
    env->DeleteLocalRef(arrayListClass);
    return arrayList;
  } catch (const std::exception &e) {
    LOGE("Error: %s", e.what());
    return nullptr;
  }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativePlayAnimation(
    JNIEnv *env,
    jobject thiz,
    jlong engine_ptr,
    jstring animation_name,
    jint time) {

  auto *mainEngine = digitalhumans::getEngine(engine_ptr);
  if (!mainEngine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
  }
  mainEngine->playAnimation(env->GetStringUTFChars(animation_name, nullptr),
                            time);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativeStopAnimation(
    JNIEnv *env,
    jobject thiz,
    jlong engine_ptr,
    jstring animation_name) {
  auto *mainEngine = digitalhumans::getEngine(engine_ptr);
  if (!mainEngine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
  }
  mainEngine->stopAnimation(env->GetStringUTFChars(animation_name, nullptr));
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_lightdigitalhuman_render_Engine_loadEnvironmentFromAssets(
    JNIEnv *env,
    jobject thiz,
    jlong engine_ptr,
    jstring env_path,
    jobject asset_manager) {
  if (!asset_manager) {
    LOGE("Environment or AssetManager not initialized");
    return JNI_FALSE;
  }
  auto *mainEngine = digitalhumans::getEngine(engine_ptr);
  if (!mainEngine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
    return JNI_FALSE;
  }

  const char *envPathStr = env->GetStringUTFChars(env_path, nullptr);
  std::string envPath(envPathStr);
  env->ReleaseStringUTFChars(env_path, envPathStr);
//  mainEngine->getState()->getEnvironment()->setAssetManager(
//      AAssetManager_fromJava(env, asset_manager));
  bool success = false;
//      mainEngine->getState()->getEnvironment()->loadEnvironmentFromAssets(
//          envPath);
  return success ? JNI_TRUE : JNI_FALSE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativeLoadEnvironmentIblFromAssets(
    JNIEnv *env,
    jobject thiz,
    jlong engine_ptr,
    jstring hdr_path,
    jobject asset_manager) {

  AAssetManager
      *nativeAssetManager = AAssetManager_fromJava(env, asset_manager);
  digitalhumans::HDRImageLoader::setAssetManager(nativeAssetManager);

  const char *path = env->GetStringUTFChars(hdr_path, nullptr);
  auto hdrImage =
      digitalhumans::HDRImageLoader::loadFromAssets(std::string(path));

  env->ReleaseStringUTFChars(hdr_path, path);
  auto *mainEngine = digitalhumans::getEngine(engine_ptr);
  if (!mainEngine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
    return JNI_FALSE;
  } else {
    return mainEngine->processEnvironmentMap(hdrImage);
  }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_lightdigitalhuman_render_Engine_nativeSetIbl(JNIEnv *env,
                                                                     jobject thiz,
                                                                     jlong engine_ptr,
                                                                     jboolean enable) {
  auto *mainEngine = digitalhumans::getEngine(engine_ptr);
  if (!mainEngine) {
    LOGE("Invalid engine pointer: %lld", static_cast<long long>(engine_ptr));
    return JNI_FALSE;
  } else {
    mainEngine->setIbL(enable);
    return JNI_TRUE;
  }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_lightdigitalhuman_render_Engine_loadFromFile(JNIEnv *env,
                                                                     jobject thiz,
                                                                     jlong native_engine_ptr,
                                                                     jstring file_path) {


  auto *engine = digitalhumans::getEngine(native_engine_ptr);
  if (!engine) {
    LOGE("Invalid engine pointer: %lld",
         static_cast<long long>(native_engine_ptr));
    return JNI_FALSE;
  }

  const char *filenameStr = env->GetStringUTFChars(file_path, nullptr);
  if (!filenameStr) {
    LOGE("Failed to get filename string");
    return JNI_FALSE;
  }

  bool success = false;
  try {
    success = digitalhumans::loader.loadFromFile(filenameStr, *engine);
  } catch (const std::exception &e) {
    LOGE("Exception during model loading: %s", e.what());
    success = false;
  } catch (...) {
    LOGE("Unknown exception during model loading");
    success = false;
  }
  env->ReleaseStringUTFChars(file_path, filenameStr);
  return success ? JNI_TRUE : JNI_FALSE;
}