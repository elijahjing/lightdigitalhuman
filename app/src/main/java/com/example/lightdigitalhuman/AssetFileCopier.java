package com.example.lightdigitalhuman;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.*;

public class AssetFileCopier {

    public static void copyAssetsToInternalStorage(Context context, String assetPath) {
        AssetManager assetManager = context.getAssets();
        String targetBasePath = context.getFilesDir().getAbsolutePath();

        System.out.println("=== 拷贝调试信息 ===");
        System.out.println("源路径: " + assetPath);
        System.out.println("目标基础路径: " + targetBasePath);

        try {
            // 清理路径
            if (assetPath.endsWith("/")) {
                assetPath = assetPath.substring(0, assetPath.length() - 1);
            }

            copyAssetDirectory(assetManager, assetPath, targetBasePath);


            System.out.println("文件拷贝完成！");
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println("文件拷贝失败: " + e.getMessage());
        }
    }

    private static void copyAssetDirectory(AssetManager assetManager, String assetPath,
                                           String targetBasePath) throws IOException {
        String[] assets = assetManager.list(assetPath);

        System.out.println("处理目录: " + assetPath);
        System.out.println("包含文件数: " + (assets != null ? assets.length : 0));

        if (assets != null && assets.length > 0) {
            // 使用File构造函数来安全拼接路径
            File targetDir = new File(targetBasePath, assetPath);
            if (!targetDir.exists()) {
                boolean created = targetDir.mkdirs();
                System.out.println("创建目录 " + targetDir.getAbsolutePath() + ": " + created);
            }

            // 处理每个子项
            for (String asset : assets) {
                System.out.println("处理子项: " + asset);
                String childAssetPath = assetPath + "/" + asset;
                String[] childAssets = assetManager.list(childAssetPath);

                if (childAssets != null && childAssets.length > 0) {
                    // 子目录，递归处理
                    copyAssetDirectory(assetManager, childAssetPath, targetBasePath);
                } else {
                    // 文件，直接拷贝 - 关键修复点！
                    File targetFile = new File(targetBasePath, childAssetPath);
                    copyFile(assetManager, childAssetPath, targetFile);
                }
            }
        }
    }

    private static void copyFile(AssetManager assetManager, String assetPath, File targetFile) throws IOException {
        System.out.println("拷贝文件: " + assetPath + " -> " + targetFile.getAbsolutePath());

        // 确保父目录存在
        File parentDir = targetFile.getParentFile();
        if (parentDir != null && !parentDir.exists()) {
            boolean created = parentDir.mkdirs();
            System.out.println("创建父目录 " + parentDir.getAbsolutePath() + ": " + created);
        }

        try (InputStream input = assetManager.open(assetPath);
             FileOutputStream output = new FileOutputStream(targetFile)) {

            byte[] buffer = new byte[4096];
            int totalBytes = 0;
            int bytesRead;
            while ((bytesRead = input.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
                totalBytes += bytesRead;
            }

            System.out.println("已拷贝: " + assetPath + " -> " + targetFile.getAbsolutePath());
        }
    }

    /**
     * 获取模型文件的完整路径
     */
    public String getModelFilePath(Context context, String relativePath) {
        return new File(context.getFilesDir(), relativePath).getAbsolutePath();
    }
}
