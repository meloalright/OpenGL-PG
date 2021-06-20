#version 330 core
out vec4 FragColor;

in vec3 rainbowColor;

void main()
{
    FragColor = vec4(rainbowColor, 0.6);
}