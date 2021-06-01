//
// Created by pekopeko on 01.06.2021.
//

#ifndef MB_OPENCL_DEVICE_H
#define MB_OPENCL_DEVICE_H

#define CL_TARGET_OPENCL_VERSION 120

#include <CL/cl.h>
#include <vector>

class Device {
    Device();

    ~Device();

    cl_device_id deviceId{};
    cl_platform_id platformId{};
    cl_uint retNumPlatforms{};
    cl_uint retNumDevices{};
    cl_context clContext{};
    cl_command_queue clCommandQueue{};
    cl_program clProgram{};
    cl_kernel clKernelEdge{};
    cl_mem clMemSrc{};
    cl_mem clMemDst{};

    int imageSizeX{}, imageSizeY{};

    std::vector<unsigned char> imageData;
    std::vector<unsigned char> imageDataEdge2;
    std::vector<unsigned char> imageDataEdge1;

public:

    Device(const Device &) = delete;

    Device(Device &&) = delete;

    Device &operator=(const Device &) = delete;

    Device &operator=(Device &&) = delete;

    static Device &Get();

    static void ExecuteProgram(const char *path, int blurParamFirst, int blurParamSecond);

    static void LoadImage(const char *path);

    static void SaveImage(const char *pathFirst, const char *pathSecond);
};


#endif //MB_OPENCL_DEVICE_H
