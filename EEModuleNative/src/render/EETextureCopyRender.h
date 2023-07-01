//
// Created by antymistor on 2023/6/10.
//

#ifndef CVTEXTREADER_EETEXTURECOPYRENDER_H
#define CVTEXTREADER_EETEXTURECOPYRENDER_H

#include "EERender.h"
#include "../gl/EETexture.h"
#include "../gl/EEFrameBuffer.h"
#include "../basedefine/EEMatrix.h"
namespace EE {
    class EETextureCopyRender : public EERender {
    public:
        struct EETextureCopyRenderParam{
            EERect destRenderArea;
            EEViewFillMode fillmode = IN_CENTER_BY_FIT;
            int rotation = 0;
            bool isFlipByY = false;
            float scale    = 1.0f;
            std::shared_ptr<EETexture>     srcTexture      = nullptr;
            std::shared_ptr<EEFrameBuffer> destFramebuffer = nullptr;
            EEShaderCode customFragshader = base2DFragmentCode;
            std::vector<float> paramList;
        };
        void build(const std::shared_ptr<EETextureCopyRenderParam>& param);
        EEReturnCode render() override;

    private:
        void updateMatrix();
        EEMatrixToRender mMatrix;
        std::shared_ptr<EETextureCopyRenderParam> mParam;
    };
}


#endif //CVTEXTREADER_EETEXTURECOPYRENDER_H
