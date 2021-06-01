struct ivec2 {
    int x, y;
};

struct ivec2 getCoordForId(uint id) {
    struct ivec2 a;
    a.x = (id / 4) % 1000;
    a.y = id / (1000 * 4);
    return a;
}

const struct ivec2 kernelCoords[9] = {{-1, -1}, {0, -1}, {1, -1},
                                      {-1,  0}, {0,  0}, {1,  0},
                                      {-1,  1}, {0,  1}, {1,  1}};

const float kernelValues[9] = {-1.41, -2, -1.41,
                               -2, 13.64,    -2,
                               -1.41, -2, -1.41};

int getIdForCoordComp(struct ivec2 coord, uint comp) {
    if (coord.x < 0) coord.x = 0;
    else if (coord.x > 999) coord.x = 999;
    
    if (coord.y < 0) coord.y = 0;
    else if (coord.y > 561) coord.y = 561;
    
    return comp + coord.x * 4 + coord.y * 1000 * 4;
}

float blur(uint id, __global uchar* data, int blurSize) {
    if (blurSize == 0) return data[id];

    float color = 0;

    struct ivec2 coord = getCoordForId(id);

    uint comp = id % 4;

    const int kernelSize = blurSize;

    int kernelCount = 0;

    for (int x = -kernelSize; x <= kernelSize; x++) {
        for (int y = -kernelSize; y <= kernelSize; y++) {
            struct ivec2 tempCoord = coord;
            tempCoord.x += x;
            tempCoord.y += y;

            color += data[getIdForCoordComp(tempCoord, comp)];

            kernelCount++;
        }
    }

    return color / kernelCount;
}

float edgeDetector(uint id, __global uchar* data, int blurSize) {
    float color = data[id];

    float colorMin = 255;
    float colorMax = 0;

    struct ivec2 coord = getCoordForId(id);

    uint comp = id % 4;

    for (int i = 0; i < 9; i++) {
        struct ivec2 tempCoord = coord;
        tempCoord.x += kernelCoords[i].x;
        tempCoord.y += kernelCoords[i].y;

        color = blur(getIdForCoordComp(tempCoord, comp), data, blurSize);

        colorMin = min(color, colorMin);
        colorMax = max(color, colorMax);
    }

    return (colorMax - colorMin) * sqrt((float) blurSize + 1);
}

__kernel void processEdge(__global uchar* imageData, __global uchar* outData, int blurSize) {
    uint id = get_global_id(0);
    uint comp = id % 4;

    struct ivec2 coord = getCoordForId(id);
    
    int compId = getIdForCoordComp(coord, 0);

    if (id % 4 == 3) {
        outData[id] = imageData[id];
    } else {
        float color = edgeDetector(id, imageData, blurSize);
        outData[id] = color;
    }
}
