package com.antymistor.eeplayer;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import androidx.appcompat.app.AppCompatActivity;
import com.antymistor.eeplayer.utils.Permission;

import java.io.File;
import java.util.Objects;

/**
 * Created by antymistor on 2023/6/22
 * @author azq2018@zju.edu.cn.com
 */
public class LaunchActivity extends AppCompatActivity {

    @Override
    protected void onStart() {
        super.onStart();
        Permission.checkPermission(this);
    }
    public void goNextpage(){
        Intent intent = new Intent(this, VideoPlayActivity.class);
        intent.putExtra("filePath", "");
        startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Objects.requireNonNull(getSupportActionBar()).hide();
        WindowManager.LayoutParams attributes = getWindow().getAttributes();
        attributes.flags |= WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION;
        getWindow().setAttributes(attributes);
        getWindow().setStatusBarColor(Color.TRANSPARENT);
        setContentView(R.layout.launch_activity);
        findViewById(R.id.button).setBackgroundColor(Color.parseColor("#00000000"));
        if(new File(getFilesDir().getPath() + "/progress.json").exists()){     //首次安装不跳入阅读页
            goNextpage();
        }else{
            Permission.checkPermission(this);
        }
    }
    public void ChooseButtom(View view) {
        Intent intent = new Intent(this, AlbumActivity.class);
        startActivity(intent);
    }
}
