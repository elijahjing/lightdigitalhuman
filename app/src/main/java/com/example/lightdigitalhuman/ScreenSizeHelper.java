package com.example.lightdigitalhuman;

import android.app.Activity;
import android.content.Context;
import android.graphics.Point;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;
import android.view.WindowMetrics;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import androidx.annotation.RequiresApi;

public class ScreenSizeHelper {

    // 方法1：使用 WindowManager (API 30 以下推荐)
    public static Point getScreenSizeWithWindowManager(Context context) {
        WindowManager windowManager =
                (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = windowManager.getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        return size;
    }

    // 方法2：使用 DisplayMetrics (API 30 以下推荐)
    public static Point getScreenSizeWithDisplayMetrics(Context context) {
        DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
        return new Point(displayMetrics.widthPixels, displayMetrics.heightPixels);
    }

    // 方法3：使用 WindowMetrics (API 30 及以上推荐)
    @RequiresApi(api = Build.VERSION_CODES.R)
    public static Point getScreenSizeWithWindowMetrics(Context context) {
        WindowManager windowManager =
                (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        WindowMetrics windowMetrics = windowManager.getCurrentWindowMetrics();
        android.graphics.Rect bounds = windowMetrics.getBounds();
        return new Point(bounds.width(), bounds.height());
    }

    // 方法4：在 Activity 中获取 (API 30 以下)
    public static Point getScreenSizeInActivity(Activity activity) {
        Display display = activity.getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        return size;
    }

    // 方法5：获取真实屏幕尺寸，包括导航栏 (API 17+)
    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    public static Point getRealScreenSize(Context context) {
        WindowManager windowManager =
                (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = windowManager.getDefaultDisplay();
        Point size = new Point();
        display.getRealSize(size);
        return size;
    }

    // 兼容性获取屏幕尺寸的工具类
    public static class ScreenUtils {

        // 获取屏幕尺寸（像素）
        public static Point getScreenSize(Context context) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                // API 30 及以上使用 WindowMetrics
                WindowManager windowManager =
                        (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
                WindowMetrics windowMetrics = windowManager.getCurrentWindowMetrics();
                android.graphics.Rect bounds = windowMetrics.getBounds();
                return new Point(bounds.width(), bounds.height());
            } else {
                // API 30 以下使用 DisplayMetrics
                DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
                return new Point(displayMetrics.widthPixels, displayMetrics.heightPixels);
            }
        }

        // 获取屏幕尺寸（dp）
        public static Point getScreenSizeDp(Context context) {
            DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
            int widthDp = (int) (displayMetrics.widthPixels / displayMetrics.density);
            int heightDp = (int) (displayMetrics.heightPixels / displayMetrics.density);
            return new Point(widthDp, heightDp);
        }

        // 获取屏幕密度
        public static float getScreenDensity(Context context) {
            DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
            return displayMetrics.density;
        }

        // 获取屏幕DPI
        public static int getScreenDensityDpi(Context context) {
            DisplayMetrics displayMetrics = context.getResources().getDisplayMetrics();
            return displayMetrics.densityDpi;
        }

        // px转dp
        public static int pxToDp(Context context, float pxValue) {
            float scale = context.getResources().getDisplayMetrics().density;
            return (int) (pxValue / scale + 0.5f);
        }

        // dp转px
        public static int dpToPx(Context context, float dpValue) {
            float scale = context.getResources().getDisplayMetrics().density;
            return (int) (dpValue * scale + 0.5f);
        }
    }


}
