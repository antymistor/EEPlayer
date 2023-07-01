//
// Created by antymistor on 2023/6/10.
//

#ifndef CVTEXTREADER_EEMATRIX_H
#define CVTEXTREADER_EEMATRIX_H

#include "EECodeDefine.h"
namespace EE{
    class EEMatrix {
    public:
        EEMatrix();
        EEMatrix(const EEMatrix& mat_);
        ~EEMatrix();
        void initView();
        void initIdentityMatrix();
        void setTranslate(float x, float y);
        void setScale(float x, float y);
        void setRotate(float r);

        void doTranslate(float dx, float dy);
        void doScale(float dx, float dy);
        void doRotate(float dr);
        EEMatrix operator*(const EEMatrix& m);
        EEMatrix &operator*=(const EEMatrix& m);
        EEMatrix &operator=(const EEMatrix& m);
        float *getPointer() const;
        void set(int x, int y, float v);

    private:
        float *mat;
    };

    class EEMatrixToRender : public EEMatrix{
    public:
        void setRotByInt(const int& rotation_);
        void setFillmode(const EEViewFillMode& fillmode_);
        void setSrcSize(const EESize& srcSize_);
        void setDestSize(const EESize& destSize_);
        void setScale(const float& scale_);
        void updateMatrix();
    private:
        unsigned rotation = 0 ;
        EEViewFillMode fillmode = IN_CENTER_BY_STRETCH;
        EESize srcSize = {0,0};
        EESize destSize = {0,0};
        float scale = 1.0f;
        bool needUpdate = false;
    };
}


#endif //CVTEXTREADER_EEMATRIX_H
