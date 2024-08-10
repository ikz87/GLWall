#version 330 core
out vec4 FragColor;

uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform float iBatteryLevel;
uniform float iLocalTime;
uniform vec4 iMouse;
uniform sampler2D myTexture;

#define pi 3.14159265

#define SPEED 0.1
#define BG_COL vec4(0.153,0.165,0.204,1.0)
#define TRIANG_1_COL vec4(0.698,0.129,0.267,1.0)
#define TRIANG_2_COL vec4(1.,0.188,0.38,1.0)
#define DISC_RADIUS 0.1
#define MOTION_BLUR 5
#define BLUR_STEP 0.007

vec4 pastel_gradient(vec2 fragCoord) {
    vec2 uv = (fragCoord * 2.0 - iResolution.xy) / min(iResolution.x, iResolution.y) * 1.5;
    float t = iTime * 0.5;

    float a = 0.0;
    float b = 0.0;
    float c = 1.0;
    for(float i = 0.0; i < 4.0; ++i) {
        a = sin(-b*b - uv.x);
        b = cos(-a*a + c*c - uv.y - cos(t-a*b*c+uv.x));
        c = cos(a - b*b + c - t);
    }
    vec3 col = vec3(a, b, 0);
    col = sin(col * 1.6 + vec3(0.4, 0.6, 1.5));
    col *= sqrt(abs(col));
    col = cos(sqrt(sqrt(cos(col * 1.56))) * 1.4 - vec3(abs(col.r)*0.5, 0.5, 0.5));
    col = col * 2.0 / (1.0 + col);
    col.rg -= c*c * 0.1;

    return vec4(col, 1);
}

float angle_mrua(float fi, float ff, float po, float time)
{
    float angle = po + 2*pi*fi*(time) + 0.5*(2*pi*(ff-fi)/0.25)*pow(time,2);
    return angle;
}

float angle_mru(float f, float po, float time)
{
    float angle = po + 2*pi*f*(time);
    return angle;
}

