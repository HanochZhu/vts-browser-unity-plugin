
uniform sampler2D texDepth;

out vec4 outDepth;

vec4 packFloatToVec4(float value)
{
    uint rgba = floatBitsToUint(value);
    float r = float((rgba & 0xff000000u) >> 24);
    float g = float((rgba & 0x00ff0000u) >> 16);
    float b = float((rgba & 0x0000ff00u) >> 8);
    float a = float((rgba & 0x000000ffu) >> 0);
    return vec4(a, b, g, r) / 255.0;
}

void main()
{
    outDepth = packFloatToVec4(texelFetch(texDepth,
        ivec2(gl_FragCoord) * 3, 0).r);
}

