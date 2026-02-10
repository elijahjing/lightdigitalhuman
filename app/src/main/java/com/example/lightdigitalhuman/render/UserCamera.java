package com.example.lightdigitalhuman.render;


import android.util.Log;


/**
 * 用户摄像机类
 * 对应C++ UserCamera类
 */
public class UserCamera {
    private static final String TAG = "UserCamera";

    // 常量定义（对应C++常量）
    public static final float PAN_SPEED_DENOMINATOR = 3500.0f;
    public static final float MAX_NEAR_FAR_RATIO = 10000.0f;

    private long nativeCameraPtr = 0;
    private boolean isInitialized = false;

    public long getNativeCameraPtr() {
        return nativeCameraPtr;
    }

    /**
     * 构造函数
     */
    public UserCamera() {
        nativeCameraPtr = nativeCreate();
        isInitialized = (nativeCameraPtr != 0);

        if (isInitialized) {
            Log.i(TAG, "UserCamera创建成功");
        } else {
            Log.e(TAG, "UserCamera创建失败");
        }
    }

    /**
     * 检查是否初始化
     */
    public boolean isInitialized() {
        return isInitialized && nativeCameraPtr != 0;
    }

    // ==================== 基本操作方法 ====================

    /**
     * 设置垂直视场角
     *
     * @param fov 视场角（度数）
     */
    public void setVerticalFoV(float fov) {
        if (isInitialized()) {
            nativeSetVerticalFoV(nativeCameraPtr, (float) Math.toRadians(fov));
        }
    }


    /**
     * 设置摄像机从from位置看向to位置
     *
     * @param fromX, fromY, fromZ 摄像机位置
     * @param toX,   toY, toZ 目标位置
     */
    public void lookAt(float fromX, float fromY, float fromZ, float toX, float toY, float toZ) {
        if (isInitialized()) {
            nativeLookAt(nativeCameraPtr, fromX, fromY, fromZ, toX, toY, toZ);
        }
    }

    /**
     * 便捷的lookAt方法
     */
    public void lookAt(float[] from, float[] to) {
        if (from.length >= 3 && to.length >= 3) {
            lookAt(from[0], from[1], from[2], to[0], to[1], to[2]);
        }
    }

    /**
     * 设置摄像机位置
     */
    public void setPosition(float x, float y, float z) {
        if (isInitialized()) {
            nativeSetPosition(nativeCameraPtr, x, y, z);
        }
    }

    public void setPosition(float[] position) {
        if (position.length >= 3) {
            setPosition(position[0], position[1], position[2]);
        }
    }

    /**
     * 设置目标位置
     */
    public void setTarget(float x, float y, float z) {
        if (isInitialized()) {
            nativeSetTarget(nativeCameraPtr, x, y, z);
        }
    }

    public void setTarget(float[] target) {
        if (target.length >= 3) {
            setTarget(target[0], target[1], target[2]);
        }
    }

    /**
     * 设置旋转（欧拉角，度数）
     *
     * @param yaw   偏航角（度）
     * @param pitch 俯仰角（度）
     */
    public void setRotation(float yaw, float pitch) {
        if (isInitialized()) {
            nativeSetRotation(nativeCameraPtr, yaw, pitch);
        }
    }

    /**
     * 设置距离目标的距离
     */
    public void setDistanceFromTarget(float distance, float targetX, float targetY, float targetZ) {
        if (isInitialized()) {
            nativeSetDistanceFromTarget(nativeCameraPtr, distance, targetX, targetY, targetZ);
        }
    }

    public void setDistanceFromTarget(float distance, float[] target) {
        if (target.length >= 3) {
            setDistanceFromTarget(distance, target[0], target[1], target[2]);
        }
    }

    // ==================== 交互操作方法 ====================

    /**
     * 缩放
     *
     * @param value 缩放值（推荐范围[-1,1]）
     */
    public void zoomBy(float value) {
        if (isInitialized()) {
            nativeZoomBy(nativeCameraPtr, value);
        }
    }

    /**
     * 轨道运动（绕目标点旋转）
     *
     * @param x x轴旋转量（弧度）
     * @param y y轴旋转量（弧度）
     */
    public void orbit(float x, float y) {
        if (isInitialized()) {
            nativeOrbit(nativeCameraPtr, x, y);
        }
    }

    /**
     * 轨道运动（度数版本）
     */
    public void orbitDegrees(float xDegrees, float yDegrees) {
        orbit((float) Math.toRadians(xDegrees), (float) Math.toRadians(yDegrees));
    }

