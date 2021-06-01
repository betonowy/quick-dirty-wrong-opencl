#include "src/Device.h"

int main() {
    Device::LoadImage("../source.png");
    Device::ExecuteProgram("../main.cl", 5);
    Device::SaveImage("../afterBlur.png", "../afterEdge.png");
    return 0;
}