struct InputVertex {
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

public struct VertexStageOutput {
    float2 uv : TEXCOORD;
    float4 color;
    float4 position : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertex_main(InputVertex input_vertex) {
    VertexStageOutput output;

    output.uv = input_vertex.uv;
    output.color = float4(input_vertex.color, 1.0);
    output.position = float4(input_vertex.position, 1.0);

    return output;
}

struct FragmentStageOutput {
    float4 color : SV_Target;
};

[shader("fragment")]
FragmentStageOutput fragment_main(float2 uv : TEXCOORD) {
    FragmentStageOutput output;

    output.color = float4(uv, 0.0f, 1.0f);

    return output;
}
