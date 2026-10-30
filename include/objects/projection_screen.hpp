#ifndef AULA_OPENGL_OBJECTS_PROJECTION_SCREEN_HPP
#define AULA_OPENGL_OBJECTS_PROJECTION_SCREEN_HPP

#include <glm/glm.hpp>

#include "gfx/material.hpp"
#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"
#include "gfx/texture.hpp"

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
//  Animação (CG-75): começa recolhida, para o quadro atrás aparecer. toggle() inverte o sentido
//  e update(deltaTime) avança um progresso linear no tempo (curso inteiro em ~2,5 s), suavizado
//  por smoothstep: acelera ao sair e freia ao chegar. Inverter no meio do curso é contínuo, porque
//  só o sentido do progresso muda. s fica entre kMinOpenAmount e kMaxOpenAmount.
//
//  UV da lona: a imagem inteira na face (textureSize = 0), para o slide da CG-77. A trama da lona
//  (lona_tela.png) repete pelo uvScale do material, a cada 25 cm nos dois eixos. Ao recolher, a
//  UV continua de 0 a 1; por isso o uvScale.y acompanha a abertura (6·s) e a trama não achata:
//  ela fica presa à barra de baixo e entra na caixa pelo topo.
//
//  Materiais (CG-78): caixa e suportes de plástico branco-gelo (na raiz); lona fosca branca com a
//  textura de trama (na articulação); barra inferior de alumínio, cordinha cinza-escura fosca e
//  puxador de plástico preto (na ponta).
//
//  Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes, a textura e
//  os materiais.
class ProjectionScreen {
public:
    ProjectionScreen();

    ProjectionScreen(const ProjectionScreen&) = delete;
    ProjectionScreen& operator=(const ProjectionScreen&) = delete;

    // Uma instância por ProjectionScreen. Chamadas seguintes devolvem a raiz já anexada, sem duplicá-la.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);

    // Limites da abertura: recolhida (só a barra na fenda) e lona inteira de 1,5 m.
    static constexpr float kMinOpenAmount = 0.0f;
    static constexpr float kMaxOpenAmount = 1.0f;

    // Salto imediato, sem animação; valores fora dos limites são limitados e não finitos, ignorados.
    // Pode ser chamado antes de addTo. Ajusta o progresso da animação para o update seguinte não
    // saltar, e o sentido fica "abrindo" se a tela já passou da metade do curso.
    void setOpenAmount(float amount);
    float openAmount() const { return openAmount_; }

    // Abrindo ou aberta: passa a recolher. Recolhendo ou recolhida: passa a abrir.
    void toggle();
    // Avança a animação deltaTime segundos; parada no limite, não faz nada.
    void update(float deltaTime);
    bool isOpening() const { return opening_; }

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

    // Os nós da árvore apontam para estes materiais, e a lona aponta para a textura: a textura vem
    // antes do material na declaração, porque os membros são construídos nessa ordem.
    Texture canvasTexture_;
    Material housingPlastic_;  // caixa e suportes laterais
    Material canvasMaterial_;  // lona; uvScale.y acompanha a abertura (applyOpenAmount)
    Material barMetal_;        // barra inferior
    Material cordMatte_;       // cordinha
    Material handlePlastic_;   // puxador

    // Não donos: nós criados em addTo, que vivem na árvore do pai.
    SceneNode* root_ = nullptr;
    SceneNode* canvasPivot_ = nullptr;
    SceneNode* bottom_ = nullptr;
    float openAmount_ = kMinOpenAmount;
    float progress_ = 0.0f;  // 0 a 1, linear no tempo; openAmount_ = smoothstep(progress_) entre os limites
    bool opening_ = false;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_PROJECTION_SCREEN_HPP
