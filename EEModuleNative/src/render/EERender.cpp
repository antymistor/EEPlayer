//
// Created by antymistor on 2023/6/10.
//

#include "EERender.h"
namespace EE {
        EERender::EERender():mMembers(std::make_unique<EERenderBaseMembers>()){}
        EERender::~EERender()= default;

        EEReturnCode EERender::updateShaderCode(const EEShaderCode& vertexshader, const EEShaderCode& fragshader){
            mMembers->mVertexShader = std::make_shared<EEShaderCode>(vertexshader);
            mMembers->mFragShader = std::make_shared<EEShaderCode>(fragshader);
            mMembers->isBuild = false;
            return EE_OK;
        }

        EEReturnCode EERender::build(){
            if(mMembers->isBuild){
                return EE_OK;
            }
            if(nullptr == mMembers->mVertexShader || nullptr == mMembers->mFragShader){
                return EE_FAIL;
            }
            mMembers->mProgram = std::make_shared<EEGLProgram>();
            mMembers->mProgram->compile(mMembers->mVertexShader, mMembers->mFragShader);
            mMembers->mProgram->link();
            mMembers->mVertexShader->fetchIndex( mMembers->mProgram);
            mMembers->mFragShader->fetchIndex( mMembers->mProgram);
            mMembers->isBuild = true;
            return EE_OK;
        }

        EEReturnCode EERender::render(){
            if(!mMembers->isBuild){
                build();
            }
            return EE_OK;
       }
}
