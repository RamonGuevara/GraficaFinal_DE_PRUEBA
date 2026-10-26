#version 330 core
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;    // reflector 1 (mitad izquierda de la pista)
uniform Light light2;   // reflector 2 (mitad derecha de la pista)
uniform sampler2D texture_diffuse;

void main()
{
    vec4 texColor = texture(texture_diffuse, TexCoords);
    if (texColor.a < 0.1)
        discard;   // huecos de la red

    // Luz 1
    // Ambiental
    vec3 ambient = light.ambient * material.diffuse;

    // Difusa
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    // Especular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    // Luz 2: mismas tres componentes con light2
    vec3 ambient2 = light2.ambient * material.diffuse;
    lightDir = normalize(light2.position - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse2 = light2.diffuse * diff * material.diffuse;
    reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular2 = light2.specular * (spec * material.specular);

    vec3 result2 = ambient2 + diffuse2 + specular2;
    vec3 result  = ambient + diffuse + specular;
    result = result + result2;
    color = vec4(result, 1.0f) * texColor;
}
