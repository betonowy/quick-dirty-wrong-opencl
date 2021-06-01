//
// Created by pekopeko on 01.06.2021.
//

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cstdio>

#include "stb_image.h"
#include "stb_image_write.h"

#include "Device.h"

Device &Device::Get() {
    static Device dev;
    return dev;
}

Device::Device() {
    cl_int errorCode;

    clGetPlatformIDs(1, &platformId, &retNumPlatforms);
    clGetDeviceIDs(platformId, CL_DEVICE_TYPE_DEFAULT, 1, &deviceId, &retNumDevices);
    clContext = clCreateContext(nullptr, 1, &deviceId, nullptr, nullptr, &errorCode);

    if (errorCode) fprintf(stderr, "Couldn't create context\n");

    clCommandQueue = clCreateCommandQueue(clContext, deviceId, 0, &errorCode);

    if (errorCode) fprintf(stderr, "Couldn't create command queue\n");
}

Device::~Device() {
    clFlush(clCommandQueue);
    clFinish(clCommandQueue);
    clReleaseKernel(clKernelBlur);
    clReleaseProgram(clProgram);
    clReleaseMemObject(clMemSrc);
    clReleaseMemObject(clMemDstBlur);
    clReleaseCommandQueue(clCommandQueue);
    clReleaseContext(clContext);
}

void Device::ExecuteProgram(const char *path, int blurParam) {
    auto &dev = Get();
    cl_int errorCode;

    auto file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }

    constexpr size_t MAX_FILE_SZ = 16384;

    char *source = new char[MAX_FILE_SZ];
    for (size_t i = 0; i < MAX_FILE_SZ; i++) {
        source[i] = 0;
    }

    fread(source, sizeof(char), MAX_FILE_SZ, file);

    if (source[0] == 0) fprintf(stderr, "Empty source code");

    fclose(file);

    auto imageSize = dev.imageSizeX * dev.imageSizeY * 4;

    dev.clMemSrc = clCreateBuffer(dev.clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR,
                                  imageSize * sizeof(char), dev.imageData.data(), &errorCode);

    dev.clMemBlurParam = clCreateBuffer(dev.clContext, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR,
                                  sizeof(blurParam), &blurParam, &errorCode);

    dev.clMemDstBlur = clCreateBuffer(dev.clContext, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
                                      imageSize * sizeof(char), nullptr, &errorCode);

    dev.clMemDstEdge = clCreateBuffer(dev.clContext, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
                                      imageSize * sizeof(char), nullptr, &errorCode);

    dev.clProgram = clCreateProgramWithSource(dev.clContext, 1, (const char **) &source, nullptr, &errorCode);

    if (errorCode) fprintf(stderr, "Couldn't create program with source\n");

    if (clBuildProgram(dev.clProgram, 1, &dev.deviceId, nullptr, nullptr, nullptr)) {
        fprintf(stderr, "Couldn't build program\n");
        size_t logSize = 0;
        clGetProgramBuildInfo(dev.clProgram, dev.deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        char *buffer = new char[logSize];
        clGetProgramBuildInfo(dev.clProgram, dev.deviceId, CL_PROGRAM_BUILD_LOG, logSize, buffer, nullptr);
        fprintf(stderr, buffer);
    }

    delete[] source;

    dev.clKernelBlur = clCreateKernel(dev.clProgram, "processBlur", &errorCode);
    dev.clKernelEdge = clCreateKernel(dev.clProgram, "processEdge", &errorCode);

    if (errorCode) fprintf(stderr, "Couldn't create kernel\n");

    if (clSetKernelArg(dev.clKernelBlur, 0, sizeof(cl_mem), &dev.clMemSrc))
        fprintf(stderr, "Couldn't set src parameter\n");

    if (clSetKernelArg(dev.clKernelBlur, 1, sizeof(cl_mem), &dev.clMemDstBlur))
        fprintf(stderr, "Couldn't set dst parameter\n");

    if (clSetKernelArg(dev.clKernelBlur, 2, sizeof(cl_mem), &dev.clMemBlurParam))
        fprintf(stderr, "Couldn't set blur parameter\n");

    if (clSetKernelArg(dev.clKernelEdge, 0, sizeof(cl_mem), &dev.clMemSrc))
        fprintf(stderr, "Couldn't set src parameter\n");

    if (clSetKernelArg(dev.clKernelEdge, 1, sizeof(cl_mem), &dev.clMemDstEdge))
        fprintf(stderr, "Couldn't set dst parameter\n");

    if (clSetKernelArg(dev.clKernelEdge, 2, sizeof(cl_mem), &dev.clMemBlurParam))
        fprintf(stderr, "Couldn't set blur parameter\n");

    const size_t globalSize = imageSize;
    if (clEnqueueNDRangeKernel(dev.clCommandQueue, dev.clKernelBlur, 1, nullptr, &globalSize,
                               nullptr, 0, nullptr, nullptr))
        fprintf(stderr, "Couldn't enqueue blur kernel\n");

    if (clEnqueueReadBuffer(dev.clCommandQueue, dev.clMemDstBlur, CL_TRUE, 0, imageSize * sizeof(char),
                            dev.imageDataBlur.data(), 0, nullptr, nullptr))
        fprintf(stderr, "Couldn't enqueue blur read\n");

    if (clEnqueueNDRangeKernel(dev.clCommandQueue, dev.clKernelEdge, 1, nullptr, &globalSize,
                               nullptr, 0, nullptr, nullptr))
        fprintf(stderr, "Couldn't enqueue edge kernel\n");

    if (clEnqueueReadBuffer(dev.clCommandQueue, dev.clMemDstEdge, CL_TRUE, 0, imageSize * sizeof(char),
                            dev.imageDataEdge.data(), 0, nullptr, nullptr))
        fprintf(stderr, "Couldn't enqueue edge read\n");
}

void Device::LoadImage(const char *path) {
    auto &dev = Get();
    auto file = fopen(path, "rb");

    auto data = stbi_load_from_file(file, &dev.imageSizeX, &dev.imageSizeY, nullptr, 4);

    fclose(file);

    size_t newSize = dev.imageSizeX * dev.imageSizeY * 4;

    dev.imageData.resize(newSize);
    dev.imageDataBlur.resize(newSize);
    dev.imageDataEdge.resize(newSize);

    memcpy(dev.imageData.data(), data, newSize);

    stbi_image_free(data);
}

void Device::SaveImage(const char *pathBlur, const char *pathEdge) {
    auto &dev = Get();

    stbi_write_png(pathBlur, dev.imageSizeX, dev.imageSizeY, 4, dev.imageDataBlur.data(), 4 * dev.imageSizeX);
    stbi_write_png(pathEdge, dev.imageSizeX, dev.imageSizeY, 4, dev.imageDataEdge.data(), 4 * dev.imageSizeX);
}
