#version 330 core
// Shader de fragmentos de la escena: modelo de iluminación de Phong
// con una luz direccional (luz general del recinto) y varios reflectores (luces de tipo foco).

#define NUM_REFLECTORES 6

struct Material
{
    sampler2D diffuse;
    float specular;      // qué tanto brilla (hielo mucho, madera poco)
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
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
uniform DirLight dirLight;
uniform SpotLight reflectores[NUM_REFLECTORES];
uniform int reflectoresEncendidos;
uniform vec3 tinte;            // color que multiplica a la textura
uniform float transparencia;   // 1 = opaco, menor = cristal
uniform int emisivo;           // 1 = brilla con los reflectores (focos), 2 = siempre brilla (pantallas)

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 base)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 ambient = light.ambient * base;
    vec3 diffuse = light.diffuse * diff * base;
    vec3 specular = light.specular * spec * material.specular;
    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 base)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // atenuación según la distancia
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    // borde suave del cono de luz
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 ambient = light.ambient * base;
    vec3 diffuse = light.diffuse * diff * base;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + (diffuse + specular) * intensity) * attenuation;
}

void main()
{
    vec4 texColor = texture(material.diffuse, TexCoords);
    if (texColor.a < 0.1)
        discard;                       // huecos de la red y esquinas de la pista
    vec3 base = texColor.rgb * tinte;

    if (emisivo == 1)
    {
        color = vec4(reflectoresEncendidos == 1 ? base : base * 0.15, 1.0);
        return;
    }
    if (emisivo == 2)
    {
        color = vec4(base, texColor.a);
        return;
    }

    vec3 norm = normalize(Normal);
    if (!gl_FrontFacing) norm = -norm; // cristal y red se ven por los dos lados
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, base);
    if (reflectoresEncendidos == 1)
        for (int i = 0; i < NUM_REFLECTORES; i++)
            result += CalcSpotLight(reflectores[i], norm, FragPos, viewDir, base);

    color = vec4(result, texColor.a * transparencia);
}
