package com.example.lightdigitalhuman.render;

import android.annotation.SuppressLint;
import androidx.annotation.NonNull;

/**
 * 场景范围类
 * 对应C++ SceneExtents结构体
 */
public class SceneExtents {
    public float minX, minY, minZ;
    public float maxX, maxY, maxZ;

    public SceneExtents() {
        this(0f, 0f, 0f, 0f, 0f, 0f);
    }

    public SceneExtents(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
        this.minX = minX;
        this.minY = minY;
        this.minZ = minZ;
        this.maxX = maxX;
        this.maxY = maxY;
        this.maxZ = maxZ;
    }

    @NonNull
    @SuppressLint("DefaultLocale")
    @Override
    public String toString() {
        return String.format("SceneExtents[min:(%.2f,%.2f,%.2f), max:(%.2f,%.2f,%.2f)]",
                minX, minY, minZ, maxX, maxY, maxZ);
    }
}