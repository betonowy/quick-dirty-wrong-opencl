#include "src/Device.h"

int main() {
    Device::LoadImage("../source.png");
    Device::ExecuteProgram("../main.cl", 0, 3);
    Device::SaveImage("../output1.png", "../output2.png");
    return 0;
}