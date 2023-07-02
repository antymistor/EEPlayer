#include "EEShared.h"
#include "../utils/EECommonUtils.h"
#include "../basedefine/EECodeDefine.h"
#include "../thread/EETaskQueue.h"
#include "../gl/EEGLContext.h"
#include "../gl/EETexture.h"
#include <mutex>
namespace EE {
        struct EEShared::EESharedMembers{
            const char* mName{};
            std::shared_ptr<EEGLContext> mGLContext = nullptr;
            std::shared_ptr<EETextureAllocator> mTextureAllocator = nullptr;
            std::shared_ptr<EETaskQueue> mTaskQueue = nullptr;
            std::mutex mMutex;
        };
         EEShared::EEShared(const char* name) : mMembers(std::make_unique<EESharedMembers>()){
             mMembers->mName = name;
         }
         EEShared::~EEShared() {
             if(mMembers->mTaskQueue){
                 mMembers->mTaskQueue->run(EETask([&](){
                     mMembers->mTextureAllocator = nullptr;
                     mMembers->mGLContext = nullptr;
                 }));
             }
             mMembers->mTextureAllocator = nullptr;
         }

         std::shared_ptr<EEGLContext> getSharedGLContext(const std::shared_ptr<EEShared>& sObj){
             if(sObj == nullptr){
                 return nullptr;
             }
             std::unique_lock<std::mutex> lock(sObj->mMembers->mMutex);
             if(sObj->mMembers->mTaskQueue == nullptr){
                 sObj->mMembers->mTaskQueue = std::make_shared<EETaskQueue>(EECommonUtils::createGlobalName(sObj->mMembers->mName));
             }
             if(sObj->mMembers->mGLContext == nullptr){
                 sObj->mMembers->mTaskQueue->run(EETask([&]() {
                     //build glcontext
                     sObj->mMembers->mGLContext = std::make_shared<EEGLContext>();
                     sObj->mMembers->mGLContext->initContext(nullptr, false);
                     sObj->mMembers->mGLContext->initSurface(nullptr, PIXEL_BUFFER_MODE);
                     sObj->mMembers->mGLContext->makeCurrent();
                 }));
             }
             return sObj->mMembers->mGLContext;
        }

        std::shared_ptr<EETextureAllocator> getSharedTextureAllocator(const std::shared_ptr<EEShared>& sObj){
            if(sObj == nullptr){
                return nullptr;
            }
            std::unique_lock<std::mutex> lock(sObj->mMembers->mMutex);
            if(sObj->mMembers->mTextureAllocator == nullptr){
                sObj->mMembers->mTextureAllocator = std::make_shared<EETextureAllocator>();
            }
            return sObj->mMembers->mTextureAllocator;
        }

        std::shared_ptr<EETaskQueue> getSharedTaskQueue(const std::shared_ptr<EEShared>& sObj){
            if(sObj == nullptr){
                return nullptr;
            }
            std::unique_lock<std::mutex> lock(sObj->mMembers->mMutex);
            if(sObj->mMembers->mTaskQueue == nullptr){
                sObj->mMembers->mTaskQueue = std::make_shared<EETaskQueue>(EECommonUtils::createGlobalName(sObj->mMembers->mName));
            }
            return sObj->mMembers->mTaskQueue;
        }




//         std::shared_ptr<EEGLContext> EEShared::getSharedGLContext(){
//             std::unique_lock<std::mutex> lock(mMembers->mMutex);
//             if(mMembers->mTaskQueue == nullptr){
//                 mMembers->mTaskQueue = std::make_shared<EETaskQueue>(EECommonUtils::createGlobalName(mMembers->mName));
//             }
//             if(mMembers->mGLContext == nullptr){
//                 mMembers->mTaskQueue->run(EETask([&]() {
//                     //build glcontext
//                     mMembers->mGLContext = std::make_shared<EEGLContext>();
//                     mMembers->mGLContext->initContext(nullptr, false);
//                     mMembers->mGLContext->initSurface(nullptr, PIXEL_BUFFER_MODE);
//                     mMembers->mGLContext->makeCurrent();
//                 }));
//             }
//             return mMembers->mGLContext;
//         }
//        std::shared_ptr<EETextureAllocator> EEShared::getSharedTextureAllocator(){
//             std::unique_lock<std::mutex> lock(mMembers->mMutex);
//             if(mMembers->mTextureAllocator == nullptr){
//                 mMembers->mTextureAllocator = std::make_shared<EETextureAllocator>();
//             }
//            return mMembers->mTextureAllocator;
//         }
//        std::shared_ptr<EETaskQueue> EEShared::getSharedTaskQueue(){
//            std::unique_lock<std::mutex> lock(mMembers->mMutex);
//            if(mMembers->mTaskQueue == nullptr){
//                mMembers->mTaskQueue = std::make_shared<EETaskQueue>(EECommonUtils::createGlobalName(mMembers->mName));
//            }
//            return mMembers->mTaskQueue;
//         }


}