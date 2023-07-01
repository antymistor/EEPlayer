//
// Created by ByteDance on 2023/6/10.
//

#include "EETextureCopyRender.h"

#include <utility>
namespace EE {

    void EETextureCopyRender::build(const std::shared_ptr<EETextureCopyRenderParam>& param){
        mParam = param;
        updateShaderCode(baseVertexCode, param->customFragshader);
        EERender::build();
        updateMatrix();
    }
    void EETextureCopyRender::updateMatrix(){
        mMatrix.setRotByInt(mParam->rotation);
        mMatrix.setDestSize({mParam->destRenderArea.width, mParam->destRenderArea.height});
        mMatrix.setFillmode(mParam->fillmode);
        mMatrix.setScale(mParam->scale);
        if(mParam->srcTexture) {
            mMatrix.setSrcSize(mParam->srcTexture->getParam().size);
        }
        mMatrix.updateMatrix();
    }

    EEReturnCode EETextureCopyRender::render(){
        if(mParam->srcTexture == nullptr){
            return EE_FAIL;
        }
        updateMatrix();

       if(mParam->destFramebuffer == nullptr){
           glBindFramebuffer(GL_FRAMEBUFFER, 0);
       }else{
           mParam->destFramebuffer->excuteBind(true);
       }
        glViewport(mParam->destRenderArea.pos_x, mParam->destRenderArea.pos_y,
                   mParam->destRenderArea.width, mParam->destRenderArea.height);
        mMembers->mProgram->use();

        //render
        ShaderIndex::fillShaderIndex(&mMembers->mVertexShader->shaderIndexVector[0], EE_VERTEXCOORDS);
        ShaderIndex::fillShaderIndex(&mMembers->mVertexShader->shaderIndexVector[1], mParam->isFlipByY ? EE_TEXTURECOORDS_FLIP_Y : EE_TEXTURECOORDS);
        ShaderIndex::fillShaderIndex(&mMembers->mVertexShader->shaderIndexVector[2], mMatrix.getPointer());
        glActiveTexture(GL_TEXTURE0);
        ShaderIndex::fillShaderIndex(&mMembers->mFragShader->shaderIndexVector[0], 0);
        mParam->srcTexture->excuteBind(true);
        for(int i = 0 ; i!= mParam->paramList.size(); ++i){
            ShaderIndex::fillShaderIndex(&mMembers->mFragShader->shaderIndexVector[i+1], mParam->paramList[i]);
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        if(mParam->destFramebuffer != nullptr){
            mParam->destFramebuffer->excuteBind(false);
        }
        ShaderIndex::disableAttribIndex(&mMembers->mVertexShader->shaderIndexVector[0]);
        ShaderIndex::disableAttribIndex(&mMembers->mVertexShader->shaderIndexVector[1]);
        return EE_OK;
    }

}