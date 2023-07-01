package com.antymistor.eeplayer;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.media.MediaMetadataRetriever;
import android.os.Build;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.antymistor.eeplayer.utils.BitmapResize;
import com.antymistor.eeplayer.utils.SearchFileProvider;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.Objects;

/**
 * Created by aizhiqiang on 2023/6/22
 *
 * @author aizhiqiang@bytedance.com
 */
public class AlbumActivity extends AppCompatActivity {
    private RecyclerView mRecyclerView;
    private RecyclerView.LayoutManager mLayoutManager;
    private static final int spancount = 3;
    private static final int pullFrameTime = 5000;
    private ArrayList<Bitmap> mImageList;
    private MediaPickAdapter mAdapter;
    private ArrayList<SearchFileProvider.FileBean> mPathList;
    private MediaMetadataRetriever mediaRetriever;
    private int destImageSize;
    private SearchFileProvider fileProvider;
    private Boolean hasbuildDir = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Objects.requireNonNull(getSupportActionBar()).hide();
        WindowManager.LayoutParams attributes = getWindow().getAttributes();
        attributes.flags |= WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION;
        getWindow().setAttributes(attributes);
        getWindow().setStatusBarColor(Color.TRANSPARENT);
        setContentView(R.layout.album_activity);
        mediaRetriever = new MediaMetadataRetriever();
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        destImageSize = dm.widthPixels / spancount;


        mImageList = new ArrayList<>();
        mAdapter = new MediaPickAdapter(mImageList);
        mRecyclerView = findViewById(R.id.recyclerView);
        mLayoutManager = new GridLayoutManager(this, spancount);
        mRecyclerView.setLayoutManager(mLayoutManager);
        mRecyclerView.scrollToPosition(0);
        mRecyclerView.setAdapter(mAdapter);

        fileProvider = new SearchFileProvider(this);
        fileProvider.setMaxCnt(-1);
        fileProvider.setLisenter(new SearchFileProvider.ISearchListener() {
            @RequiresApi(api = Build.VERSION_CODES.O_MR1)
            @Override
            public void onSearchFinish() {
                drawRecyclerView();
            }
            @Override
            public void onSearchFile(SearchFileProvider.FileBean f) {
            }
        });
        fileProvider.searchLocalVideoFile();
    }

    private void goPlayPage(String filePath){
        Intent intent = new Intent(this, VideoPlayActivity.class);
        intent.putExtra("filePath", filePath);
        startActivity(intent);
    }

    private void drawRecyclerView() {
        mPathList = fileProvider.getOrWriteList(null, SearchFileProvider.operateType.READ);
        mAdapter.setOnItemClickListener(new MediaPickAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(int position) {
                Log.i("setOnItemClickListener", "click on " + position);
                goPlayPage(mPathList.get(position).filePath);
            }
        });
      //  for(  SearchFileProvider.FileBean item : mPathList){
        for(int i=0; i<mPathList.size(); ++i){
            String fileName = mPathList.get(i).fileName.substring(0,  mPathList.get(i).fileName.length() - 4);
            String cachepath = getFilesDir().getPath() + "/cache/" + fileName + ".jpeg";
            File file = new File(cachepath);
            if(file.exists()) {
                mImageList.add(BitmapFactory.decodeFile(cachepath));
                runOnUiThread(() -> {
                    mAdapter.notifyItemChanged(mImageList.size() - 1);
                });
                continue;
            }
            try {
                mediaRetriever.setDataSource( mPathList.get(i).filePath);
            } catch (Exception ie){
                mPathList.remove(i);
                --i;
                continue;
            }
            Bitmap srcmap = mediaRetriever.getFrameAtTime(pullFrameTime);
            Bitmap finalmap = BitmapResize.BitmapCut(srcmap,destImageSize,destImageSize);
            mImageList.add(BitmapResize.BitmapCut(finalmap, destImageSize,destImageSize));
            runOnUiThread(() -> {
                mAdapter.notifyItemChanged(mImageList.size() - 1);
            });
            if(!hasbuildDir){
                File dir = new File(getFilesDir().getPath() + "/cache/");
                if (!dir.exists()) {
                    hasbuildDir = dir.mkdirs();
                }
            }
            try {
                finalmap.compress(Bitmap.CompressFormat.JPEG, 50, new FileOutputStream(cachepath));
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

}
