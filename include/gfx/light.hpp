#ifndef AULA_OPENGL_GFX_LIGHT_HPP
#define AULA_OPENGL_GFX_LIGHT_HPP

#include <glm/glm.hpp>

#include <vector>

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

// Luz vinda de muito longe (a claridade do dia): todos os raios paralelos, sem atenuação.
struct DirectionalLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};  // para onde a luz VAI (do céu para a sala); não precisa ser unitária
    glm::vec3 color{0.0f};                   // preto = desligada
};

// Tudo o que ilumina a cena num frame.
struct Lighting {
    // Luz ambiente: aproxima a luz rebatida nas paredes, que o modelo local não calcula.
    // Garante que nenhuma face fique totalmente preta, mesmo de costas para todas as luzes.
    glm::vec3 ambient{0.0f};

    DirectionalLight fill;  // preenchimento fraco (claridade do dia), não sai de nenhum objeto da cena

    // No máximo kMaxPointLights: o renderer ignora as excedentes (com aviso no console).
    std::vector<PointLight> pointLights;
};

#endif  // AULA_OPENGL_GFX_LIGHT_HPP
