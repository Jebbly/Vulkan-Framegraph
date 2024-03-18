struct Matrices {
    float4x4 mvp;
};

ConstantBuffer<Matrices> model_matrices;

struct InputVertex {
    float3	position : POSITION;
    float3	color : COLOR;
};

struct PerVertexData {
    float3 color;
};

struct VertexStageOutput {
    PerVertexData vertex_info : PerVertexData;
    float4 sv_position : SV_Position;
};

struct FragmentStageOutput {
    float4 color : SV_Target;
};

[shader("vertex")]
VertexStageOutput vertex_main(InputVertex input_vertex) {
    VertexStageOutput output;

    output.vertex_info.color = input_vertex.color;
    output.sv_position = mul(model_matrices.mvp, float4(input_vertex.position, 1.0));

    return output;
}

[shader("fragment")]
FragmentStageOutput fragment_main(PerVertexData vertex_data : PerVertexData) {
    FragmentStageOutput output;

    output.color = float4(vertex_data.color, 1.0f);

    return output;
}
