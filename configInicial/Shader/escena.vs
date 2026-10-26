#version 330 core
// Shader de vértices de la escena: igual que lighting.vs de la práctica de iluminación.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 repeticion;     // cuántas veces se repite la textura (1,1 = una vez)
uniform vec2 desplazamiento; // corre la textura (sirve para elegir un dígito del marcador)

void main()
{
    FragPos = vec3(model * vec4(position, 1.0f));
    // matriz normal: mantiene la normal perpendicular aunque el objeto esté escalado
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = texCoords * repeticion + desplazamiento;
    gl_Position = projection * view * vec4(FragPos, 1.0f);
}
