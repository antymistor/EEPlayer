//
// Created by ByteDance on 2023/6/10.
//

#ifndef CVTEXTREADER_EERENDER_H
#define CVTEXTREADER_EERENDER_H
#include "../basedefine/EECodeDefine.h"
#include <memory>
#include "EEGLShaderLib.h"
#include "EEGLProgram.h"
namespace EE {
    class EERender {
    public:
        EERender();
        virtual ~EERender();
        virtual EEReturnCode build();
        virtual EEReturnCode render();


    protected:
        EEReturnCode updateShaderCode(const EEShaderCode& vertexshader, const EEShaderCode& fragshader);
        struct EERenderBaseMembers{
            std::shared_ptr<EEGLProgram>  mProgram = nullptr;
            std::shared_ptr<EEShaderCode> mVertexShader = nullptr;
            std::shared_ptr<EEShaderCode> mFragShader = nullptr;
            bool isBuild = false;
        };
        std::unique_ptr<EERenderBaseMembers> mMembers;

    };
}


#endif //CVTEXTREADER_EERENDER_H
