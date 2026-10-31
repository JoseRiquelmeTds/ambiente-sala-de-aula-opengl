#ifndef AULA_OPENGL_GFX_MATERIAL_HPP
#define AULA_OPENGL_GFX_MATERIAL_HPP

#include <glm/glm.hpp>

class Shader;
class Texture;

// =============================================================================
//  Material: como uma superfície responde à luz (CG-72). Técnica em docs/ILUMINACAO.md § 7.
// =============================================================================
//
//  Cada campo entra num termo do Blinn-Phong (shaders/basic.frag):
//
//      cor = emissive + ambiente·D + Σ luzes atenuação·corDaLuz·( max(N·L,0)·D + max(N·H,0)^shininess·specular )
//      D   = diffuse × textura(uv · uvScale)      (só diffuse, sem textura)
//
//  O material é ligado a uma peça por ponteiro (SceneNode::material) e herdado pelos filhos:
//  defina na raiz do objeto e sobrescreva só nas peças diferentes. Peça sem material em
//  nenhum ancestral usa materials::neutral().
//
//  Uso (num objeto; material e textura são membros, criados antes da árvore):
//      Texture woodTexture_{"assets/textures/madeira.png"};
//      Material wood_ = materials::wood();     // no construtor: wood_.diffuseMap = &woodTexture_;
//      SceneNode& root = parent.addChild(&top_, placement);
//      root.material = &wood_;                 // tampo e tudo abaixo dele
//      leg.material = &metal_;                 // só a perna (e os filhos dela) muda
struct Material {
    // Cor difusa (albedo): a fração de cada canal que a superfície espalha. Com textura,
    // tinge a textura (branco = textura pura).
    glm::vec3 diffuse{1.0f};

    // Textura da cor difusa. Não dona: quem cria a Texture a mantém viva enquanto o material
    // for usado. nullptr = só a cor de diffuse.
    const Texture* diffuseMap = nullptr;

    // Repetições da textura por unidade de UV (tiling). Ex.: {2, 1} repete duas vezes em u.
    // Para uma peça esticada não esticar a textura, aumente na direção que cresceu.
    glm::vec2 uvScale{1.0f};

    // Cor do reflexo especular. Plástico e tinta: cinza/branco (o reflexo tem a cor da luz).
    // Metal: alto e da cor do metal. Fosco: perto de zero.
    glm::vec3 specular{0.0f};

    // Expoente do especular: 4 a 16 = reflexo largo e fraco; 64 a 128 = pequeno e intenso.
    // Mínimo 1 (pow(0, 0) não é definido no GLSL).
    float shininess = 8.0f;

    // Luz própria, somada sem depender de luz nenhuma (tubo da lâmpada acesa, CG-73).
    glm::vec3 emissive{0.0f};

    // Envia o material para os uniforms uMaterial.* do shader em uso e liga a textura na
    // unidade kDiffuseTextureUnit.
    void apply(const Shader& shader) const;
};

// Unidade de textura da cor difusa (o sampler uMaterial.diffuseMap lê dela). Uma textura por
// material, então uma unidade basta; outra textura (normal map, por exemplo) usaria a 1.
constexpr int kDiffuseTextureUnit = 0;

// Materiais prontos, com valores plausíveis para Blinn-Phong. Retornam por valor: guarde o
// resultado num membro e ajuste o que quiser (cor, textura, uvScale) antes de apontar uma peça
// para ele.
namespace materials {

// Material padrão das peças sem material: cinza claro fosco, sem textura.
Material neutral();

// Aço pintado/escovado: difusa escura, reflexo forte e concentrado.
Material metal(const glm::vec3& color = glm::vec3(0.30f, 0.30f, 0.32f));

// Madeira/MDF: quase só difusa, reflexo mínimo e largo.
Material wood(const glm::vec3& color = glm::vec3(0.55f, 0.37f, 0.22f));

// Plástico: reflexo branco médio, de tamanho médio.
Material plastic(const glm::vec3& color = glm::vec3(0.70f, 0.12f, 0.10f));

// Fosco (quadro, parede, lona): só difusa, sem reflexo perceptível.
Material matte(const glm::vec3& color = glm::vec3(0.80f, 0.80f, 0.78f));

}  // namespace materials

#endif  // AULA_OPENGL_GFX_MATERIAL_HPP
