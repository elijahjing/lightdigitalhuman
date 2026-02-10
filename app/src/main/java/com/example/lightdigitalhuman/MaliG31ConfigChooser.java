package com.example.lightdigitalhuman;

import static android.opengl.EGL14.EGL_OPENGL_ES2_BIT;

import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public class MaliG31ConfigChooser implements GLSurfaceView.EGLConfigChooser {

    @Override
    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
        return tryLowPerformanceMSAA(egl, display);
    }

    private EGLConfig tryLowPerformanceMSAA(EGL10 egl, EGLDisplay display) {
        // 先尝试 2x MSAA - 性能友好
        EGLConfig config = trySpecificConfig(egl, display, 2);
        if (config != null) {
            Log.i("MALI_AA", "Successfully configured 2x MSAA");
            return config;
        }

        // 如果 2x 不行，直接用无 MSAA 的高质量配置
        Log.w("MALI_AA", "MSAA not available, using standard config");
        return getStandardConfig(egl, display);
    }

    private EGLConfig trySpecificConfig(EGL10 egl, EGLDisplay display, int samples) {
        // 电视端轻量级配置
        int[] attributes = {
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_GREEN_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 8,
                EGL10.EGL_ALPHA_SIZE, 8,
                EGL10.EGL_DEPTH_SIZE, 16,    // 降低到16位，减少内存带宽
                EGL10.EGL_STENCIL_SIZE, 0,   // 去掉模板缓冲，节省性能
                EGL10.EGL_SAMPLE_BUFFERS, 1,
                EGL10.EGL_SAMPLES, samples,
                EGL10.EGL_NONE
        };

        EGLConfig[] configs = new EGLConfig[10];
        int[] numConfigs = new int[1];

        if (!egl.eglChooseConfig(display, attributes, configs, configs.length, numConfigs)) {
            Log.e("MALI_AA", "eglChooseConfig failed for " + samples + "x MSAA");
            return null;
        }

        if (numConfigs[0] == 0) {
            Log.w("MALI_AA", "No configs found for " + samples + "x MSAA");
            return null;
        }

        // 选择第一个可用配置
        EGLConfig config = configs[0];
        logConfigDetails(egl, display, config);
        return config;
    }

    private int getConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config, int attribute) {
        int[] value = new int[1];
        egl.eglGetConfigAttrib(display, config, attribute, value);
        return value[0];
    }

    private void logConfigDetails(EGL10 egl, EGLDisplay display, EGLConfig config) {
        int samples = getConfigAttrib(egl, display, config, EGL10.EGL_SAMPLES);
        int sampleBuffers = getConfigAttrib(egl, display, config, EGL10.EGL_SAMPLE_BUFFERS);
        int depthSize = getConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE);

        Log.i("MALI_AA", "Selected config - Samples: " + samples +
                ", Sample buffers: " + sampleBuffers +
                ", Depth size: " + depthSize);
    }

    private EGLConfig getStandardConfig(EGL10 egl, EGLDisplay display) {
        // 无 MSAA 但优化的配置
        int[] attributes = {
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_GREEN_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 8,
                EGL10.EGL_ALPHA_SIZE, 8,
                EGL10.EGL_DEPTH_SIZE, 16,  // 16位深度缓冲
                EGL10.EGL_NONE
        };

        EGLConfig[] configs = new EGLConfig[1];
        int[] numConfigs = new int[1];
        egl.eglChooseConfig(display, attributes, configs, 1, numConfigs);

        if (numConfigs[0] > 0) {
            Log.i("MALI_AA", "Using standard config without MSAA");
            return configs[0];
        }

        return null;
    }
}