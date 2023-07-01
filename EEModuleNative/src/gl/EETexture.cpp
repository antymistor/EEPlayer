//
// Created by ByteDance on 2023/6/5.
//

#include "EETexture.h"
#include <mutex>
#include <unordered_map>
namespace EE {

    std::mutex mMutexGlobal;
    std::shared_ptr<EETextureAllocator> EETextureAllocator::getSharedInstance(){
        static std::shared_ptr<EETextureAllocator> globalInstance = nullptr;
        if(globalInstance == nullptr){
            std::unique_lock<std::mutex> lock(mMutexGlobal);
            if(globalInstance == nullptr) {
                globalInstance = std::make_shared<EETextureAllocator>();
            }
        }
        return globalInstance;
    }

    struct EETextureAllocator::EETextureAllocatorMembers{
        unsigned MaxCapcity = 10;
        EETextureContainer TextureMap;
        std::mutex mMutex;
    };

    EETextureAllocator::EETextureAllocator(int initMaxCapcity) : mMembers(std::make_unique<EETextureAllocatorMembers>()){
        mMembers->MaxCapcity = initMaxCapcity;
    }

    void EETextureAllocator::clearAll(){
        mMembers->TextureMap.clear();
    }

    std::shared_ptr<EETexture> EETextureAllocator::allocateEETexture(const EE::EEGLUtils::TextureBuildParam& param){
        auto hash = param.getHash();
        std::unique_lock<std::mutex> lock(mMembers->mMutex);
        auto iter = mMembers->TextureMap.equal_range(hash);
        auto it = iter.first;
        while (it != iter.second){
            if(it->second.use_count() == 1){
                return it->second;
            }
            it++;
        }

        if(mMembers->TextureMap.size() >= mMembers->MaxCapcity){
            cleanIdleTexture();
        }

        if(mMembers->TextureMap.size() >= mMembers->MaxCapcity){
            mMembers->MaxCapcity = mMembers->MaxCapcity * 3 / 2;
        }

        auto sp_tex = std::shared_ptr<EETexture>(new EETexture(param));
        auto iter_tex = mMembers->TextureMap.emplace(hash, sp_tex);
        sp_tex->pushContextAllocator(shared_from_this(), iter_tex);

        return sp_tex;
    }

    void EETextureAllocator::unRegisterTexture(EETextureContainer::iterator iter){
        std::unique_lock<std::mutex> lock(mMembers->mMutex);
        mMembers->TextureMap.erase(iter);
    }

    void EETextureAllocator::cleanIdleTexture(){
        for (auto iter = mMembers->TextureMap.begin(); iter != mMembers->TextureMap.end(); ) {
            if(iter->second.use_count() == 1){
                iter = mMembers->TextureMap.erase(iter);
            }else{
                ++iter;
            }
        }
    }

    EETextureAllocator::~EETextureAllocator() = default;

    EETexture::EETexture(const EE::EEGLUtils::TextureBuildParam& param){
        mParam = param;
        mTexID = EEGLUtils::createTexture(mParam);
    }

    EE::EEGLUtils::TextureBuildParam EETexture::getParam() const{
        return mParam;
    }

    GLuint EETexture::getTexID() const{
        return mTexID;
    }

    void EETexture::excuteBind(bool isbind) const {
        glBindTexture(mParam.textype, isbind ? mTexID : 0);
    }

    EEReturnCode EETexture::unRegisterFromAllocator(){
       auto sp_allocator = mallocator.lock();
       if(sp_allocator){
           sp_allocator->unRegisterTexture(miter);
           return EE_OK;
       }
       return EE_FAIL;
    }

    void  EETexture::pushContextAllocator(const std::shared_ptr<EETextureAllocator>& allocator, EETextureContainer::iterator iter){
        mallocator = allocator;
        miter = iter;
    }

    EETexture::~EETexture(){
        EEGLUtils::deleteTexture(mTexID);
    }
}