StructuredBuffer<float> buffer0; 
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 threadId: SV_DispatchThreadID) {
    uint index = threadId.x;
    output[index] = buffer0[index] + buffer1[index];
}
