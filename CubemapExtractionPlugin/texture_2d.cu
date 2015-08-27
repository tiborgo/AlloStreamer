/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <helper_cuda.h>

#define PI 3.1415926536f

// This must be global. Otherwise, cudaBindTextureToArray will fail
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef; 
//texture<float4, 2, cudaReadModeElementType> texRef;

__device__ __inline__ float3 make_float3(uchar4& v)
{
	return make_float3(v.x, v.y, v.z);
}

/*
 * Paint a 2D texture with a moving red/green hatch pattern on a
 * strobing blue background.  Note that this kernel reads to and
 * writes from the texture, hence why this texture was not mapped
 * as WriteDiscard.
 */
__global__ void cuda_kernel_texture_2d(uint8_t* buffer, int width, int height, int t)
{
    int x = (blockIdx.x*blockDim.x + threadIdx.x) * 2;
    int y = (blockIdx.y*blockDim.y + threadIdx.y) * 2;

    // in the case where, due to quantization into grids, we have
    // more threads than pixels, skip the threads which don't
    // correspond to valid pixels
    if ((x+1) >= width || (y+1) >= height) return;

	float3 yuv = make_float3(0.0f, 0.0f, 0.0f);

	for (int x_ = x; x_ <= x + 1; x_++)
	{
		for (int y_ = y; y_ <= y + 1; y_++)
		{
			float3 rgb = make_float3(tex2D(texRef, x_, y_));

			yuv.x  =  (0.257f * rgb.z) + (0.504f * rgb.y) + (0.098f * rgb.x) +  16.f;
			yuv.y +=  (0.439f * rgb.z) - (0.368f * rgb.y) - (0.071f * rgb.x) + 128.f;
			yuv.z += -(0.148f * rgb.z) - (0.291f * rgb.y) + (0.439f * rgb.x) + 128.f;
		
			uint8_t* yPtr = (buffer + (height - y_ - 1) * width + x_);
			*yPtr = yuv.x;
		}
	}

	uint8_t* uPtr = (buffer + width * height                        + (height/2 - y/2 - 1) * width/2 + x/2);
	uint8_t* vPtr = (buffer + width * height + (width * height) / 4 + (height/2 - y/2 - 1) * width/2 + x/2);

	*uPtr = yuv.y / 4.f;
	*vPtr = yuv.z / 4.f;
}

extern "C"
void* cuda_texture_2d(cudaGraphicsResource* cudaResource, int width, int height, float t)
{
    cudaError_t error = cudaSuccess;

	//texture<uint8_t, cudaTextureType2D, cudaReadModeElementType> texRef;
	static int x = 0;
	x++;

	cudaArray* cuArray;

	void* cudaLinearMemory;
	//size_t pitch;

	cudaMalloc(&cudaLinearMemory, width * height * 4);
	getLastCudaError("cudaMallocPitch (g_texture_2d) failed");
	cudaMemset(cudaLinearMemory, 0, width * height * 3);

	error = cudaGraphicsSubResourceGetMappedArray(&cuArray, cudaResource, 0, 0);
	getLastCudaError("cudaGraphicsSubResourceGetMappedArray (cuda_texture_2d) failed");

	
	error = cudaBindTextureToArray(texRef, cuArray);
	getLastCudaError("cudaGraphicsSubResourceGetMappedArray (cuda_texture_2d) failed");

    dim3 Db = dim3(16, 16);   // block dimensions are fixed to be 256 threads
    dim3 Dg = dim3(((width/2)+Db.x-1)/Db.x, ((height/2)+Db.y-1)/Db.y);

    cuda_kernel_texture_2d<<<Dg,Db>>>((uint8_t*)cudaLinearMemory, width, height, x);

    error = cudaGetLastError();

    if (error != cudaSuccess)
    {
        printf("cuda_kernel_texture_2d() failed to launch error = %d\n", error);
    }

	//error = cudaMemcpyFromArray(cudaLinearMemory, cuArray, 0, 0, width * height * 4, cudaMemcpyDeviceToDevice);

	/*error = cudaMemcpy2DToArray(
		cuArray, // dst array
		0, 0,    // offset
		cudaLinearMemory, pitch,       // src
		width * 4 * sizeof(uint8_t), height, // extent
		cudaMemcpyDeviceToDevice);*/
	//getLastCudaError("cudaMemcpy2DToArray failed");

	return cudaLinearMemory;
}
