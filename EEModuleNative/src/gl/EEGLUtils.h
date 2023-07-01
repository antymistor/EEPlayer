//
// Created by ByteDance on 2023/6/4.
//

#ifndef CVTEXTREADER_EEGLUTILS_H
#define CVTEXTREADER_EEGLUTILS_H
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../basedefine/EECodeDefine.h"
#include <string>

namespace EE {
    class EEGLUtils {
    public:
        struct TextureBuildParam{
            GLenum textype = GL_TEXTURE_2D;
            GLint  upfilter = GL_NEAREST;
            GLint  downfilter = GL_NEAREST;
            GLint  wrap_s = GL_CLAMP_TO_EDGE;
            GLint  wrap_t = GL_CLAMP_TO_EDGE;
            void* pdata = nullptr;
            EESize size;
            TextureBuildParam(EESize s = {0,0} , void* data = nullptr){
                size = s;
                pdata = data;
            }
            int32_t getHash() const{
                return size.width    | size.height << 1 | textype << 2 |
                       upfilter << 3 | downfilter  << 4 | wrap_s  << 5 |
                       wrap_t << 6;
            }
        };
        static GLuint createTexture(const TextureBuildParam& para);
        static void copy2DTexture(GLuint nSrcID, GLuint nDstID, EERect rect);
        static void readTexture(GLuint nSrcI, EERect rect, void* data, GLenum textype = GL_TEXTURE_2D);
        static void deleteTexture(GLuint tex);
        static void writeTextureToBmp(const char* path, const GLuint& tex, const EESize& size, GLenum textype = GL_TEXTURE_2D);
    };
}


#endif //CVTEXTREADER_EEGLUTILS_H
