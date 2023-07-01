//
// Created by antymistor on 2023/6/4.
//

#include "EEGLUtils.h"
#include "../utils/EEMmapUtils.h"
namespace EE {
    GLuint EEGLUtils::createTexture(const TextureBuildParam& para){
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(para.textype, tex);
        if(para.textype == GL_TEXTURE_2D){
           glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, para.size.width, para.size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, para.pdata);
        }
        glTexParameteri(para.textype, GL_TEXTURE_MAG_FILTER, para.upfilter);
        glTexParameteri(para.textype, GL_TEXTURE_MIN_FILTER, para.downfilter);
        glTexParameteri(para.textype, GL_TEXTURE_WRAP_S, para.wrap_s);
        glTexParameteri(para.textype, GL_TEXTURE_WRAP_T, para.wrap_t);
        return tex;
    }

    void EEGLUtils::copy2DTexture(GLuint nSrcID, GLuint nDstID, EERect rect){
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nSrcID, 0);
        glBindTexture(GL_TEXTURE_2D, nDstID);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, rect.pos_x, rect.pos_y, 0, 0, rect.width, rect.height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }
    }

    void EEGLUtils::readTexture(GLuint nSrcI, EERect rect, void* data, GLenum textype){
        GLint old_fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(textype, nSrcI);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textype, nSrcI, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE){
            return;
        }
        glReadPixels(rect.pos_x, rect.pos_y, rect.width, rect.height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
        glDeleteFramebuffers(1, &fbo);
    }

    void EEGLUtils::deleteTexture(GLuint tex) {
        if (glIsTexture(tex) == GL_TRUE) {
            glDeleteTextures(1, &tex);
        }
    }

   void EEGLUtils::writeTextureToBmp(const char* path, const GLuint& tex, const EESize& size, GLenum textype){
           EE::EEMmapUtils::mmap_write_rgba_to_bitmap(path,[&](char *buf){
               EE::EEGLUtils::readTexture(  tex, size, buf, textype);
               return EE::EE_OK;
           }, size.width , size.height );
    }

}