#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

// Modo de cor (definido pelo renderer, src/gfx/renderer.cpp):
//   0 = iluminado (Blinn-Phong): textura como cor difusa + luzes da cena. Padrão.
//   1 = normal como cor: cada eixo vira um canal (x -> R, y -> G, z -> B)
//   2 = UV como cor: u -> R, v -> G
//   3 = textura sem luz (uTexture amostrada pela UV)
uniform int uDebugMode;

// Textura ligada na unidade indicada por setInt("uTexture", unidade) (veja include/gfx/texture.hpp).
uniform sampler2D uTexture;

// --- Iluminação (CG-71) ------------------------------------------------------
// Structs e limite iguais aos de include/gfx/light.hpp; quem envia é renderer::draw.
// Técnica explicada em docs/ILUMINACAO.md.
const int kMaxPointLights = 4;

struct PointLight {
    vec3 position;  // metros, mundo
    vec3 color;
    float constant;  // atenuação 1 / (constant + linear·d + quadratic·d²)
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;  // para onde a luz vai
    vec3 color;
};

uniform vec3 uViewPos;  // posição da câmera no mundo: o especular depende de onde se olha
uniform vec3 uAmbient;
uniform DirectionalLight uWindowLight;
uniform PointLight uPointLights[kMaxPointLights];
uniform int uPointLightCount;  // quantas posições do array valem neste frame (luzes ligadas)

// Material provisório, igual para a cena toda, até existir material por peça (CG-72).
const vec3 kSpecularColor = vec3(0.3);  // reflexo branco fraco
const float kShininess = 32.0;          // tamanho do brilho: maior = brilho menor e mais concentrado

// Difusa + especular de UMA luz, com L = direção do fragmento PARA a luz (unitária).
// N e V também unitários: com vetores unitários, dot() é o cosseno do ângulo entre eles.
vec3 blinnPhong(vec3 lightColor, vec3 L, vec3 N, vec3 V, vec3 albedo) {
    // Difusa (Lambert): máxima com a luz batendo de frente (N·L = 1), zero de lado ou por trás.
    float diffuse = max(dot(N, L), 0.0);

    // Especular (Blinn): H fica no meio entre a luz e o olho. Quanto mais H se alinha com a
    // normal, mais o fragmento reflete a luz direto para a câmera.
    vec3 H = normalize(L + V);
    float specular = pow(max(dot(N, H), 0.0), kShininess);
    // Face de costas para a luz não tem reflexo (evita brilho "vazando" para o lado escuro).
    if (diffuse <= 0.0) {
        specular = 0.0;
    }

    return lightColor * (diffuse * albedo + specular * kSpecularColor);
}

vec3 shade(vec3 albedo) {
    // A interpolação entre os vértices encurta a normal: normaliza de novo por fragmento.
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);

    // Ambiente: constante, não depende de direção. Impede que faces de costas fiquem pretas.
    vec3 color = uAmbient * albedo;

    // Janela: direcional, sem atenuação. L é o oposto da direção em que a luz viaja.
    color += blinnPhong(uWindowLight.color, normalize(-uWindowLight.direction), N, V, albedo);

    // Luzes pontuais: L muda por fragmento e a força cai com a distância.
    for (int i = 0; i < uPointLightCount; ++i) {
        vec3 toLight = uPointLights[i].position - vWorldPos;
        float d = length(toLight);
        float attenuation = 1.0 / (uPointLights[i].constant + uPointLights[i].linear * d +
                                   uPointLights[i].quadratic * d * d);
        color += attenuation * blinnPhong(uPointLights[i].color, toLight / d, N, V, albedo);
    }
    return color;
}

void main() {
    if (uDebugMode == 1) {
        // A normal vai de -1 a 1 e a cor de 0 a 1: n * 0.5 + 0.5 faz essa conversão.
        // Face para +X fica vermelha, +Y verde, +Z azul.
        FragColor = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
    } else if (uDebugMode == 2) {
        FragColor = vec4(vUV, 0.0, 1.0);
    } else if (uDebugMode == 3) {
        // texture() busca a cor na posição (u, v) da imagem, já com wrap e filtro aplicados:
        // UV fora de 0..1 repete (GL_REPEAT) e entre dois texels a cor é misturada (GL_LINEAR).
        FragColor = texture(uTexture, vUV);
    } else {
        // Soma de várias luzes pode passar de 1; o OpenGL corta em 1 ao gravar a cor.
        FragColor = vec4(shade(texture(uTexture, vUV).rgb), 1.0);
    }
}
