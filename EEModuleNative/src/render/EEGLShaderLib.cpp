//
// Created by ByteDance on 2023/6/10.
//
#include "EEGLShaderLib.h"
#include "EEGLProgram.h"
namespace EE{
    EEReturnCode EEShaderCode::fetchIndex(std::shared_ptr<EEGLProgram> program){
        for(auto& iter : shaderIndexVector){
            ShaderIndex::makeShaderIndex(program->getProgramID(), &iter);
        }
        return EE_OK;
    }


   EEReturnCode ShaderIndex::makeShaderIndex(GLuint program, ShaderIndex* const index){
       if(nullptr == index){
           return EE_FAIL;
       }
       switch (index->indexType) {
           case Attribute: index->mIndex = glGetAttribLocation(program, index->name);  break;
           case Uniform  : index->mIndex = glGetUniformLocation(program, index->name); break;
       }
       return EE_OK;
   }
   EEReturnCode ShaderIndex::disableAttribIndex(ShaderIndex* const index){
       if(index->indexType == Attribute && index->valueType == FLOAT_ARRAY_V){
          glDisableVertexAttribArray(index->mIndex);
          return  EE_OK;
       }
       return EE_FAIL;
    }

   EEReturnCode ShaderIndex::fillShaderIndex(ShaderIndex* const index, ...){
       EEReturnCode ret = EE_OK;
       if(nullptr == index || index->mIndex < 0){
           return EE_FAIL;
       }
       va_list args;
       va_start(args, index);
       switch (index->indexType) {
           case Attribute:{
               switch (index->valueType) {
                   case FLOAT_ARRAY_V: {
                       glEnableVertexAttribArray(index->mIndex);
                       glVertexAttribPointer(index->mIndex, 2, GL_FLOAT, 0, 0, va_arg(args, GLfloat*));
                   } break;
                   default: ret = EE_FAIL;break;
               }
           }
           case Uniform  : {
               switch (index->valueType) {
                   case INT_1 :{
                       glUniform1i(index->mIndex, va_arg(args, GLint));
                   }break;
                   case FLOAT_1 :{
                       glUniform1f(index->mIndex, va_arg(args, double));
                   }break;
                   case INT_2 :{
                       GLint i1 = va_arg(args, GLint);
                       GLint i2 = va_arg(args, GLint);
                       glUniform2i(index->mIndex, i1, i2);
                   }break;
                   case FLOAT_2 :{
                       GLfloat f1 = va_arg(args, double);
                       GLfloat f2 = va_arg(args, double);
                       glUniform2f(index->mIndex, f1, f2);
                   }break;
                   case FLOAT_4 :{
                       GLfloat f1 = va_arg(args, double);
                       GLfloat f2 = va_arg(args, double);
                       GLfloat f3 = va_arg(args, double);
                       GLfloat f4 = va_arg(args, double);
                       glUniform4f(index->mIndex, f1, f2, f3, f4);
                   }break;
                   case MATRI_4_V :{
                       glUniformMatrix4fv(index->mIndex, 1, false, va_arg(args, GLfloat*));
                   }break;
                   default: ret = EE_FAIL;break;
               }
           }
       }
       va_end(args);
       return ret;
   }
}
