package com.example.lightdigitalhuman.render;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import com.example.lightdigitalhuman.ModelGestureController;


/**
 * 自定义GLSurfaceView，集成手势控制
 */
public class Model3DView extends GLSurfaceView {
    private ModelGestureController gestureController;

    public Model3DView(Context context) {
        super(context);
    }
    public Model3DView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    public void init(Engine engine, UserCamera camera) {
        // 等待UserCamera创建完成
        if (camera != null) {
            gestureController = new ModelGestureController(getContext(), engine, camera);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int pointerCount = event.getPointerCount();
        Log.i("pointerCount", "✅ 手势控制器已连接"+pointerCount);

        if (gestureController != null) {
            return gestureController.onTouchEvent(event);
        }
        return super.onTouchEvent(event);
    }

    /**
     * 请求重绘
     */
    public void requestRender() {
        super.requestRender();
    }
}