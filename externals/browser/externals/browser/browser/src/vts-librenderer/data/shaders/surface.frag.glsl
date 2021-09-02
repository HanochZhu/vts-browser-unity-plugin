
uniform sampler2D texColor;
uniform sampler2D texMask;
uniform lowp sampler2DArray texBlueNoise;

layout(std140) uniform uboSurface
{
    mat4 uniP;
    mat4 uniMv;
    vec4 uniUvTrans; // scale-x, scale-y, offset-x, offset-y
    vec4 uniUvClip;
    vec4 uniColor;
    ivec4 uniFlags; // mask, monochromatic, flat shading, uv source, lodBlendingWithDithering, ..., blendingCoverage, frameIndex
};

in vec2 varUvTex;
#ifdef VTS_NO_CLIP
in vec2 varUvExternal;
#endif
in vec3 varViewPosition;
#ifdef VTS_ATM_PER_VERTEX
in float varAtmDensity;
#endif

layout(location = 0) out vec4 outColor;

bool getFlag(int i)
{
    return (uniFlags[i / 8] & (1 << (i % 8))) != 0;
}

void main()
{
    // derivatives computed before leaving uniform control flow
    vec2 uvDx = dFdx(varUvTex);
    vec2 uvDy = dFdy(varUvTex);
    vec3 viewDx = dFdx(varViewPosition);
    vec3 viewDy = dFdy(varViewPosition);

#ifdef VTS_NO_CLIP
    // no hardware clipping, do it here instead
    vec4 clipDistance;
    clipDistance[0] = (varUvExternal[0] - uniUvClip[0]) * +1.0;
    clipDistance[1] = (varUvExternal[1] - uniUvClip[1]) * +1.0;
    clipDistance[2] = (varUvExternal[0] - uniUvClip[2]) * -1.0;
    clipDistance[3] = (varUvExternal[1] - uniUvClip[3]) * -1.0;
    if (any(lessThan(clipDistance, vec4(0.0))))
        discard;
#endif

    // mask
    if (getFlag(0))
    {
        // assuming mipmaps are not needed:
        //   is textureLod more efficient than regular texture access?
        if (textureLod(texMask, varUvTex, 0.0).r < 0.5)
            discard;
    }

    // dithered lod blending
    if (getFlag(4))
    {
        float smpl = texelFetch(texBlueNoise, ivec3(
            ivec2(gl_FragCoord.xy) % 64, uniFlags.w % 16), 0).x;
        if (float(uniFlags[2]) < smpl * 1000.0f)
            discard;
    }

    if (getFlag(2))
    {
        // flat shading
        vec3 n = normalize(cross(viewDx, viewDy));
        outColor = vec4(vec3(max(n.z * 0.8, 0.0) + 0.125), 1.0);
    }
    else
    {
        // base color
        outColor = textureGrad(texColor, varUvTex, uvDx, uvDy);

        // monochromatic texture
        if (getFlag(1))
            outColor = outColor.rrra;
    }

    // atmosphere
    outColor *= uniColor;
#ifdef VTS_ATM_PER_VERTEX
    outColor = atmColor(varAtmDensity, outColor);
#else
    float atmDensity = atmDensity(varViewPosition);
    outColor = atmColor(atmDensity, outColor);
#endif
}

