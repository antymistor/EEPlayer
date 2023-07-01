//
// Created by antymistor on 2023/6/10.
//

#include "EEMatrix.h"
#include <memory>
namespace EE{
        EEMatrix::EEMatrix() {
            mat = new float[16];
            initIdentityMatrix();
        }

        EEMatrix::EEMatrix(const EEMatrix& mat_){
            mat = new float[16];
            memcpy((void*)mat, mat_.mat, 16 * sizeof(float));
        }
        EEMatrix::~EEMatrix(){
            delete[] mat;
        }

        void EEMatrix::initView(){
            initIdentityMatrix();
            set(2, 2, -1.0f);
        }

        void EEMatrix::initIdentityMatrix(){
            memset((void*)mat, 0, 16 * sizeof(float));
            mat[0] = mat[5] = mat[10] = mat[15] = 1;
        }

        void EEMatrix::set(int x, int y, float v){
            mat[x + y * 4] = v;
        }

        void EEMatrix::setTranslate(float x, float y){
            set(3, 0, x);
            set(3, 1, y);
        }

        void EEMatrix::setScale(float x, float y){
            set(0, 0, x);
            set(1, 1, y);
        }

        void EEMatrix::doRotate(float r){
            float p00 = mat[0];
            float p10 = mat[1];
            float p01 = mat[4];
            float p11 = mat[5];

            set(0, 0,  p00 * cosf(r)  + p10 * sinf(r));
            set(1, 0,  p00 * -sinf(r) + p10 * cosf(r));
            set(0, 1,  p01 * cosf(r)  + p11 * sinf(r));
            set(1, 1,  p01 * -sinf(r) + p11 * cosf(r));
        }


        void EEMatrix::doTranslate(float dx, float dy){
            set(0, 3, mat[12] + dx);
            set(1, 3, mat[13] + dy);
        }

        void EEMatrix::doScale(float dx, float dy){
            set(0, 0, mat[0] * dx);
            set(1, 1, mat[5] * dy);
        }
        void EEMatrix::setRotate(float dr){
            initIdentityMatrix();
            doRotate(dr);
        }

        EEMatrix EEMatrix::operator*(const EEMatrix& m){
            EEMatrix matnew;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matnew.set(i, j,
                             mat[i] * m.mat[4 * j] +
                                mat[i + 4] * m.mat[4 * j] +
                                mat[i + 4] * m.mat[4 * j] +
                                mat[i + 4] * m.mat[4 * j]);
                }
            }
            return matnew;
        }

        EEMatrix & EEMatrix::operator*=(const EEMatrix& m){
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    set(i, j,mat[i]      * m.mat[4 * j]     +
                                mat[i + 4]  * m.mat[1 + 4 * j] +
                                mat[i + 8]  * m.mat[2 + 4 * j] +
                                mat[i + 12] * m.mat[3 + 4 * j]);
                }
            }
            return *this;
        }

        EEMatrix & EEMatrix::operator=(const EEMatrix& m) {
            memcpy((void*)mat, m.mat, 16 * sizeof(float));
            return *this;
        }

        float *EEMatrix::getPointer() const{
            return mat;
        }

    void EEMatrixToRender::setRotByInt(const int& rotation_){
        unsigned rotate =  (rotation_ % 360) + (rotation_ < 0 ? 360 : 0);
        if(rotation != rotate){
            rotation = rotate;
            needUpdate = true;
        }
    }
    void EEMatrixToRender::setFillmode(const EEViewFillMode& fillmode_){
        if(fillmode != fillmode_){
            fillmode = fillmode_;
            needUpdate = true;
        }
    }
    void EEMatrixToRender::setSrcSize(const EESize& srcSize_){
        if( srcSize != srcSize_ ){
            srcSize = srcSize_;
            needUpdate = true;
        }
    }
    void EEMatrixToRender::setDestSize(const EESize& destSize_){
        if(destSize != destSize_){
            destSize = destSize_;
            needUpdate = true;
        }
    }
    void EEMatrixToRender::setScale(const float& scale_){
        if(scale != scale_){
            scale = scale_;
            needUpdate = true;
        }
    }
    void EEMatrixToRender::updateMatrix(){
        if(needUpdate){
           //first for rot
            initIdentityMatrix();
           if(fillmode != IN_CENTER_BY_STRETCH &&
              srcSize  != EESize(0,0)    &&
              destSize != EESize(0,0)    &&
              rotation % 90 == 0){
              EESize newSrcSize = rotation % 180 != 90 ? srcSize : EESize{srcSize.height, srcSize.width};
              float destR = 1.0f * destSize.height  / destSize.width;
              float srcR =  1.0f * newSrcSize.height / newSrcSize.width;
              float scaleX = 1.0f;
              float scaleY = 1.0f;
              if(destR - srcR > 0.02f){
                  scaleY = fillmode == IN_CENTER_BY_FIT  ? srcR / destR : 1.0f;
                  scaleX = fillmode == IN_CENTER_BY_CROP ? destR / srcR : 1.0f;
              }else if(srcR - destR > 0.02f){
                  scaleX = fillmode == IN_CENTER_BY_FIT  ? destR / srcR : 1.0f;
                  scaleY = fillmode == IN_CENTER_BY_CROP ? srcR  / destR : 1.0f;
              }
              if(rotation % 180 == 90){
                  std::swap(scaleX, scaleY);
              }
               doScale(scaleX, scaleY);
           }
            doScale(scale, scale);
            if(rotation != 0){
                doRotate((float)(rotation * M_PI / 180));
            }
           needUpdate = false;
        }
    }
}