#include "gfx/shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>

namespace {

// Lê o arquivo inteiro para uma string. Retorna false se não conseguir abrir.
bool readFile(const std::string& path, std::string& out) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

// Compila um estágio (vertex ou fragment). Retorna 0 em caso de erro, já
// imprimindo o log do driver. O log vem no formato "0(12) : error ...", onde 12
// é a linha dentro do arquivo indicado em path.
GLuint compileStage(GLenum type, const std::string& path) {
    std::string source;
    if (!readFile(path, source)) {
        std::fprintf(stderr,
                     "[Shader] Não foi possível abrir '%s'.\n"
                     "         Rode o programa a partir da raiz do repositório (cmake --build ... --target run).\n",
                     path.c_str());
        return 0;
    }

    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength > 0 ? logLength : 1, '\0');
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());

        const char* stage = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        std::fprintf(stderr, "[Shader] Erro ao compilar o %s shader '%s':\n%s\n", stage, path.c_str(), log.c_str());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

}  // namespace

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    GLuint vertex = compileStage(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = compileStage(GL_FRAGMENT_SHADER, fragmentPath);
    if (vertex == 0 || fragment == 0) {
        glDeleteShader(vertex);  // glDeleteShader(0) é ignorado pelo OpenGL
        glDeleteShader(fragment);
        return;
    }

    // Link: junta os estágios num programa e confere se as saídas do vertex shader
    // (out) batem com as entradas do fragment shader (in).
    program_ = glCreateProgram();
    glAttachShader(program_, vertex);
    glAttachShader(program_, fragment);
    glLinkProgram(program_);

    // Depois do link os objetos de shader não são mais necessários.
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength > 0 ? logLength : 1, '\0');
        glGetProgramInfoLog(program_, static_cast<GLsizei>(log.size()), nullptr, log.data());

        std::fprintf(stderr, "[Shader] Erro ao linkar '%s' + '%s':\n%s\n", vertexPath.c_str(), fragmentPath.c_str(),
                     log.c_str());
        glDeleteProgram(program_);
        program_ = 0;
    }
}

Shader::~Shader() {
    glDeleteProgram(program_);
}

void Shader::use() const {
    glUseProgram(program_);
}

GLint Shader::location(const std::string& name) const {
    // Guarda a location de cada nome: glGetUniformLocation é uma busca por string no
    // driver e os set* são chamados todo frame.
    auto cached = locations_.find(name);
    if (cached != locations_.end()) {
        return cached->second;
    }

    // -1 quando o uniform não existe (erro de digitação?) ou foi removido pelo compilador
    // por não ser usado. glUniform* com -1 é ignorado, então não trava nada: só avisamos,
    // uma vez por nome, para ajudar a depurar.
    GLint loc = glGetUniformLocation(program_, name.c_str());
    if (loc == -1 && program_ != 0) {
        std::fprintf(stderr, "[Shader] Aviso: uniform '%s' não encontrado (não existe ou não é usado).\n",
                     name.c_str());
    }
    locations_[name] = loc;
    return loc;
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(location(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(location(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(location(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(location(name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    // GL_FALSE: o GLM já guarda as matrizes por coluna, como o OpenGL espera.
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
}
