
layout(std140) uniform uboTriangleData
{
    vec4 uniColor;
    vec4 uniVisibilities;
    ivec4 uniFlags; // shading
};

layout(location = 0) out vec4 outColor;

in float varOpacity;
in vec3 varViewPosition;

void main()
{
    outColor = uniColor;
    outColor.a *= varOpacity;

    // flat shading
    if (uniFlags[0] == 2)
    {
        vec3 viewDx = dFdx(varViewPosition);
        vec3 viewDy = dFdy(varViewPosition);
        vec3 n = normalize(cross(viewDx, viewDy));
        outColor.xyz *= max(n.z * 0.8, 0.0) + 0.125;
    }
}

