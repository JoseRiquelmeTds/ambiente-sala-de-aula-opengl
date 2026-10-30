#ifndef AULA_OPENGL_GFX_RENDERER_HPP
#define AULA_OPENGL_GFX_RENDERER_HPP

class Camera;
class SceneNode;
class Texture;
struct Lighting;

// Pipeline de desenho: estado do OpenGL, shader e matrizes da câmera. Não conhece o conteúdo
// da cena: desenha qualquer árvore de SceneNode que receber (montada em scene/classroom).
namespace renderer {

// Estado inicial do OpenGL (cor de fundo etc.). Chamar depois de carregar o GLAD.
void init();

// Desenha um quadro. Chamado uma vez por iteração do loop principal.
// texture vale para a cena inteira (nullptr = nenhuma) até existir material por peça (CG-72).
// lighting são as luzes do frame (include/gfx/light.hpp), montadas por quem conhece a cena.
void draw(const Camera& camera, const SceneNode& root, const Texture* texture, const Lighting& lighting);

// Ajusta o viewport ao tamanho do framebuffer (em pixels, não em coordenadas de tela).
void resize(int width, int height);

// Libera o shader. Chamar antes de destruir a janela, enquanto o contexto existe.
void shutdown();

}  // namespace renderer

#endif  // AULA_OPENGL_GFX_RENDERER_HPP
