#ifndef AULA_OPENGL_GFX_TEXTURE_HPP
#define AULA_OPENGL_GFX_TEXTURE_HPP

#include <glad/gl.h>

#include <string>

// =============================================================================
//  Texture: imagem (PNG/JPG) carregada com stb_image e enviada para a GPU.
// =============================================================================
//
//  O construtor lê o arquivo, cria a textura OpenGL, gera os mipmaps e configura wrap e
//  filtro. Se a leitura ou criação na GPU falhar, o erro vai para o console e a textura
//  fica inválida: id e dimensões zerados, isValid() == false e bind() não faz nada.
//  Construção, bind e destruição exigem contexto OpenGL ativo e GLAD carregado.
//  O construtor consulta glGetError(): erros preexistentes são registrados separadamente
//  e consumidos para isolar o diagnóstico. A API OpenGL não permite restaurar esses flags.
//
//  Wrap (o que acontece com UV fora de 0..1):
//   - Repeat (padrão): a textura se repete. É o que faz o tiling das primitivas funcionar
//     (piso de 4 m com textureSize = 1 tem UV de 0 a 4 e mostra a imagem 4 vezes).
//   - ClampToEdge: a borda da imagem é esticada. Para imagens que aparecem uma vez só,
//     inteiras (o slide, o quadro): evita que a borda oposta "vaze" no filtro linear.
//
//  Filtro:
//   - Ampliação (objeto perto, 1 texel cobre vários pixels): GL_LINEAR, mistura os 4 texels
//     vizinhos e a imagem não fica quadriculada.
//   - Redução (objeto longe, vários texels num pixel): GL_LINEAR_MIPMAP_LINEAR, usa as cópias
//     reduzidas (mipmaps) e mistura os dois níveis mais próximos. Sem mipmap, um piso visto de
//     longe "cintila", porque cada pixel pega um texel quase aleatório da imagem inteira.
//
//  Uso: na cena, a textura entra por um Material (include/gfx/material.hpp), que faz o bind:
//      Texture madeira("assets/textures/madeira.png");
//      material.diffuseMap = &madeira;
//  O que Material::apply faz por baixo:
//      madeira.bind(0);                                 // unidade de textura 0
//      shader.setInt("uMaterial.diffuseMap", 0);        // o sampler2D lê da unidade 0
//      mesh.draw();
class Texture {
public:
    enum class Wrap { Repeat, ClampToEdge };

    explicit Texture(const std::string& path, Wrap wrap = Wrap::Repeat);
    ~Texture();

    // Como o Shader e o Mesh: uma cópia apagaria a mesma textura no destrutor.
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    bool isValid() const { return id_ != 0; }
    GLuint id() const { return id_; }
    int width() const { return width_; }
    int height() const { return height_; }

    // Liga a textura numa unidade (0, 1, 2...). O sampler2D do shader recebe o NÚMERO da
    // unidade via setInt, não o id da textura.
    // Unidades fora do limite do contexto são ignoradas sem alterar estado OpenGL,
    // com um aviso no máximo por instância.
    void bind(int unit = 0) const;

private:
    GLuint id_ = 0;
    int width_ = 0;
    int height_ = 0;
    mutable bool warnedInvalidUnit_ = false;
};

#endif  // AULA_OPENGL_GFX_TEXTURE_HPP
