#include "cglm/cglm.h" // Must be included before SDL since SDL won't redefine M_PI. Also brings in stdbool.h

#define SHADER(name, prefix, version, shaderSource) static const char* prefix##name = "#version " #version "\n" #shaderSource
#define VERTEX_SHADER(name, shaderSource) SHADER(name, VERTEX_SHADER_, 330, shaderSource)
#define FRAGMENT_SHADER(name, shaderSource) SHADER(name, FRAGMENT_SHADER_, 330, shaderSource)

typedef struct {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} Material;

VERTEX_SHADER(PYRAMID,
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * norm; // Do this on the CPU and pass in via uniform
    gl_Position = projection * view * model * vec4(pos, 1.0);
});

FRAGMENT_SHADER(PYRAMID,
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

void main()
{
    vec3 ambient = material.ambient * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * material.diffuse * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = material.specular * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
});

VERTEX_SHADER(LIGHT,
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
});

FRAGMENT_SHADER(LIGHT,
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
});
