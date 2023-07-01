//
// Created by ByteDance on 2023/6/9.
//

#ifndef CVTEXTREADER_EEGLSHADERLIB_H
#define CVTEXTREADER_EEGLSHADERLIB_H
#define SHADER_STRING(...) #__VA_ARGS__
#include <vector>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../basedefine/EECodeDefine.h"
namespace EE{

    using EEShaderString = const char *;
    enum ShaderIndexType {
        Attribute,
        Uniform,
    };
    enum ShaderValueType {
        INT_1,
        INT_2,
        FLOAT_1,
        FLOAT_2,
        FLOAT_4,
        FLOAT_ARRAY_V,
        MATRI_4_V,
    };

    static GLfloat EE_VERTEXCOORDS[8] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
    };

    static GLfloat EE_TEXTURECOORDS[8] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
    };
    static GLfloat EE_TEXTURECOORDS_FLIP_Y[8] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
    };

    struct ShaderIndex{
        const char *name;
        ShaderIndexType indexType;
        ShaderValueType valueType;
        GLint mIndex = -1;
        ShaderIndex(const char *name_, const ShaderIndexType indextype_, const ShaderValueType valuetype_):
                    name(name_),indexType(indextype_),valueType(valuetype_){}
        static EEReturnCode makeShaderIndex(GLuint program,  ShaderIndex* const index);
        static EEReturnCode fillShaderIndex(ShaderIndex* const index, ...);
        static EEReturnCode disableAttribIndex(ShaderIndex* const index);
    };

    class EEGLProgram;
    struct EEShaderCode {
        EEShaderString code;
        std::vector<ShaderIndex> shaderIndexVector;
        EEReturnCode fetchIndex(std::shared_ptr<EEGLProgram>  program);
    };



    const EEShaderCode baseVertexCode{
            .code = SHADER_STRING(
                        attribute vec2 aPosition;
                        attribute vec2 aTexCoord;
                        uniform mat4 uModelMatrix;
                        varying vec2 vTexCoord;
                        void main() {
                            vTexCoord = aTexCoord;
                            gl_Position = uModelMatrix * vec4(aPosition, 0.0, 1.0);
                        }
                    ),
            .shaderIndexVector = {{"aPosition",    Attribute, FLOAT_ARRAY_V},
                                  {"aTexCoord",    Attribute, FLOAT_ARRAY_V},
                                  {"uModelMatrix", Uniform,   MATRI_4_V}}
    };

    const EEShaderCode base2DFragmentCode{
            .code = SHADER_STRING(
                            precision mediump float;
                            varying vec2 vTexCoord;
                            uniform sampler2D uTexture;
                            void main() {
                                gl_FragColor = texture2D(uTexture, vTexCoord);
                            }
                    ),
            .shaderIndexVector = {{"uTexture",    Uniform, INT_1}}
    };

    const EEShaderCode baseOESFragmentCode{
            .code =    "#extension GL_OES_EGL_image_external : require\n"
                       SHADER_STRING(
                        precision mediump float;
                        varying vec2 vTexCoord;
                        uniform samplerExternalOES uTexture;
                        void main() {
                            gl_FragColor = texture2D(uTexture, vTexCoord);
                        }
                    ),
            .shaderIndexVector = {{"uTexture",    Uniform, INT_1}}
    };

    const EEShaderCode sharp2DFragmentCode{
            .code = SHADER_STRING(
                    precision mediump float;
                    varying vec2 vTexCoord;
                    uniform sampler2D uTexture;
                    uniform float uTexCoordWidth;
                    uniform float uTexCoordHeight;
                    uniform float ublurredRatio;
                    uniform float ublurredRadium;
                    void main() {
                        gl_FragColor = (1.0 - 4.0 * ublurredRatio) * texture2D(uTexture, vTexCoord) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( ublurredRadium/uTexCoordWidth, ublurredRadium/uTexCoordHeight)) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( ublurredRadium/uTexCoordWidth, -ublurredRadium/uTexCoordHeight)) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( -ublurredRadium/uTexCoordWidth, ublurredRadium/uTexCoordHeight)) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( -ublurredRadium/uTexCoordWidth, -ublurredRadium/uTexCoordHeight));
                    }
            ),
            .shaderIndexVector = {{"uTexture",       Uniform, INT_1},
                                  {"uTexCoordWidth", Uniform, FLOAT_1},
                                  {"uTexCoordHeight", Uniform, FLOAT_1},
                                  {"ublurredRatio", Uniform, FLOAT_1},
                                  {"ublurredRadium", Uniform, FLOAT_1}}
    };

    const EEShaderCode gauss2DFragmentCode{
            .code = SHADER_STRING(
                    precision mediump float;
                    varying vec2 vTexCoord;
                    uniform sampler2D uTexture;
                    uniform float uTexCoordWidth;
                    uniform float uTexCoordHeight;
                    uniform float ublurredRatio;
                    uniform float ublurredRadium;
                    void main() {
                        gl_FragColor = (1.0 - 12.0 * ublurredRatio) * texture2D(uTexture, vTexCoord) +
                                       2.0 * ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( 0.0, ublurredRadium/uTexCoordHeight)) +
                                       2.0 * ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( 0.0, -ublurredRadium/uTexCoordHeight)) +
                                       2.0 * ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( -ublurredRadium/uTexCoordWidth, 0.0)) +
                                       2.0 * ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( -ublurredRadium/uTexCoordWidth, 0.0))+
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( ublurredRadium/uTexCoordWidth, ublurredRadium/uTexCoordHeight)) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( ublurredRadium/uTexCoordWidth, -ublurredRadium/uTexCoordHeight)) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( -ublurredRadium/uTexCoordWidth, ublurredRadium/uTexCoordHeight)) +
                                       ublurredRatio *  texture2D(uTexture, vTexCoord + vec2( -ublurredRadium/uTexCoordWidth, -ublurredRadium/uTexCoordHeight));
                    }
            ),
            .shaderIndexVector = {{"uTexture",       Uniform, INT_1},
                                  {"uTexCoordWidth", Uniform, FLOAT_1},
                                  {"uTexCoordHeight", Uniform, FLOAT_1},
                                  {"ublurredRatio", Uniform, FLOAT_1},
                                  {"ublurredRadium", Uniform, FLOAT_1}}
    };
}
#endif //CVTEXTREADER_EEGLSHADERLIB_H
