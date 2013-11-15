#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
__constant char hw[] = "Hello World!\n";
__kernel void hello(__global char* out)
{
    size_t tid = get_global_id(0);
    out[tid] = hw[tid];
}
__kernel void square(__global short* in, __global short* out)
{
    size_t tid = get_global_id(0);
    out[tid] = in[tid] * in[tid];
}
