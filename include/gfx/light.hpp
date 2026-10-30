#ifndef AULA_OPENGL_GFX_LIGHT_HPP
#define AULA_OPENGL_GFX_LIGHT_HPP

#include <glm/glm.hpp>

#include <vector>

class Texture;

// =============================================================================
//  Luzes da cena (CG-71). Só dados: quem monta é scene/classroom, quem envia ao shader é o
//  renderer (renderer::draw). A técnica está em docs/ILUMINACAO.md.
// =============================================================================
//
//  Cores em RGB linear, de 0 a 1 por canal. A cor já inclui a intensidade: (0.5, 0.5, 0.5) é
//  uma luz branca com metade da força de (1, 1, 1).

// Máximo de luzes pontuais por frame. Tem que ser igual a kMaxPointLights em shaders/basic.frag:
// o array de uniforms do shader tem tamanho fixo.
constexpr int kMaxPointLights = 4;

// Luz que sai de um ponto para todos os lados (o tubo da luminária) e enfraquece com a distância.
struct PointLight {
    glm::vec3 position{0.0f};  // metros, no espaço do mundo
    glm::vec3 color{1.0f};

    // Atenuação: a luz que chega a uma distância d (metros) é multiplicada por
    //     1 / (constant + linear · d + quadratic · d²)
    // constant = 1 garante fator 1 (sem amplificar) em d = 0.
    float constant = 1.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;

    bool enabled = true;  // luz desligada não é enviada ao shader (interruptor, CG-73)
};

// Luz vinda de muito longe (sol entrando pela janela): todos os raios paralelos, sem atenuação.
struct DirectionalLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};  // para onde a luz VAI (do céu para a sala); não precisa ser unitária
    glm::vec3 color{0.0f};                   // preto = desligada
};

// Projetor (CG-77): luz que carrega uma imagem (textura projetiva). Cada fragmento é levado ao
// espaço de clip do projetor, como se o projetor fosse uma câmera; dentro do frustum, o texel do
// slide naquela posição é a cor da luz que chega ao fragmento. Técnica em docs/ILUMINACAO.md § 10.
struct ProjectorLight {
    bool enabled = false;
    glm::mat4 viewProjection{1.0f};  // mundo -> clip do projetor (P_proj · V_proj)
    glm::vec3 position{0.0f};        // centro da lente, mundo: de onde vem a luz (Lambert e distância)
    glm::vec3 color{1.0f};           // força do feixe na distância de referência, multiplica o slide
    float referenceDistance = 1.0f;  // metros; a força cai com (referenceDistance / d)², 1 nessa distância
    const Texture* image = nullptr;  // slide (não dono); nulo ou inválido = projetor não envia nada
};

// Tudo o que ilumina a cena num frame.
struct Lighting {
    // Luz ambiente: aproxima a luz rebatida nas paredes, que o modelo local não calcula.
    // Garante que nenhuma face fique totalmente preta, mesmo de costas para todas as luzes.
    glm::vec3 ambient{0.0f};

    DirectionalLight window;  // preenchimento fraco da janela

    // No máximo kMaxPointLights: o renderer ignora as excedentes (com aviso no console).
    std::vector<PointLight> pointLights;

    ProjectorLight projector;  // a imagem do projetor (CG-77)
};

#endif  // AULA_OPENGL_GFX_LIGHT_HPP
