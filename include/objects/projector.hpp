#ifndef AULA_OPENGL_OBJECTS_PROJECTOR_HPP
#define AULA_OPENGL_OBJECTS_PROJECTOR_HPP

#include <glm/glm.hpp>

#include <cstddef>
#include <memory>
#include <vector>

#include "gfx/material.hpp"
#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"
#include "gfx/texture.hpp"

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
//                 └─ lente: peça, mesh e material próprios (vidro escuro), que acende ligada (CG-77)
//
//  Inclinação positiva abaixa a frente (regra da mão direita em +X). aimAt calcula a
//  inclinação para o eixo da lente passar por um ponto. O giro em Y (para onde o projetor
//  aponta na sala) é do placement.
//
//  Materiais (CG-78): placa do teto e haste de metal grafite (na raiz); suporte e corpo de
//  plástico cinza com textura (plastico_projetor.png, na cabeça); lente de vidro escuro e
//  brilhante, com material próprio para a CG-77 acender só ela (emissive do lensGlass_).
//
//  Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes, a
//  textura e os materiais.
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
    // "Cima" da imagem projetada, no espaço do projetor: o +Y da cabeça, inclinado junto com ela
    // (perpendicular a lensDirection). É o up do lookAt da projeção (CG-77).
    glm::vec3 lensUp() const;

    // Liga ou desliga o projetor (CG-77). Aqui só a lente muda: ligada, ela brilha (emissive do
    // vidro). A luz que leva o slide até a tela é a Lighting::projector, que a scene/classroom liga
    // junto com esta chamada. Pode ser chamado antes de addTo.
    void setOn(bool on);
    void toggle();
    bool isOn() const { return on_; }

    // Slide que o projetor mostra (CG-77): imagens 4:3 carregadas no construtor. nullptr se
    // nenhuma carregou (o erro [Texture] já foi impresso); slide que falhou é pulado.
    const Texture* currentSlide() const;
    // Próximo / anterior, em ciclo (do último volta ao primeiro). Funciona desligado também: a imagem
    // nova aparece quando ligar.
    void nextSlide();
    void previousSlide();

private:
    glm::mat4 headTransform() const;

    Mesh ceilingPlate_;
    Mesh rod_;
    Mesh bracket_;
    Mesh body_;
    Mesh lensMesh_;

    // Os nós da árvore apontam para estes materiais, e o plástico aponta para a textura: a textura
    // vem antes do material na declaração, porque os membros são construídos nessa ordem.
    Texture plasticTexture_;
    Material metal_;      // placa do teto e haste
    Material plastic_;    // suporte e corpo
    Material lensGlass_;  // lente; o emissive acende com o projetor ligado (setOn, CG-77)
    // Slides (CG-77), donos. ClampToEdge: a imagem aparece uma vez só e a borda não pode vazar
    // para o lado oposto no filtro linear.
    std::vector<std::unique_ptr<Texture>> slides_;
    std::size_t currentSlide_ = 0;

    // Não donos: nós criados em addTo, que vivem na árvore do pai.
    SceneNode* root_ = nullptr;
    SceneNode* head_ = nullptr;
    SceneNode* lens_ = nullptr;  // aponta para lensGlass_
    float tiltRadians_ = 0.0f;
    bool on_ = false;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_PROJECTOR_HPP
