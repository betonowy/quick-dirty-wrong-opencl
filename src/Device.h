//
// Created by pekopeko on 01.06.2021.
//

#ifndef MB_OPENCL_DEVICE_H
#define MB_OPENCL_DEVICE_H

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
    cl_kernel clKernelBlur{};
    cl_kernel clKernelEdge{};
    cl_mem clMemSrc{};
    cl_mem clMemBlurParam{};
    cl_mem clMemDstBlur{};
    cl_mem clMemDstEdge{};

    int imageSizeX{}, imageSizeY{};

    std::vector<unsigned char> imageData;
    std::vector<unsigned char> imageDataBlur;
    std::vector<unsigned char> imageDataEdge;

public:

    Device(const Device &) = delete;

    Device(Device &&) = delete;

    Device &operator=(const Device &) = delete;

    Device &operator=(Device &&) = delete;

    static Device &Get();

    static void ExecuteProgram(const char *path, int blurParam);

    static void LoadImage(const char *path);

    static void SaveImage(const char *pathBlur, const char *pathEdge);
};


#endif //MB_OPENCL_DEVICE_H
