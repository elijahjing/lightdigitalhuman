package com.example.lightdigitalhuman;

import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

// 🔥 FPS计算器 - Pure Java版本
public class FPSCounter {
    private long lastTime;           // 上次更新时间
    private long currentTime;        // 当前时间
    private int frameCount;          // 帧计数
    private float currentFPS;        // 当前FPS
    private float averageFPS;        // 平均FPS
    private List<Float> fpsHistory;  // FPS历史记录
    private static final int MAX_HISTORY = 60; // 保存60个FPS样本

    public FPSCounter() {
        fpsHistory = new ArrayList<>();
        reset();
    }

    public void reset() {
        lastTime = System.currentTimeMillis();
        currentTime = lastTime;
        frameCount = 0;
        currentFPS = 0.0f;
        averageFPS = 0.0f;
        fpsHistory.clear();
    }

    // 🔥 每帧调用一次
    public void update() {
        frameCount++;
        currentTime = System.currentTimeMillis();

        // 每秒计算一次FPS
        long deltaTime = currentTime - lastTime;
        if (deltaTime >= 1000) { // 1秒 = 1000毫秒
            currentFPS = (float) frameCount * 1000.0f / (float) deltaTime;

            // 🔥 更新FPS历史
            fpsHistory.add(currentFPS);
            if (fpsHistory.size() > MAX_HISTORY) {
                fpsHistory.remove(0);
            }

            // 🔥 计算平均FPS
            float sum = 0.0f;
            for (Float fps : fpsHistory) {
                sum += fps;
            }
            averageFPS = sum / fpsHistory.size();

            // 重置计数器
            lastTime = currentTime;
            frameCount = 0;

            // 输出FPS信息
            Log.i("FPSCounter", String.format("FPS: 当前=%.1f, 平均=%.1f, 历史样本=%d",
                    currentFPS, averageFPS, fpsHistory.size()));
        }
    }

    // 🔥 获取当前FPS
    public float getCurrentFPS() {
        return currentFPS;
    }

    public float getAverageFPS() {
        return averageFPS;
    }

    // 🔥 获取详细信息
    public FPSInfo getDetailedInfo() {
        FPSInfo info = new FPSInfo();
        info.currentFPS = currentFPS;
        info.averageFPS = averageFPS;

        if (!fpsHistory.isEmpty()) {
            info.minFPS = Collections.min(fpsHistory);
            info.maxFPS = Collections.max(fpsHistory);
        }

        info.sampleCount = fpsHistory.size();
        return info;
    }

    // 🔥 FPS信息数据类
    public static class FPSInfo {
        public float currentFPS = 0.0f;
        public float averageFPS = 0.0f;
        public float minFPS = 0.0f;
        public float maxFPS = 0.0f;
        public int sampleCount = 0;
    }
}