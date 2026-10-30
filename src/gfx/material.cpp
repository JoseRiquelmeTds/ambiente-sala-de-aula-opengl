#include "gfx/material.hpp"

#include "gfx/shader.hpp"
#include "gfx/texture.hpp"

void Material::apply(const Shader& shader) const {
    shader.setVec3("uMaterial.diffuse", diffuse);
    shader.setVec2("uMaterial.uvScale", uvScale);
    shader.setVec3("uMaterial.specular", specular);
    shader.setFloat("uMaterial.shininess", shininess);
    shader.setVec3("uMaterial.emissive", emissive);

    // O shader só amostra a textura se hasDiffuseMap for verdadeiro: sem textura ligada, a
    // unidade guardaria a textura da peça anterior. Textura que falhou ao carregar conta como
    // nenhuma (fica só a cor).
    const bool hasDiffuseMap = diffuseMap != nullptr && diffuseMap->isValid();
    shader.setInt("uMaterial.hasDiffuseMap", hasDiffuseMap ? 1 : 0);
    if (hasDiffuseMap) {
        diffuseMap->bind(kDiffuseTextureUnit);
    }
    // O sampler2D recebe o NÚMERO da unidade, não o id da textura.
    shader.setInt("uMaterial.diffuseMap", kDiffuseTextureUnit);
}

namespace materials {

// Valores em RGB de 0 a 1, escolhidos pelo comportamento real de cada superfície:
//  - não metais (madeira, plástico, tinta) têm reflexo branco, da cor da luz, e a cor do
//    objeto vem da difusa;
//  - metal devolve quase toda a luz como reflexo: especular alto e difusa escura;
//  - quanto mais lisa a superfície, maior o shininess (reflexo menor e mais nítido).

Material neutral() {
    Material material;
    material.diffuse = glm::vec3(0.72f);
    material.specular = glm::vec3(0.04f);
    material.shininess = 8.0f;
    return material;
}

Material metal(const glm::vec3& color) {
    Material material;
    material.diffuse = color;
    material.specular = glm::vec3(0.80f, 0.80f, 0.82f);
    material.shininess = 96.0f;
    return material;
}

Material wood(const glm::vec3& color) {
    Material material;
    material.diffuse = color;
    material.specular = glm::vec3(0.06f);
    material.shininess = 12.0f;
    return material;
}

Material plastic(const glm::vec3& color) {
    Material material;
    material.diffuse = color;
    material.specular = glm::vec3(0.45f);
    material.shininess = 32.0f;
    return material;
}

Material matte(const glm::vec3& color) {
    Material material;
    material.diffuse = color;
    material.specular = glm::vec3(0.01f);
    material.shininess = 4.0f;
    return material;
}

}  // namespace materials
