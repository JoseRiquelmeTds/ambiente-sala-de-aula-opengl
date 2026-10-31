#ifndef AULA_OPENGL_GFX_SHADER_HPP
#define AULA_OPENGL_GFX_SHADER_HPP

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

// Programa de shader (vertex + fragment) lido de arquivos .vert/.frag.
//
// O construtor lê os dois arquivos, compila cada um e linka o programa. Se algo falhar,
// o erro do driver vai para o console com o nome do arquivo (ex.: "shaders/basic.frag")
// e o programa fica inválido: isValid() retorna false e use() não faz nada.
//
// Uso:
//     Shader shader("shaders/basic.vert", "shaders/basic.frag");
//     shader.use();
//     shader.setMat4("uModel", model);
class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    // Um programa do OpenGL não pode ser copiado: a cópia apagaria o mesmo id no destrutor.
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool isValid() const { return program_ != 0; }
    GLuint id() const { return program_; }

    // Torna este o programa ativo. Os set* abaixo valem para o programa ativo,
    // então chame use() antes deles.
    void use() const;

    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    GLint location(const std::string& name) const;

    GLuint program_ = 0;
    mutable std::unordered_map<std::string, GLint> locations_;  // cache nome -> location
};

#endif  // AULA_OPENGL_GFX_SHADER_HPP
