#ifndef AULA_OPENGL_OBJECTS_FAN_HPP
#define AULA_OPENGL_OBJECTS_FAN_HPP

#include <glm/glm.hpp>

#include "gfx/material.hpp"
#include "gfx/mesh.hpp"
#include "gfx/scene_node.hpp"

namespace objects {

// CG-59: metros, Y para cima, frente +Z; origem no centro da face encostada na parede.
// CG-76: liga/desliga com aceleração e desaceleração; só as pás e a tampa (o rotor) giram, em torno de Z.
// Criar com contexto OpenGL ativo e manter vivo enquanto a árvore usar seus meshes.
class Fan {
public:
    Fan();

    Fan(const Fan&) = delete;
    Fan& operator=(const Fan&) = delete;

    // Uma instância por Fan. Chamadas seguintes devolvem a raiz já anexada, sem duplicá-la.
    SceneNode& addTo(SceneNode& parent, const glm::mat4& placement);
    // Graus, inclusive antes de addTo. Só altera a articulação das pás e da tampa.
    void setRotorAngle(float degrees);
    float rotorAngle() const { return rotorAngle_; }

    // Liga ou desliga o motor. O rotor não para nem arranca na hora: update acelera ou desacelera.
    void toggle();
    bool isOn() const { return on_; }
    // Próxima velocidade em ciclo (0 -> 1 -> 2 -> 0: 2, 3 e 4 voltas/s). Desligado, só guarda a escolha
    // para a próxima vez que ligar.
    void cycleSpeed();
    int speedLevel() const { return speedLevel_; }
    // Uma vez por frame, também desligado (a hélice continua girando até parar). deltaTime em segundos.
    void update(float deltaTime);

private:
    // Materiais vivem com os meshes; os nós fixos e móveis só guardam ponteiros.
    Mesh mount_;
    Mesh support_;
    Mesh motor_;
    Mesh shaft_;
    Mesh blade_;
    Mesh hub_;
    Mesh frontGuard_;
    Mesh rearGuard_;
    Material paintedMetal_;
    Material guardMetal_;
    Material motorPlastic_;
    Material bladePlastic_;
    Material hubPlastic_;
    SceneNode* root_ = nullptr;   // não donos; a árvore deve sobreviver às chamadas de setRotorAngle
    SceneNode* rotor_ = nullptr;
    float rotorAngle_ = 0.0f;
    float speed_ = 0.0f;   // velocidade angular atual do rotor, graus/s
    bool on_ = false;
    int speedLevel_ = 2;   // índice em kSpeeds (fan.cpp); começa na mais rápida
};

}  // namespace objects

#endif  // AULA_OPENGL_OBJECTS_FAN_HPP
