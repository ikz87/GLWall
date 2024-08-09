#version 330 core
out vec4 FragColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform float iBatteryLevel;
uniform float iLocalTime;
uniform vec4 iMouse;
uniform sampler2D myTexture;


// Paste shadertoy code here, as long as the shader doesn't
// use any channels or buffers, it should work

void main()
{
    mainImage(FragColor, gl_FragCoord.xy);
}
