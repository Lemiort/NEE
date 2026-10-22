#ifndef RANDOMTEXTURE_H
#define RANDOMTEXTURE_H
#include <glad/gl.h>

class RandomTexture {
 public:
  RandomTexture();
  virtual ~RandomTexture();
  bool InitRandomTexture(GLsizei size);
  void Bind(GLenum TextureUnit) const;

 protected:
 private:
  GLuint textureID{0};
};

#endif  // RANDOMTEXTURE_H
