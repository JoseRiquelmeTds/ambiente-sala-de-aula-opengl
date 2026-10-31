#version 330 core

// Atributos do vértice. As locations batem com a struct Vertex (include/gfx/vertex.hpp).
layout(location = 0) in vec3 aPosition;  // metros, espaço do objeto
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// model: objeto -> mundo (T * R * S do objeto)
// view: mundo -> câmera
// projection: câmera -> clip (perspectiva)
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Normal matrix = transpose(inverse(mat3(uModel))), calculada na CPU por peça (SceneNode::draw).
uniform mat3 uNormalMatrix;

// Saídas interpoladas para o fragment shader.
out vec3 vWorldPos;  // posição no mundo (a iluminação vai precisar)
out vec3 vNormal;    // normal no mundo
out vec2 vUV;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);  // w = 1: ponto, recebe translação
    vWorldPos = worldPos.xyz;

    // Normal matrix: com escala não uniforme, multiplicar a normal pela model a entorta
    // e ela deixa de ser perpendicular à superfície. A inversa transposta corrige isso.
    // Não normaliza aqui: a interpolação entre vértices encurta o vetor de qualquer jeito,
    // então o fragment shader normaliza depois.
    vNormal = uNormalMatrix * aNormal;

    vUV = aUV;

    // Lido da direita para a esquerda: objeto -> mundo -> câmera -> clip.
    gl_Position = uProjection * uView * worldPos;
}
