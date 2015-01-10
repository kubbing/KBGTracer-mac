//
//  Renderer.h
//  Tracer
//
//  Created by Jakub Hladík on 02.01.15.
//  Copyright (c) 2015 jakubhladik.pro. All rights reserved.
//

#ifndef __Tracer__Renderer__
#define __Tracer__Renderer__

#include "Geometry.h"

#include <vector>
#include <atomic>
using namespace std;

inline float clamp(float x) {
    return x<0 ? 0 : x>1 ? 1 : x;
}

inline int toInt(float x) {
    return int(pow(clamp(x), 1/2.2f) * 255.0f + .5f);
}

class Renderer {
    Ray camera;
    unsigned width, height, nSamples;
    float *rawRenderedData;
    unsigned char *renderedData;
    vector<Sphere> objects;
    atomic<unsigned> pixelsDone;
    
public:
    Renderer() : Renderer(Ray::cameraRay(), 1280, 800, 100) {

    }
    ~ Renderer() {
        delete [] this->renderedData;
        delete [] this->rawRenderedData;
    }
    
    Renderer(Ray ray, unsigned width, unsigned height, unsigned nSamples);

    Ray coordinateAdjustedRay(long i, long j);
    void render();
    Vector trace(Ray ray);
    bool intersect(Ray& ray, float* distance, unsigned* index);
    
    unsigned char* getImageData(unsigned* width, unsigned* height, float* progress) const {
        *width = this->width;
        *height = this->height;
        
        unsigned size = (this->width * this->height);
        *progress = this->pixelsDone / (float)size;
        
        for (unsigned counter = 0; counter < 4*size; counter++) {
            int i = toInt(rawRenderedData[counter]);
            renderedData[counter] = i;
        }
        
        return renderedData;
    }
};

#endif /* defined(__Tracer__Renderer__) */
