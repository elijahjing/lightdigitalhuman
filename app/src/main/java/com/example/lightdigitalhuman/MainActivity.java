package com.example.lightdigitalhuman;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Point;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.example.lightdigitalhuman.render.Engine;
import com.example.lightdigitalhuman.render.Model3DView;
import com.example.lightdigitalhuman.render.UserCamera;

import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'lightdigitalhuman' library on application startup.


    private static String TAG = "DigitalHuman";

    private Model3DView glSurfaceView = null;

    private int surfaceWidth = 0;
    private int surfaceHeight = 0;
    Point screenSize;

    private CheckBox btnIbL;


    LinearLayout horizontalLayoutAnime;
    Engine engine;

    UserCamera userCamera;
    boolean play = false;

    VRMLoader loader;

    public class MyGLSurfaceView extends GLSurfaceView {

        public MyGLSurfaceView(Context context) {
            super(context);

            // 确保连续渲染模式
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

            // 检查是否有帧率限制
            Log.i("GL_CONFIG", "Render mode: " + getRenderMode());
        }
    }


    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 创建GLSurfaceView
        setFullScreenImmersive();
        setContentView(R.layout.activity_main);
        btnIbL = findViewById(R.id.action_IbL);
        horizontalLayoutAnime = findViewById(R.id.horizontalLayout_anime);
        btnIbL.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                engine.setIbl(btnIbL.isChecked());
            }
        });

        glSurfaceView = findViewById(R.id.sample_text33);
        ActivityManager activityManager =
                (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        screenSize = ScreenSizeHelper.ScreenUtils.getScreenSize(this);

        // 设置OpenGL ES版本
        glSurfaceView.setEGLContextClientVersion(3);
        FPSCounter fpsCounter = new FPSCounter();
        engine = new Engine(this);
        userCamera = new UserCamera();
        engine.setUserCamera(userCamera);
        // 设置渲染器 - 使用默认的简单渲染器
        glSurfaceView.setEGLConfigChooser(new MaliG31ConfigChooser());
        glSurfaceView.setEGLContextClientVersion(3); // 使用 OpenGL ES 3.0
        glSurfaceView.init(engine, userCamera);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
               // String vrmFilename = "testmodel/glb/g2.glb";//需要根据实际情况控制光照
                String vrmFilename = "testmodel/DamagedHelmet/DamagedHelmet.glb";
                modelLoad(vrmFilename);
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Log.d(TAG, "onSurfaceChanged: " + width + "x" + height);
                surfaceWidth = width;
                surfaceHeight = height;
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                fpsCounter.update();

                engine.renderFrame(surfaceWidth, surfaceHeight);

            }
        });

    }

    public void modelLoad(String name) {
        if (loader == null) {
            loader = new VRMLoader();
        }
        loader.loadVRMFile(name, getApplicationContext(), engine, new VRMLoader.InitResult() {
            @Override
            public void success() {
                horizontalLayoutAnime.post(new Runnable() {
                    @Override
                    public void run() {
                        TextView textView;
                        List<String> animeName = engine.getAnimationAllName();

                        for (int i = 0; i < animeName.size(); i++) {
                            textView = new TextView(getApplication());
                            textView.setText(animeName.get(i));
                            // 设置外边距（margin）
                            textView.setPadding(32, 16, 32, 16); // 左、上、右、下 (单位：像素)

                            LinearLayout.LayoutParams params =
                                    new LinearLayout.LayoutParams(
                                            LinearLayout.LayoutParams.WRAP_CONTENT,
                                            LinearLayout.LayoutParams.WRAP_CONTENT
                                    );
                            params.setMargins(16, 8, 16, 8); // 左、上、右、下
                            textView.setLayoutParams(params);
                            textView.setTextColor(Color.WHITE);
                            textView.setBackgroundColor(Color.BLUE);
                            TextView finalTextView1 = textView;
                            textView.setOnClickListener(new View.OnClickListener() {
                                @Override
                                public void onClick(View v) {
                                    play = !play;
                                    if (play) {
                                        engine.playAnimation(finalTextView1
                                                .getText().toString(), -1);
                                    } else {
                                        engine.stopAnimation(finalTextView1
                                                .getText().toString());
                                    }
                                }
                            });
                            horizontalLayoutAnime.addView(textView);
                        }

                    }
                });
            }

            @Override
            public void faild() {

            }
        });
    }

    private void hideActionBar() {
        if (getSupportActionBar() != null) {
            getSupportActionBar().hide();
        }
    }

    private void setFullScreenImmersive() {
        hideActionBar();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // API 30+ 使用 WindowInsetsController
            WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
            WindowInsetsControllerCompat controller =
                    WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
            controller.hide(WindowInsetsCompat.Type.systemBars());
            controller.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        } else {

            // API 30 以下使用 SystemUiVisibility
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
            );
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        engine.destroy();
        userCamera.destroy();
    }
}