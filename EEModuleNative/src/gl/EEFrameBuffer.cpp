//
// Created by ByteDance on 2023/6/5.
//

#include "EEFrameBuffer.h"
#include <list>
#include <mutex>
namespace EE {

        struct EEFrameBufferAllocator::EEFrameBufferAllocatorMembers{
            unsigned MaxCapcity = 0;
            std::list<std::shared_ptr<EEFrameBuffer>> mFboList;
            std::mutex mMutex;
        };

        EEFrameBufferAllocator::EEFrameBufferAllocator( int initMaxCapcity): mMembers(std::make_unique<EEFrameBufferAllocatorMembers>()){
            mMembers->MaxCapcity = initMaxCapcity;
        }

        std::shared_ptr<EEFrameBuffer> EEFrameBufferAllocator::allocateEEFrameBuffer(){
            std::lock_guard<std::mutex> lock(mMembers->mMutex);
            for(auto& spFbo : mMembers->mFboList){
                if(spFbo.use_count() == 1){
                    return spFbo;
                }
            }
            if(mMembers->mFboList.size() >= mMembers->MaxCapcity){
                cleanIdleFrameBuffer();
            }
            std::shared_ptr<EEFrameBuffer> spfbo= std::shared_ptr<EEFrameBuffer>(new EEFrameBuffer());
            mMembers->mFboList.emplace_back(spfbo);
            return spfbo;
        }

        void EEFrameBufferAllocator::cleanIdleFrameBuffer(){
            auto iter = mMembers->mFboList.begin();
            while (iter!=mMembers->mFboList.end()){
                if( iter->use_count() == 1){
                    iter = mMembers->mFboList.erase(iter);
                }else{
                    iter++;
                }
            }
        }


        EEFrameBufferAllocator::~EEFrameBufferAllocator()= default;


        GLuint EEFrameBuffer::getFrameBufferID() const{
            return mFramebufferID;
        }

        EEFrameBuffer::~EEFrameBuffer(){
            if (mFramebufferID > 0)  {
                glDeleteFramebuffers(1, &mFramebufferID);
            }
        }

        EEReturnCode EEFrameBuffer::bindEETexture(const std::shared_ptr<EETexture> &sptex) const {
            if(sptex == nullptr || sptex->getTexID() == 0){
                return EE_NULLINPUT;
            }
            if(mFramebufferID == 0){
                return EE_NOTINIT;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sptex->getParam().textype, sptex->getTexID(), 0);
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(status != GL_FRAMEBUFFER_COMPLETE){
                return EE_FAIL;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return EE_OK;
        }

        void EEFrameBuffer::excuteBind(bool isbind) const{
            glBindFramebuffer(GL_FRAMEBUFFER, isbind ? mFramebufferID : 0);
        }

        EEFrameBuffer::EEFrameBuffer(){
            glGenFramebuffers(1, &mFramebufferID);
        }
}
