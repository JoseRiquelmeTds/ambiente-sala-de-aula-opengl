#include "objects/furniture_style.hpp"

namespace objects::furniture {

Material tubeMaterial() {
    // Parte do preset de metal (difusa escura, reflexo forte), mas a tinta eletrostática deixa
    // o reflexo um pouco mais fraco e largo que o do aço nu: specular 0,55 e shininess 48 em vez
    // de 0,80 e 96. Grafite em vez do cinza médio, a cor mais comum em carteira escolar.
    Material material = materials::metal(glm::vec3(0.20f, 0.20f, 0.22f));
    material.specular = glm::vec3(0.55f);
    material.shininess = 48.0f;
    return material;
}

Material boardMaterial() {
    // Laminado melamínico: a madeira fica impressa sob uma camada de resina acetinada, então
    // reflete um pouco mais que a madeira crua do preset (0,06 e 12): reflexo fraco e largo.
    Material material = materials::wood(glm::vec3(1.0f));
    material.specular = glm::vec3(0.12f);
    material.shininess = 24.0f;
    return material;
}

}  // namespace objects::furniture