void main()
{
    vec2 uv = gl_FragCoord.xy/iResolution.xy;
    float ratio = iResolution.y/iResolution.x;
    float time_cyclic = mod(iTime*SPEED, 1.0);
    float time_sin = sin(time_cyclic*2*pi);
    float time_cos = cos(time_cyclic*2*pi);
    //FragColor = BG_COL;
    FragColor = pastel_gradient(vec2(gl_FragCoord));
    FragColor = mix(vec4(FragColor.xyz/(FragColor.x+FragColor.y+FragColor.z), FragColor.z), BG_COL, 0.5);
    
    // Corner triangles 1
    // Top left
    if (uv.y + ((time_cos+1)/2)*0.1 > uv.x/ratio + 0.55 )
    {
        FragColor = TRIANG_1_COL;
    }
    else
    {
        float shadow = smoothstep(uv.y + ((time_cos+1)/2)*0.1 + 0.1, uv.y + ((time_cos+1)/2)*0.1, uv.x/ratio + 0.55);
        FragColor *= 1-shadow/3;
    }

    // Bottom right
    if (1-uv.y + ((time_sin+1)/2)*0.1 > (1-uv.x)/ratio + 0.55 )
    {
        FragColor = TRIANG_1_COL;
    }
    else
    {
        float shadow = smoothstep(1-uv.y + ((time_sin+1)/2)*0.1 + 0.1, 1-uv.y + ((time_sin+1)/2)*0.1, (1-uv.x)/ratio + 0.55);
        FragColor *= 1-shadow/3;
    }

    // Corner triangles 2
    // Top left
    if (uv.y + ((time_cos+time_sin+1)/2)*0.1 > uv.x/ratio + 0.7 )
    {
        FragColor = TRIANG_2_COL;
    }
    else
    {
        float shadow = smoothstep(uv.y + ((time_cos+time_sin+1)/2)*0.1 + 0.1, uv.y + ((time_cos+time_sin+1)/2)*0.1, uv.x/ratio + 0.7);
        FragColor *= 1-shadow/3;
    }

    // Bottom right
    if (1-uv.y + ((time_sin-time_cos+1)/2)*0.1 > (1-uv.x)/ratio + 0.7 )
    {
        FragColor = TRIANG_2_COL;
    }
    else
    {
        float shadow = smoothstep(1-uv.y + ((time_sin-time_cos+1)/2)*0.1 + 0.1, 1-uv.y + ((time_sin-time_cos+1)/2)*0.1, (1-uv.x)/ratio + 0.7);
        FragColor *= 1-shadow/3;
    }


    // Melee disc
    vec2 disc_center = vec2(0.5);
    disc_center.y += time_cos/30;

    vec2 adjustedUV = vec2(uv.x, uv.y * ratio);
    vec2 adjustedCenter = vec2(disc_center.x, disc_center.y * ratio);

    if (distance(adjustedUV, adjustedCenter) < DISC_RADIUS)
    {
        // Motion blur
        vec2 curr_uv = adjustedUV - adjustedCenter;
        vec2 tmp;
        vec4 total_color = vec4(0.0);
        for (int i = 0; i < MOTION_BLUR; ++i)
        {
            vec4 step_color;
            float mtime_cyclic = mod(iTime*SPEED + float(i) / float(MOTION_BLUR) * BLUR_STEP * SPEED, 1.0);

            // Bruh i had to ask chatgpt for this
            // edit: nvm I just solved it on my own after all
            float mtime_cos, mtime_sin = 0;
            float fi = 1;
            float ff = 50;
            if (mtime_cyclic < 0.25)
            {
                float angle = angle_mru(fi, 0.0, mtime_cyclic);
                mtime_cos = cos(angle);
                mtime_sin = sin(angle);
            }
            else if (mtime_cyclic < 0.5)
            {
                float angle = angle_mrua(fi, ff, 
                        angle_mru(fi, 0.0, 0.25),
                        mtime_cyclic-0.25);
                mtime_cos = cos(angle);
                mtime_sin = sin(angle);
            }
            else if (mtime_cyclic < 0.75)
            {
                float angle = angle_mru(ff, 0.75, mtime_cyclic);
                mtime_cos = cos(angle);
                mtime_sin = sin(angle);
            }
            else
            {
                float angle = angle_mrua(ff, fi, 
                        angle_mru(ff, 0.75, 0.75),
                        mtime_cyclic-0.75);
                mtime_cos = cos(angle);
                mtime_sin = sin(angle);
            }

            tmp.x = curr_uv.x * mtime_cos + curr_uv.y * mtime_sin;
            tmp.y = -curr_uv.x * mtime_sin + curr_uv.y * mtime_cos;
            vec2 new_uv = tmp + adjustedCenter;

            vec2 texCoord = vec2((new_uv.x - disc_center.x + 0.5) / DISC_RADIUS / 2.0, 
                    (-new_uv.y + adjustedCenter.y + 0.5) / DISC_RADIUS / 2.0);
            step_color = texture(myTexture, texCoord);
            if (distance(step_color, vec4(0.0, 1.0, 0.0, 1.0)) < 0.7) { step_color = pastel_gradient(gl_FragCoord.xy/DISC_RADIUS); }
            if (distance(step_color, vec4(0.0, 0.0, 0.0, 1.0)) < 0.15) { step_color = BG_COL; }
            if (distance(adjustedUV, adjustedCenter) < DISC_RADIUS/5)
            {
                float shadow = smoothstep(DISC_RADIUS/5 - 0.02, DISC_RADIUS/5, distance(adjustedUV, adjustedCenter));
                step_color *= 1-shadow/3;
            }
            total_color += step_color;
        }
        total_color /= float(MOTION_BLUR);

        FragColor = total_color;

    }
    else
    {
        float shadow = smoothstep(DISC_RADIUS+0.01, DISC_RADIUS-0.01, distance(adjustedUV, adjustedCenter+vec2(0.01, -0.01*ratio)));
        FragColor *= 1-shadow/3;
    }
}
