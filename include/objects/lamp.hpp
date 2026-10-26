#ifndef AULA_OPENGL_OBJECTS_LAMP_HPP
#define AULA_OPENGL_OBJECTS_LAMP_HPP

#include <glm/glm.hpp>

#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// Luminária de teto de sobrepor, com 2 tubos T8 de 1,20 m (CG-63). Metros, Y para cima.
// Pivô no ponto de fixação: a origem é o centro da face encostada no teto e o corpo desce para -Y.
// Os tubos ficam deitados ao longo de X. Medidas: 1,27 (X) x 0,075 (Y) x 0,25 m (Z).
// Criar com o contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class Lamp {
public:
    Lamp();

    // Os nós da árvore apontam para os meshes: uma cópia apontaria para os do original.
    Lamp(const Lamp&) = delete;
    Lamp& operator=(const Lamp&) = delete;

    // Pode ser chamada várias vezes (uma por luminária da sala): todas usam os mesmos meshes.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement) const;

private:
    // Um mesh por material da CG-72. O tubo fica separado porque é a parte que acende (CG-71/73).
    Mesh body_;       // calha em chapa de aço pintada
    Mesh reflector_;  // refletor em "W" e aletas, de alumínio
    Mesh tube_;       // vidro do tubo, usado pelos 2 tubos
    Mesh socket_;     // soquete de plástico, usado pelos 4 soquetes
};

// Interruptor de parede de uma tecla (CG-63), espelho de 0,08 x 0,12 m. Metros, Y para cima.
// Pivô no ponto de fixação: a origem é o centro da face encostada na parede; frente para +Z.
// A tecla bascula em torno de um eixo horizontal (paralelo a X), como uma tecla real.
// Criar com o contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class LightSwitch {
public:
    LightSwitch();

    LightSwitch(const LightSwitch&) = delete;
    LightSwitch& operator=(const LightSwitch&) = delete;

    // Uma instância por LightSwitch, porque ele guarda a articulação da tecla para setOn.
    // Chamadas seguintes devolvem a raiz já anexada, sem duplicá-la.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);

    // Só inclina a tecla (metade de cima afundada = ligado); acender a luz fica para a CG-73.
    // Pode ser chamada antes de addTo: a posição é aplicada quando a tecla for criada.
    void setOn(bool on);
    bool isOn() const { return on_; }

private:
    Mesh plate_;  // espelho
    Mesh key_;    // tecla basculante
    SceneNode* root_ = nullptr;      // não donos: nós da árvore de quem chamou addTo
    SceneNode* keyJoint_ = nullptr;  // pivô no eixo de basculamento da tecla
    bool on_ = false;
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_LAMP_HPP
