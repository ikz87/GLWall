#version 330 core
out vec4 FragColor;

uniform vec3 iResolution; // Resolution of the shader window
uniform float iTime; // Time in seconds since start of rendering
uniform float iTimeDelta; // Time in seconds since last frame
uniform float iBatteryLevel; // Battery level of your pc, from 0 (empty) to 1 (full)
uniform float iLocalTime; // Local time of your pc, from 0 (12 am) to 1 (11:59:59 pm)
uniform vec4 iMouse; // Position of your mouse (z and w components unused as of now)
uniform sampler2D myTexture; // Texture extracted from a jpeg file

// Paste shadertoy code here, as long as the shader doesn't
// use any channels or buffers, it should work

void main()
{
    mainImage(FragColor, gl_FragCoord.xy);
}
