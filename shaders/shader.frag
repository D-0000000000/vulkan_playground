#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main()
{
    // vec2 pos=gl_PointCoord-vec2(0.5f,0.5f);
    // float dist =length(pos);
    // if(dist>0.5)
    // {
    //     discard;
    // }
    outColor = vec4(fragColor, 1.0);
    // float dist = length(fragPos);
    // dist/=20.0;
    // outColor=vec4(vec3(dist),1.0);
}
