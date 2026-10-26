#ifndef AULA_OPENGL_OBJECTS_PROJECTOR_HPP
#define AULA_OPENGL_OBJECTS_PROJECTOR_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// =============================================================================
//  Projector: projetor preso ao teto (CG-65).
// =============================================================================
//
//  Metros, Y para cima, frente (a lente) para +Z. Origem no ponto de fixação no teto: o
//  projetor fica pendurado em -Y e o placement só precisa pôr a origem no teto.
//
//      raiz ─┬─ placa do teto e haste (fixas)
//            └─ cabeça: T(ponta da haste) · Rx(inclinação), pivô da inclinação
//                 ├─ suporte e corpo (0,30 x 0,10 x 0,25 m)
//                 └─ lente: peça e mesh próprios, para a CG-77 dar brilho só a ela
//
//  Inclinação positiva abaixa a frente (regra da mão direita em +X). aimAt calcula a
//  inclinação para o eixo da lente passar por um ponto. O giro em Y (para onde o projetor
//  aponta na sala) é do placement.
//
//  Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class Projector {
public:
    Projector();

    Projector(const Projector&) = delete;
    Projector& operator=(const Projector&) = delete;

    // Uma instância por Projector. Chamadas seguintes devolvem a raiz já anexada, sem duplicá-la.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);

    // Inclina a cabeça para o eixo da lente passar por target, dado no espaço do projetor
    // (use a inversa do placement para trazer um ponto do mundo). Só o plano YZ conta. Alvo
    // mais perto do pivô do que o eixo da lente não tem solução e é ignorado.
    // Pode ser chamado antes de addTo.
    void aimAt(const glm::vec3& target);
    float tiltDegrees() const;

    // Centro da face da frente da lente e direção do eixo, no espaço do projetor, já com a
    // inclinação: origem e direção da projeção (CG-77).
    glm::vec3 lensCenter() const;
    glm::vec3 lensDirection() const;

private:
    glm::mat4 headTransform() const;

    Mesh ceilingPlate_;
    Mesh rod_;
    Mesh bracket_;
    Mesh body_;
    Mesh lensMesh_;
    // Não donos: nós criados em addTo, que vivem na árvore do pai.
    SceneNode* root_ = nullptr;
    SceneNode* head_ = nullptr;
    SceneNode* lens_ = nullptr;  // a CG-77 troca o material daqui
    float tiltRadians_ = 0.0f;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_PROJECTOR_HPP
