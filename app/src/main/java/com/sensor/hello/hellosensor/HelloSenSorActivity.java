package com.sensor.hello.hellosensor;

import android.Manifest;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.IntentFilter;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import java.util.Timer;
import java.util.TimerTask;
import java.lang.Runnable;
import android.view.View;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import 	android.content.res.AssetManager;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class HelloSenSorActivity extends Activity implements SurfaceHolder.Callback {

    public static final String TAG = "HelloSenSorActivity";
    public static final String FIRST_TIME_TAG = "first_time";
    public static final  String ASSETS_SUB_FOLDER_NAME = "raw";
    public static final int BUFFER_SIZE = 1024;
    private SurfaceView MySurfaceView;
    public long mNativeAPP;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    /*
     * copy the Assets from assets/raw to app's external file dir
     */
    public void copyAssetsToExternal() {
        AssetManager assetManager = getAssets();
        String[] files = null;
        try {
            InputStream in = null;
            OutputStream out = null;

            files = assetManager.list(ASSETS_SUB_FOLDER_NAME);
            for (int i = 0; i < files.length; i++) {
                in = assetManager.open(ASSETS_SUB_FOLDER_NAME + "/" + files[i]);
                String outDir = getExternalFilesDir(null).toString() + "/";

                File outFile = new File(outDir, files[i]);
                out = new FileOutputStream(outFile);
                copyFile(in, out);
                in.close();
                in = null;
                out.flush();
                out.close();
                out = null;
            }
        } catch (IOException e) {
            Log.e("tag", "Failed to get asset file list.", e);
        }
        File file = getExternalFilesDir(null);
        Log.d("tag", "file:" + file.toString());
    }
    /*
     * read file from InputStream and write to OutputStream.
     */
    private void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[BUFFER_SIZE];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        //setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
//        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
//        if (!prefs.getBoolean(FIRST_TIME_TAG, false)) {
//            SharedPreferences.Editor editor = prefs.edit();
//            editor.putBoolean(FIRST_TIME_TAG, true);
//            editor.apply();
//            copyAssetsToExternal();
//            Log.v(TAG, "====FIRST_TIME_TAG");
//        }
        TextView xx = findViewById(R.id.sample_text);
        MySurfaceView = new SurfaceView(this);
        setContentView(MySurfaceView);
        MySurfaceView.getHolder().addCallback(this);
        // TODO(b/139010241): Avoid that action and status bar are displayed when pressing settings
        // button.
        setImmersiveSticky();
        // Forces screen to max brightness.
        WindowManager.LayoutParams layout = getWindow().getAttributes();
        layout.screenBrightness = 1.f;
        getWindow().setAttributes(layout);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        // Prevents screen from dimming/locking.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mNativeAPP = createNativeApp();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.v(TAG, "====Activity onPause.");
        nativePause(mNativeAPP);

    }
    @Override
    protected void onResume() {
        super.onResume();
        Log.v(TAG, "====Activity onResume.");
        nativeResume(mNativeAPP);

    }
    @Override
    protected void onDestroy() {
        Log.v(TAG, "====Activity onDestroy");
        super.onDestroy();

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "====Surface surfaceCreated");
        nativeSurfaceCreated( mNativeAPP );
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d(TAG, "====Surface surfaceChanged");

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "====Surface surfaceDestroyed");

    }

    private void setImmersiveSticky() {
        getWindow()
                .getDecorView()
                .setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native long createNativeApp();
    public native void nativePause(long nativePtr);
    public native void nativeResume(long nativePtr);
    public native void nativeDestroy(long nativePtr);
    public native void nativeSurfaceCreated(long nativePtr);
    public native void nativeSurfaceChanged(long nativePtr);
    public native void nativeSurfaceDestroyed(long nativePtr);
}
