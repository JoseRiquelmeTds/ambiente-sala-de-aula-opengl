#ifndef AULA_OPENGL_GFX_SCENE_NODE_HPP
#define AULA_OPENGL_GFX_SCENE_NODE_HPP

#include <glm/glm.hpp>

#include <cstddef>
#include <memory>
#include <vector>

#include "gfx/material.hpp"
#include "gfx/mesh.hpp"
#include "gfx/shader.hpp"

// =============================================================================
//  SceneNode: uma peça de um objeto composto, com transformação própria e filhos.
// =============================================================================
//
//  `transform` é a posição/rotação/escala da peça EM RELAÇÃO AO PAI (no espaço do pai).
//  No draw, a model de cada peça é:
//
//      model_peça = model_pai · transform
//
//  Lido da direita para a esquerda no vértice: primeiro a transformação da própria peça,
//  depois a do pai. Por isso, mover o pai leva os filhos junto, mas girar um filho
//  (as hélices) não mexe no pai (o corpo do ventilador).
//
//  Uma peça sem mesh (mesh = nullptr) é só um ponto de articulação: serve de pivô para os
//  filhos. Ex.: ventilador
//
//      SceneNode ventilador(&corpoMesh, T(posição_na_parede));
//      SceneNode& helices = ventilador.addChild(nullptr);   // pivô no centro das hélices
//      helices.addChild(&pasMesh);                          // pás, em volta do pivô
//      ...
//      // todo frame (ângulo += velocidade * deltaTime):
//      helices.transform = glm::rotate(T(centro_helice), ângulo, eixo);
//
//  Ex.: tela retrátil com o mesh criado com o topo em y = 0 (crescendo para -Y):
//
//      tela.transform = T(topo) · S(1, escalaY, 1);
//      tela.visible = escalaY > 0.001f;   // com escala 0 a normal matrix é singular
//
//  Um mesh pode ser usado por várias peças (as 4 pernas da cadeira apontam para o mesmo
//  Mesh): a peça só guarda um ponteiro e quem cria o Mesh continua sendo o dono dele, então
//  o Mesh tem que existir enquanto as peças forem desenhadas.
//
//  Material (CG-72) funciona igual: ponteiro não dono e HERDADO. Peça com material = nullptr
//  usa o do pai (que pode ter herdado do avô...). Basta definir na raiz do objeto e sobrescrever
//  nas peças diferentes:
//
//      cadeira.material = &madeira;     // assento, encosto e tudo abaixo
//      perna.material = &metal;         // só a perna e os filhos dela
class SceneNode {
public:
    explicit SceneNode(const Mesh* mesh = nullptr, const glm::mat4& transform = glm::mat4(1.0f));

    // Cria um filho e devolve uma referência para ele (para montar netos ou animar depois).
    // A referência continua válida mesmo adicionando mais filhos.
    SceneNode& addChild(const Mesh* mesh, const glm::mat4& transform = glm::mat4(1.0f));

    // Desenha esta peça e todos os descendentes. Chamar com o shader já em uso (shader.use()).
    // Define os uniforms "uModel", "uNormalMatrix" e "uMaterial" de cada peça antes de desenhá-la.
    // inheritedMaterial é o material do pai; na raiz, o padrão de quem desenha (o renderer).
    void draw(const Shader& shader, const Material& inheritedMaterial,
              const glm::mat4& parentModel = glm::mat4(1.0f)) const;

    // Filhos diretos, na ordem em que foram criados (ex.: a inspeção esconde todos menos um).
    std::size_t childCount() const { return children_.size(); }
    SceneNode& child(std::size_t index) { return *children_[index]; }

    const Mesh* mesh;
    glm::mat4 transform;
    const Material* material = nullptr;  // não dono; nullptr = herda o do pai
    bool visible = true;  // false esconde a peça e os filhos (ex.: tela totalmente recolhida)

private:
    // unique_ptr: cada filho fica num endereço fixo, então as referências devolvidas por
    // addChild não quebram quando o vector cresce e realoca.
    std::vector<std::unique_ptr<SceneNode>> children_;
};

#endif  // AULA_OPENGL_GFX_SCENE_NODE_HPP
