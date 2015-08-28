#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <helper_cuda.h>

#include "AlloShared/Cubemap.hpp"

// This must be global. Otherwise, cudaBindTextureToArray will fail
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef0;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef1;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef2;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef3;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef4;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef5;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef6;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef7;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef8;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef9;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef10;
texture<uchar4, cudaTextureType2D, cudaReadModeElementType> texRef11;

__device__ __inline__ float3 make_float3(uchar4& v)
{
	return make_float3(v.x, v.y, v.z);
}

__global__ void cuda_kernel_texture_2d(uint8_t* buffer, int width, int height, int face)
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
			float3 rgb;
			
			switch (face)
			{
			case 0:
				rgb = make_float3(tex2D(texRef0, x_, y_));
				break;
			case 1:
				rgb = make_float3(tex2D(texRef1, x_, y_));
				break;
			case 2:
				rgb = make_float3(tex2D(texRef2, x_, y_));
				break;
			case 3:
				rgb = make_float3(tex2D(texRef3, x_, y_));
				break;
			case 4:
				rgb = make_float3(tex2D(texRef4, x_, y_));
				break;
			case 5:
				rgb = make_float3(tex2D(texRef5, x_, y_));
				break;
			case 6:
				rgb = make_float3(tex2D(texRef6, x_, y_));
				break;
			case 7:
				rgb = make_float3(tex2D(texRef7, x_, y_));
				break;
			case 8:
				rgb = make_float3(tex2D(texRef8, x_, y_));
				break;
			case 9:
				rgb = make_float3(tex2D(texRef9, x_, y_));
				break;
			case 10:
				rgb = make_float3(tex2D(texRef10, x_, y_));
				break;
			case 11:
				rgb = make_float3(tex2D(texRef11, x_, y_));
				break;
			}
			

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

extern "C" void* cuda_texture_2d(cudaGraphicsResource* cudaResource, int width, int height, int face)
{
    cudaError_t error = cudaSuccess;
	cudaArray* cuArray;
	void* cudaLinearMemory;

	error = cudaMalloc(&cudaLinearMemory, width * height * 4);
	getLastCudaError("cudaMallocPitch (g_texture_2d) failed");
	error = cudaMemset(cudaLinearMemory, 0, width * height * 3);

	error = cudaGraphicsSubResourceGetMappedArray(&cuArray, cudaResource, 0, 0);
	getLastCudaError("cudaGraphicsSubResourceGetMappedArray (cuda_texture_2d) failed");

	switch (face)
	{
	case 0:
		error = cudaBindTextureToArray(texRef0, cuArray);
		break;
	case 1:
		error = cudaBindTextureToArray(texRef1, cuArray);
		break;
	case 2:
		error = cudaBindTextureToArray(texRef2, cuArray);
		break;
	case 3:
		error = cudaBindTextureToArray(texRef3, cuArray);
		break;
	case 4:
		error = cudaBindTextureToArray(texRef4, cuArray);
		break;
	case 5:
		error = cudaBindTextureToArray(texRef5, cuArray);
		break;
	case 6:
		error = cudaBindTextureToArray(texRef6, cuArray);
		break;
	case 7:
		error = cudaBindTextureToArray(texRef7, cuArray);
		break;
	case 8:
		error = cudaBindTextureToArray(texRef8, cuArray);
		break;
	case 9:
		error = cudaBindTextureToArray(texRef9, cuArray);
		break;
	case 10:
		error = cudaBindTextureToArray(texRef10, cuArray);
		break;
	case 11:
		error = cudaBindTextureToArray(texRef11, cuArray);
		break;
	}
	
	getLastCudaError("cudaGraphicsSubResourceGetMappedArray (cuda_texture_2d) failed");

    dim3 Db = dim3(16, 16);   // block dimensions are fixed to be 256 threads
    dim3 Dg = dim3(((width/2)+Db.x-1)/Db.x, ((height/2)+Db.y-1)/Db.y);

    cuda_kernel_texture_2d<<<Dg,Db>>>((uint8_t*)cudaLinearMemory, width, height, face);

    error = cudaGetLastError();

    if (error != cudaSuccess)
    {
        printf("cuda_kernel_texture_2d() failed to launch error = %d\n", error);
    }

	return cudaLinearMemory;
}
