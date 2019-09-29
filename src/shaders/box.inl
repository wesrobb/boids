#include "../shaders.h"

VERTEX_SHADER(BOX,
layout(location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
});

FRAGMENT_SHADER(BOX,
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.2, 0.2, 0.2, 0.5);
});
