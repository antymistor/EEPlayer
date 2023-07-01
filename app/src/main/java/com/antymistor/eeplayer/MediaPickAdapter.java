package com.antymistor.eeplayer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;

/**
 * Created by antymistor on 2023/6/23
 *
 * @author azq2018@zju.edu.cn.com
 */
public class MediaPickAdapter  extends RecyclerView.Adapter<MediaPickAdapter.ViewHolder>{
    private final ArrayList<Bitmap> mImageList;
    private MediaPickAdapter.OnItemClickListener mOnItemClickListener;

    public void setOnItemClickListener(MediaPickAdapter.OnItemClickListener listener) {
        mOnItemClickListener = listener;
    }

    public interface OnItemClickListener {
        void onItemClick(int position);
    }

    public MediaPickAdapter(ArrayList<Bitmap> data) {
        mImageList = data;
    }


    public static class ViewHolder extends RecyclerView.ViewHolder {
        private final ImageView imageView;
        public ViewHolder(View v) {
            super(v);
            v.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.d("MediaPickAdapter", "Element " + getAdapterPosition() + " clicked.");
                }
            });
            imageView = (ImageView) v.findViewById(R.id.image_iv);
        }

        public ImageView getImageView() {
            return imageView;
        }
    }


    @NonNull
    @Override
    public MediaPickAdapter.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.pick_item, parent, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull MediaPickAdapter.ViewHolder holder, int position) {
        holder.getImageView().setImageBitmap(mImageList.get(position));
        holder.imageView.setOnClickListener((v) -> {
            if(mOnItemClickListener != null) {
                mOnItemClickListener.onItemClick(position);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mImageList == null ? 0 : mImageList.size();
    }
}
