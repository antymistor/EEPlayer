//
// Created by ByteDance on 2023/6/9.
//

#ifndef CVTEXTREADER_EEGLPROGRAM_H
#define CVTEXTREADER_EEGLPROGRAM_H
#include "../basedefine/EECodeDefine.h"
#include <memory>
#include "EEGLShaderLib.h"
namespace EE {
    class EEGLProgram {
        friend struct EEShaderCode;
        public:
            ~EEGLProgram();
            EEReturnCode compile(const std::shared_ptr<EEShaderCode>& vertexShader , const std::shared_ptr<EEShaderCode>& fragShader);
            EEReturnCode link();
            bool isLinked() const;
            void use() const;
        private:
            GLuint getProgramID() const;
            GLuint mProgram = 0;
            GLuint mVertexShaderID = 0;
            GLuint mFragShaderID = 0;
            bool misLinked = false;
    };
}


#endif //CVTEXTREADER_EEGLPROGRAM_H
