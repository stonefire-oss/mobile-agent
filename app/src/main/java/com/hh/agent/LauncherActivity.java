package com.hh.agent;

import android.content.Intent;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;

/**
 * 启动界面 - 直接进入原生界面
 */
public class LauncherActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 直接进入 MainActivity
        startActivity(new Intent(this, MainActivity.class));
        finish();
    }
}
