package com.example.lightdigitalhuman;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import com.example.lightdigitalhuman.render.Engine;

public class VRMLoader {

    private static String TAG = "DigitalHuman";

    public interface InitResult {

        void success();

        void faild();
    }

    public boolean loadGltfFile(String fileName, Context context, Engine engine) {
//        String vrmFilename = "testmodel/test";
//        String vrmFilename2 = "testmodel/test/DiffuseTransmissionPlant.gltf";
        String targetBasePath = context.getFilesDir().getAbsolutePath();
        String vrmFilename = targetBasePath + "/" + fileName;
        // assets/testmodel/BrainStem
        // String vrmFilename = "testmodel/DamagedHelmet/DamagedHelmet.glb";

        AssetFileCopier.copyAssetsToInternalStorage(context, fileName);
        // 调用native函数
        return engine.loadModelFromFile(vrmFilename);
    }

    public boolean loadGlbFile(String fileName, Context context, Engine engine) {
        AssetManager assetManager = context.getAssets();
        // String vrmFilename = "testmodel/vrm/AvatarSample_K.vrm";
        //String vrmFilename = "testmodel/glb/g3.glb";
        // 调用native函数
        return engine.loadModel(assetManager, fileName);
    }

    public void loadVRMFile(String name, Context context, Engine engine, InitResult initResult) {
        try {
            // 获取AssetManager
            AssetManager assetManager = context.getAssets();
            boolean glb = name.endsWith(".glb");
            boolean success;
            if (glb) {
                success = loadGlbFile(name, context, engine);
            } else {
                success = loadGltfFile(name, context, engine);

            }
            if (success) {
                Log.i(TAG, "VRM loaded successfully");
                boolean loadEnvironmentIblFromAssets = engine.loadEnvironmentIblFromAssets("envs" +
                        "/afrikaans_church_interior_1k.hdr", assetManager);
                Log.i(TAG,
                        "loadEnvironmentIblFromAssets loadEnvironmentIblFromAssets " + loadEnvironmentIblFromAssets);

                initResult.success();

            } else {
                Log.e(TAG, "Failed to load VRM");
                initResult.faild();
            }

        } catch (Exception e) {
            Log.e(TAG, "Error loading VRM: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
