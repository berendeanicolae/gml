#ifndef  __GPU_KERNELS_
#define  __GPU_KERNELS_

#define GPU_NR_KERNELS  2
char* GpuProgramSource [GPU_NR_KERNELS] = {
    "__kernel void VectorAdd(__global float* input, __global float* output, const unsigned int count) {"
        "int i = get_global_id(0);"
        "if (i<count)"
            "output[i] = input[i] * input[i];"
    "}",
    "__kernel void VectorAdd2(__global float* input, __global float* output, const unsigned int count) {"
    "int i = get_global_id(0);"
    "if (i<count)"
    "output[i] = input[i] * input[i];"
    "}"
};

#endif