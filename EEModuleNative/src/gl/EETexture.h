//
// Created by antymistor on 2023/6/5.
//

#ifndef CVTEXTREADER_EETEXTURE_H
#define CVTEXTREADER_EETEXTURE_H
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../basedefine/EECodeDefine.h"
#include "EEGLUtils.h"
#include <memory>
#include <unordered_map>
namespace EE {
    class EETexture;
    using EETextureContainer = std::unordered_multimap<int32_t , std::shared_ptr<EETexture>>;
class EETextureAllocator : public std::enable_shared_from_this<EETextureAllocator>{
        friend class EETexture;
        public:
            static std::shared_ptr<EETextureAllocator> getSharedInstance();
            EETextureAllocator(int initMaxCapcity = 10);
            std::shared_ptr<EETexture> allocateEETexture(const EE::EEGLUtils::TextureBuildParam& param);
            ~EETextureAllocator();
            void clearAll();

       private:
            void unRegisterTexture(EETextureContainer::iterator iter);
            void cleanIdleTexture();
            struct EETextureAllocatorMembers;
            std::unique_ptr<EETextureAllocatorMembers> mMembers;
    };
    class EETexture {
        friend class EETextureAllocator;
        public:
            GLuint getTexID() const;
            EE::EEGLUtils::TextureBuildParam getParam() const;
            ~EETexture();
            void excuteBind(bool isbind) const;
            EEReturnCode unRegisterFromAllocator();
        private:
            void pushContextAllocator(const std::shared_ptr<EETextureAllocator>& allocator, EETextureContainer::iterator iter);
            std::weak_ptr<EETextureAllocator> mallocator;
            EETextureContainer::iterator miter;
            EE::EEGLUtils::TextureBuildParam mParam;
            GLuint mTexID = 0;
            EETexture(const EE::EEGLUtils::TextureBuildParam& param);
    };
}


#endif //CVTEXTREADER_EETEXTURE_H
