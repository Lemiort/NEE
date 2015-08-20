#ifndef MATERIAL_H
#define MATERIAL_H
#include <GL\glew.h>
#include "Texture.h"
#include "Shader.h"

class Material
{
    public:
        Material();
        virtual ~Material();
        bool Init(Shader* _sh);
        void Use();
        void SetColorTexture(Texture2D* _colorMap);
        void SetNormalTexture(Texture2D* _normalMap);
        void SetSpecularTexture(Texture2D* _specularMap);
        void SetShadowTexture(Texture2D* _shadowMap);
        void SetTexture(Texture2D*_map, GLuint num);
        void SetTexture(GLuint _map, GLuint num);
        Shader* GetShader()
        {
            return shaderProgram;
        }
    protected:
        bool ownColorMap, ownNormapMap, ownSpecularMap;
        bool ownShadowMap;
        Shader* shaderProgram;
        Texture2D *colorMap, *normalMap, *specularMap;
        Texture2D *shadowMap;
        AbstractTexture *abstractMap;
        GLuint colTexID, texBufferID,normBufferID,normSamplerID;
		GLuint specBufferID, specSamplerID;
		GLuint *abstractSamplersID;
		//GLuint colSamplerUI, normSamplerUI;
		GLuint shadowSamplerID;
        GLint max_texture_units;
		GLuint* texturesID;
};

#endif // MATERIAL_H
