#define BLOCK_SIZE 8 

struct ComputeShaderInput
{
	uint3 groupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
    uint3 groupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
    uint3 dispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
    uint  groupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}