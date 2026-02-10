package com.example.lightdigitalhuman;

import android.content.Context;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import com.example.lightdigitalhuman.render.Engine;
import com.example.lightdigitalhuman.render.UserCamera;

/**
 * 3D模型手势控制管理器
 * 结合UserCamera实现手势交互
 */
public class ModelGestureController {
    private static final String TAG = "ModelGestureController";

    private UserCamera userCamera;

    private Engine engine;

    private GestureDetector gestureDetector;
    private ScaleGestureDetector scaleGestureDetector;

    // 手势控制参数
    private static final float ORBIT_SENSITIVITY = 0.5f;    // 旋转灵敏度
    private static final float PAN_SENSITIVITY = 1f;     // 平移灵敏度
    private static final float ZOOM_SENSITIVITY = 1f;      // 缩放灵敏度

    public ModelGestureController(Context context, Engine engine, UserCamera camera) {
        this.userCamera = camera;
        this.engine = engine;
        initGestureDetectors(context);
    }

    private void initGestureDetectors(Context context) {
        // 1. 普通手势检测器（单指操作）
        gestureDetector = new GestureDetector(context,
                new GestureDetector.SimpleOnGestureListener() {

                    @Override
                    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
                                            float distanceY) {
                        // 单指滑动 = 轨道旋转（绕模型旋转）
                        handleOrbitGesture(distanceX, distanceY);
                        return true;
                    }

                    @Override
                    public boolean onDoubleTap(MotionEvent e) {
                        // 双击重置视角
                        handleResetView();
                        return true;
                    }

                    @Override
                    public boolean onSingleTapConfirmed(MotionEvent e) {
                        // 单击可以用来选择模型部件（可选）
                        handleModelTap(e.getX(), e.getY());
                        return true;
                    }
                });

        // 2. 缩放手势检测器（双指操作）
        scaleGestureDetector = new ScaleGestureDetector(context,
                new ScaleGestureDetector.SimpleOnScaleGestureListener() {

                    @Override
                    public boolean onScale(ScaleGestureDetector detector) {
                        // 双指缩放 = 相机缩放
                        handleZoomGesture(detector.getScaleFactor());
                        return true;
                    }
                });

    }

    public boolean onTouchEvent(MotionEvent event) {
        // 多指操作检测
        boolean scaleHandled = scaleGestureDetector.onTouchEvent(event);
        boolean gestureHandled = false;
        // 如果不是缩放手势，才处理其他手势
        if (!scaleGestureDetector.isInProgress()) {
            if (event.getPointerCount() == 2) {
                handleTwoFingerPan(event);
                gestureHandled = true;
            } else if (event.getPointerCount() == 1) {
                gestureHandled = gestureDetector.onTouchEvent(event);
            }
        }

        return scaleHandled || gestureHandled;
    }

    /**
     * 处理轨道旋转手势（绕模型旋转）
     *
     * @param distanceX X轴滑动距离
     * @param distanceY Y轴滑动距离
     */
    private void handleOrbitGesture(float distanceX, float distanceY) {
        if (userCamera == null || !userCamera.isInitialized()) {
            return;
        }
        float deltaX = distanceX * ORBIT_SENSITIVITY;
        float deltaY = distanceY * ORBIT_SENSITIVITY;
        userCamera.orbit(deltaX, deltaY);
    }

    /**
     * 处理缩放手势
     *
     * @param scaleFactor 缩放因子（>1放大，<1缩小）
     */
    private void handleZoomGesture(float scaleFactor) {
        twoFingerPanStarted = false;
        if (userCamera == null || !userCamera.isInitialized()) {
            return;
        }
        float newScaleFactor = scaleFactor * ZOOM_SENSITIVITY;
        userCamera.zoomBy(newScaleFactor);
    }

    private float lastTwoFingerX = 0;
    private float lastTwoFingerY = 0;
    private boolean twoFingerPanStarted = false;

    /**
     * 处理双指平移
     */
    private void handleTwoFingerPan(MotionEvent event) {
        if (event.getPointerCount() < 2) return;

        // 计算两指中点
        float currentX = (event.getX(0) + event.getX(1)) / 2;
        float currentY = (event.getY(0) + event.getY(1)) / 2;

        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_POINTER_DOWN:
                // 第二个手指按下，开始双指操作
                lastTwoFingerX = currentX;
                lastTwoFingerY = currentY;
                twoFingerPanStarted = true;
                break;

            case MotionEvent.ACTION_MOVE:
                if (twoFingerPanStarted && userCamera != null) {
                    float deltaX = (currentX - lastTwoFingerX) * PAN_SENSITIVITY;
                    float deltaY = (currentY - lastTwoFingerY) * PAN_SENSITIVITY;
                    userCamera.pan(deltaX, -deltaY);
                    lastTwoFingerX = currentX;
                    lastTwoFingerY = currentY;

                }
                break;

            case MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_UP:
                twoFingerPanStarted = false;
                break;
        }
    }

    /**
     * 重置视角
     */
    private void handleResetView() {
        if (userCamera == null || !userCamera.isInitialized()) {
            return;
        }
        userCamera.reset();
        if (engine != null) {
            userCamera.resetView(engine.getNativeEnginePtr(), 0);  // 场景索引0
        }
    }

    /**
     * 点击选择模型（可选功能）
     */
    private void handleModelTap(float x, float y) {
        // 这里可以实现射线检测，选择模型部件
        // 或者实现点击聚焦功能
    }
}