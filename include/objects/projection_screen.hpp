#ifndef AULA_OPENGL_OBJECTS_PROJECTION_SCREEN_HPP
#define AULA_OPENGL_OBJECTS_PROJECTION_SCREEN_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// =============================================================================
//  ProjectionScreen: tela de projeção retrátil, de puxar (CG-66).
// =============================================================================
//
//  Metros, Y para cima, frente (a face da lona) para +Z. Origem no centro da face de trás
//  da caixa, a que encosta na parede: o placement só precisa pôr esse ponto na parede.
//
//      raiz ─┬─ caixa e suportes laterais (fixos)
//            ├─ articulação da lona: T(fenda) · S(1, s, 1), pivô no TOPO da lona
//            │    └─ lona de 2,0 x 1,5 m, de y = -1,5 a 0 no espaço da articulação
//            └─ ponta: T(fenda + (0, -1,5 · s, 0))
//                 └─ barra inferior, cordinha e puxador
//
//  s = openAmount(): 1 = aberta, 0 = recolhida. A escala em Y acontece em volta do pivô, que
//  está no topo da lona, então ela recolhe para cima sem descolar da caixa. A barra fica num nó
//  à parte porque um filho herda a escala do pai: embaixo da articulação ela seria achatada.
//
//  UV da lona: a imagem inteira na face (textureSize = 0), para o slide da CG-77. Ao recolher,
//  a UV continua de 0 a 1 e a imagem achata em Y. A distorção é aceita: a lona é lisa e o slide
//  só aparece com a tela aberta.
//
//  Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class ProjectionScreen {
public:
    ProjectionScreen();

    ProjectionScreen(const ProjectionScreen&) = delete;
    ProjectionScreen& operator=(const ProjectionScreen&) = delete;

    // Uma instância por ProjectionScreen. Chamadas seguintes devolvem a raiz já anexada, sem duplicá-la.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);

    // 0 (recolhida) a 1 (aberta); valores fora disso são limitados e não finitos, ignorados.
    // Pode ser chamado antes de addTo. A animação e a tecla T ficam para a CG-75.
    void setOpenAmount(float amount);
    float openAmount() const { return openAmount_; }

    // Centro da face da frente da lona aberta, no espaço da tela: o alvo do projetor (CG-65).
    glm::vec3 canvasCenter() const;

private:
    void applyOpenAmount();

    Mesh housing_;
    Mesh endCap_;  // usado pelos dois suportes laterais
    Mesh canvas_;
    Mesh bottomBar_;
    Mesh cord_;
    Mesh handle_;
    // Não donos: nós criados em addTo, que vivem na árvore do pai.
    SceneNode* root_ = nullptr;
    SceneNode* canvasPivot_ = nullptr;
    SceneNode* bottom_ = nullptr;
    float openAmount_ = 1.0f;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_PROJECTION_SCREEN_HPP
