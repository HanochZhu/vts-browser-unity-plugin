
layout(std140) uniform uboLabelFlat
{
    vec4 uniColor[2];
    vec4 uniOutline;
    vec4 uniPosition; // xyz
    vec4 uniCoordinates[1000];
    // even xyzw: clip space position
    // odd zw: uv (zw for compatibility with screen labels)
    // odd z: + plane index (multiplied by 2)
};

out vec2 varUv;
flat out int varPlane;

void main()
{
    // quad
    // 2--3
    // |  |
    // 0--1
    // triangles
    // 0-1-2
    // 1-3-2
    int tri = (gl_VertexID / 3) % 2;
    int vert = gl_VertexID % 3;
    int vertId = tri == 0 ? vert :
        vert == 0 ? 1 : vert == 1 ? 3 : 2;
    int coordId = 4 * (gl_VertexID / 6) + vertId;
    gl_Position = uniCoordinates[coordId * 2 + 0];
    vec4 coord = uniCoordinates[coordId * 2 + 1];
    varPlane = int(coord.z) / 2;
    varUv = coord.zw;
    varUv.x -= float(varPlane * 2);
    cullingCorrection();
}

