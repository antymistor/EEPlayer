//
// Created by antymistor on 2023/6/9.
//

#include "EEGLProgram.h"
#include "../basedefine/EECodeDefine.h"
#define LOG_TAG "EEGLProgram"
namespace EE{

    EEGLProgram::~EEGLProgram() {
        glDeleteProgram(mProgram);
        glDeleteShader(mVertexShaderID);
        glDeleteShader(mFragShaderID);
    }

    EEReturnCode EEGLProgram::compile(const std::shared_ptr<EEShaderCode>& vertexShader , const std::shared_ptr<EEShaderCode>& fragShader){
        mProgram = glCreateProgram();

        //Build VertexShader
        mVertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(mVertexShaderID, 1, &vertexShader->code, nullptr);
        glCompileShader(mVertexShaderID);
        GLint compileSatuts;
        glGetShaderiv(mVertexShaderID, GL_COMPILE_STATUS, &compileSatuts);
        if (compileSatuts != GL_TRUE) {
            GLint compileLogLength = 0;
            glGetShaderiv(mVertexShaderID, GL_INFO_LOG_LENGTH, &compileLogLength);
            if (compileLogLength > 0) {
                auto *compileLog = new GLchar[compileLogLength];
                glGetShaderInfoLog(mVertexShaderID, compileLogLength, &compileLogLength,compileLog);
                LOGE("log VertexShader info = %s", compileLog)
                delete[] compileLog;
            }
            return EE_FAIL;
        }

        //Build FragShader
        mFragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(mFragShaderID, 1, &fragShader->code, nullptr);
        glCompileShader(mFragShaderID);
        glGetShaderiv(mFragShaderID, GL_COMPILE_STATUS, &compileSatuts);
        if (compileSatuts != GL_TRUE) {
            GLint compileLogLength = 0;
            glGetShaderiv(mFragShaderID, GL_INFO_LOG_LENGTH, &compileLogLength);
            if (compileLogLength > 0) {
                auto *compileLog = new GLchar[compileLogLength];
                glGetShaderInfoLog(mFragShaderID, compileLogLength, &compileLogLength,compileLog);
                LOGE("log FragShader info = %s", compileLog)
                delete[] compileLog;
            }
            return EE_FAIL;
        }

        glAttachShader(mProgram, mVertexShaderID);
        glAttachShader(mProgram, mFragShaderID);
        return EE_OK;
    }

    EEReturnCode EEGLProgram::link(){
        glLinkProgram(mProgram);
        GLint linkStatus;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            GLint linkLogLength;
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &linkLogLength);
            if (linkLogLength > 0) {
                auto *linklog = new GLchar[linkLogLength];
                glGetProgramInfoLog(mProgram, linkLogLength, &linkLogLength, linklog);
                LOGE("log link info = %s", linklog)
                delete[] linklog;
            }
            return EE_FAIL;
        }
        misLinked = true;
        return EE_OK;
    }

    bool EEGLProgram::isLinked() const{return misLinked;}

    GLuint EEGLProgram::getProgramID() const{
        return mProgram;
    }

    void EEGLProgram::use() const{
        glUseProgram(mProgram);
    }

}