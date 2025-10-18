#version 330 core

in vec2 vUV;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.f, 0.2f, 1.0f);
    FragColor = vec4(vUV, 0.2f, 1.0f);
}
