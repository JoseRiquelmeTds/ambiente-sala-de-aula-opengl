#include "gfx/scene_node.hpp"

SceneNode::SceneNode(const Mesh* mesh, const glm::mat4& transform) : mesh(mesh), transform(transform) {}

SceneNode& SceneNode::addChild(const Mesh* mesh, const glm::mat4& transform) {
    children_.push_back(std::make_unique<SceneNode>(mesh, transform));
    return *children_.back();
}

void SceneNode::draw(const Shader& shader, const Material& inheritedMaterial, const glm::mat4& parentModel) const {
    if (!visible) {
        return;
    }

    // Leva o vértice do espaço desta peça para o do pai e, pela parentModel, até o mundo.
    const glm::mat4 model = parentModel * transform;

    // Herança: sem material próprio, a peça (e quem herdar dela) usa o do pai.
    const Material& currentMaterial = material != nullptr ? *material : inheritedMaterial;

    if (mesh != nullptr) {
        shader.setMat4("uModel", model);
        // Normal matrix: a inversa transposta da parte 3x3 da model (sem translação). Com escala
        // não uniforme (a lona da tela em Y), multiplicar a normal pela própria model a entorta e
        // ela deixa de ser perpendicular à superfície; a inversa transposta desfaz isso.
        // Calculada aqui, uma vez por peça, e não no vertex shader, uma vez por vértice.
        // Escala 0 deixaria a matriz singular (sem inversa): por isso a peça totalmente
        // recolhida usa visible = false e nem chega aqui.
        shader.setMat3("uNormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        currentMaterial.apply(shader);
        mesh->draw();
    }

    // Os filhos recebem a model e o material desta peça como "pai": a hierarquia se acumula.
    for (const auto& child : children_) {
        child->draw(shader, currentMaterial, model);
    }
}
