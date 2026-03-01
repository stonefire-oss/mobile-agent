package com.hh.agent;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import androidx.appcompat.app.AppCompatActivity;

/**
 * 启动界面 - 选择进入原生或 Vue 界面
 */
public class LauncherActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launcher);

        Button btnNative = findViewById(R.id.btnNative);
        Button btnVue = findViewById(R.id.btnVue);

        btnNative.setOnClickListener(v -> {
            startActivity(new Intent(this, MainActivity.class));
        });

        btnVue.setOnClickListener(v -> {
            startActivity(new Intent(this, VueActivity.class));
        });
    }
}
