#ifndef RANDOMTEXTURE_H
#define RANDOMTEXTURE_H
#include <glad/gl.h>

class RandomTexture {
 public:
  RandomTexture();
  virtual ~RandomTexture();
  RandomTexture(const RandomTexture&) = delete;
  RandomTexture& operator=(const RandomTexture&) = delete;
  RandomTexture(RandomTexture&&) = delete;
  RandomTexture& operator=(RandomTexture&&) = delete;
  bool InitRandomTexture(GLsizei size);
  void Bind(GLenum TextureUnit) const;

 protected:
 private:
  GLuint textureID{0};
};

#endif  // RANDOMTEXTURE_H