    /**
     * 平移摄像机
     */
    public void pan(float x, float y) {
        if (isInitialized()) {
            nativePan(nativeCameraPtr, x, y);
        }
    }

    /**
     * 根据场景调整平移速度
     */
    public void fitPanSpeedToScene(SceneExtents extents) {
        if (isInitialized()) {
            nativeFitPanSpeedToScene(nativeCameraPtr,
                    extents.minX, extents.minY, extents.minZ,
                    extents.maxX, extents.maxY, extents.maxZ);
        }
    }

    // ==================== 重置和适配方法 ====================

    /**
     * 重置摄像机
     */
    public void reset() {
        if (isInitialized()) {
            nativeReset(nativeCameraPtr);
        }
    }

    /**
     * 重置视图以适应场景
     *
     * @param gltfPtr    glTF对象指针
     * @param sceneIndex 场景索引
     */
    public void resetView(long gltfPtr, int sceneIndex) {
        if (isInitialized()) {
            nativeResetView(nativeCameraPtr, gltfPtr, sceneIndex);
        }
    }

    /**
     * 调整视图以适应场景但保持旋转
     */
    public void fitViewToScene(long gltfPtr, int sceneIndex) {
        if (isInitialized()) {
            nativeFitViewToScene(nativeCameraPtr, gltfPtr, sceneIndex);
        }
    }

    /**
     * 根据范围调整距离
     */
    public void fitDistanceToExtents(SceneExtents extents) {
        if (isInitialized()) {
            nativeFitDistanceToExtents(nativeCameraPtr,
                    extents.minX, extents.minY, extents.minZ,
                    extents.maxX, extents.maxY, extents.maxZ);
        }
    }

    /**
     * 根据范围调整摄像机目标
     */
    public void fitCameraTargetToExtents(SceneExtents extents) {
        if (isInitialized()) {
            nativeFitCameraTargetToExtents(nativeCameraPtr,
                    extents.minX, extents.minY, extents.minZ,
                    extents.maxX, extents.maxY, extents.maxZ);
        }
    }

    /**
     * 根据范围调整摄像机近远平面
     */
    public void fitCameraPlanesToExtents(SceneExtents extents) {
        if (isInitialized()) {
            nativeFitCameraPlanesToExtents(nativeCameraPtr,
                    extents.minX, extents.minY, extents.minZ,
                    extents.maxX, extents.maxY, extents.maxZ);
        }
    }



    // ==================== 资源管理 ====================

    /**
     * 清理资源
     */
    public void destroy() {
        if (nativeCameraPtr != 0) {
            nativeDestroy(nativeCameraPtr);
            nativeCameraPtr = 0;
            isInitialized = false;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (nativeCameraPtr != 0) {
                destroy();
            }
        } finally {
            super.finalize();
        }
    }

    // ==================== Native方法声明 ====================

    private native long nativeCreate();

    private native void nativeDestroy(long cameraPtr);

    // 基本操作
    private native void nativeSetVerticalFoV(long cameraPtr, float fov);


    private native void nativeLookAt(long cameraPtr, float fromX, float fromY, float fromZ,
                                     float toX, float toY, float toZ);

    private native void nativeSetPosition(long cameraPtr, float x, float y, float z);

    private native void nativeSetTarget(long cameraPtr, float x, float y, float z);

    private native void nativeSetRotation(long cameraPtr, float yaw, float pitch);

    private native void nativeSetDistanceFromTarget(long cameraPtr, float distance, float targetX
            , float targetY, float targetZ);

    // 交互操作
    private native void nativeZoomBy(long cameraPtr, float value);

    private native void nativeOrbit(long cameraPtr, float x, float y);

    private native void nativePan(long cameraPtr, float x, float y);

    private native void nativeFitPanSpeedToScene(long cameraPtr, float minX, float minY,
                                                 float minZ, float maxX, float maxY, float maxZ);

    // 重置和适配
    private native void nativeReset(long cameraPtr);

    private native void nativeResetView(long cameraPtr, long gltfPtr, int sceneIndex);

    private native void nativeFitViewToScene(long cameraPtr, long gltfPtr, int sceneIndex);

    private native void nativeFitDistanceToExtents(long cameraPtr, float minX, float minY,
                                                   float minZ, float maxX, float maxY, float maxZ);

    private native void nativeFitCameraTargetToExtents(long cameraPtr, float minX, float minY,
                                                       float minZ, float maxX, float maxY,
                                                       float maxZ);

    private native void nativeFitCameraPlanesToExtents(long cameraPtr, float minX, float minY,
                                                       float minZ, float maxX, float maxY,
                                                       float maxZ);


}