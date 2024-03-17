struct Buffers {
    StructuredBuffer<float> buffer0;
    StructuredBuffer<float> buffer1;
    RWStructuredBuffer<float> output;
};

ParameterBlock<Buffers> buffers;

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(
    uint3 threadId: SV_DispatchThreadID) {
    uint index = threadId.x;
    buffers.output[index] = buffers.buffer0[index] + buffers.buffer1[index];
}
