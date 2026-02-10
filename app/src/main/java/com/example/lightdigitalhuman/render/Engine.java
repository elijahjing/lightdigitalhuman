package com.example.lightdigitalhuman.render;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.util.List;

public class Engine {
    private static final String TAG = "Engine";
    private long nativeEnginePtr = 0;
    private boolean isInitialized = false;

    static {
        System.loadLibrary("lightdigitalhuman");
    }

    public Engine(Context context) {
        nativeEnginePtr = nativeCreate();
        isInitialized = (nativeEnginePtr != 0);

        if (isInitialized) {
            Log.i(TAG, "Engine创建成功, ptr: " + Long.toHexString(nativeEnginePtr));
        } else {
            Log.e(TAG, "Engine创建失败");
        }
        if (isInitialized) {
            initializeOpenGLResources(context);
        }
    }

    public long getNativeEnginePtr() {
        return nativeEnginePtr;
    }

    public boolean isInitialized() {
        return isInitialized && nativeEnginePtr != 0;
    }


    public void destroy() {
        if (nativeEnginePtr != 0) {
            nativeDestroy(nativeEnginePtr);
            nativeEnginePtr = 0;
            isInitialized = false;
        }
    }

    public boolean loadModel(AssetManager assetManager, String patch) {
        if (nativeEnginePtr != 0) {
            return loadModel(nativeEnginePtr, assetManager, patch);
        }
        return false;
    }

    public boolean loadModelFromFile(String patch) {
        if (nativeEnginePtr != 0) {
            return loadFromFile(nativeEnginePtr, patch);
        }
        return false;
    }


    public void renderFrame(int width, int height) {
        if (nativeEnginePtr != 0) {
            renderFrame(nativeEnginePtr, width, height);
        }
    }

    public void setUserCamera(UserCamera camera) {
        setUserCamera(nativeEnginePtr, camera.getNativeCameraPtr());
    }

    public boolean loadEnvironmentFromAssets(String path, AssetManager assetManager) {
        if (nativeEnginePtr != 0) {
           return loadEnvironmentFromAssets(nativeEnginePtr, path, assetManager);
        }
        return false;
    }

    public boolean loadEnvironmentIblFromAssets(String path, AssetManager assetManager) {
        if (nativeEnginePtr != 0) {
            return nativeLoadEnvironmentIblFromAssets(nativeEnginePtr, path, assetManager);
        }
        return false;
    }

    // 析构函数确保资源清理
    @Override
    protected void finalize() throws Throwable {
        try {
            if (nativeEnginePtr != 0) {
                Log.w(TAG, "Engine未手动销毁，在finalize中清理");
                destroy();
            }
        } finally {
            super.finalize();
        }
    }


    public List<String> getAnimationAllName() {
        if (!isInitialized()) {
            Log.w(TAG, "⚠️ Engine未初始化");
            return null;
        }

        try {
            List<String> animationNames = nativeGetAnimationAllName(nativeEnginePtr);

            if (animationNames != null) {
                Log.i(TAG, " 获取到 " + animationNames.size() + " 个动画名称");
                for (int i = 0; i < animationNames.size(); i++) {
                    Log.d(TAG, String.format("   [%d]: %s", i, animationNames.get(i)));
                }
            } else {
                Log.w(TAG, " 获取动画名称为空");
            }

            return animationNames;

        } catch (Exception e) {
            Log.e(TAG, " 获取动画名称异常", e);
            return null;
        }
    }

    public void playAnimation(String animationName, int time) {
        if (!isInitialized()) {
            Log.w(TAG, "⚠️ Engine未初始化");
        }

        try {
            nativePlayAnimation(nativeEnginePtr, animationName, time);
        } catch (Exception e) {
            Log.e(TAG, " 播放动画异常", e);
        }
    }

    public void stopAnimation(String animationName) {
        if (!isInitialized()) {
            Log.w(TAG, "⚠️ Engine未初始化");
        }

        try {
            nativeStopAnimation(nativeEnginePtr, animationName);
        } catch (Exception e) {
            Log.e(TAG, " 播放动画异常", e);
        }
    }


    public void  setIbl(boolean enable) {
        if (!isInitialized()) {
            Log.w(TAG, "⚠️ Engine未初始化");
        }

        try {
            nativeSetIbl(nativeEnginePtr, enable);
        } catch (Exception e) {
            Log.e(TAG, " 设置IBL异常", e);
        }
    }

    // ==================== Native方法声明 ====================
    private native long nativeCreate();

    private native void renderFrame(long nativeEnginePtr, int width, int height);

    private native void initializeOpenGLResources(Context context);

    private native boolean loadModel(long nativeEnginePtr, AssetManager assetManager, String patch);

    private native boolean loadFromFile(long nativeEnginePtr, String filePath);

    private native void nativeDestroy(long enginePtr);

    private native void setUserCamera(long enginePtr, long cameraDataPtr);

    private native List<String> nativeGetAnimationAllName(long enginePtr);

    private native void nativePlayAnimation(long enginePtr, String animationName, int time);

    private native void nativeStopAnimation(long enginePtr, String animationName);

    private native boolean loadEnvironmentFromAssets(long enginePtr, String env_path, AssetManager assetManager);

    private native boolean nativeLoadEnvironmentIblFromAssets(long enginePtr, String env_path, AssetManager assetManager);

    private native boolean nativeSetIbl(long enginePtr, boolean enable);

}