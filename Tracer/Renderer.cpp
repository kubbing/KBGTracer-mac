//
//  Renderer.cpp
//  Tracer
//
//  Created by Jakub Hladík on 02.01.15.
//  Copyright (c) 2015 jakubhladik.pro. All rights reserved.
//

#include "Renderer.h"

#include <cstdlib>
#include <chrono>

Renderer::Renderer(Ray ray, unsigned width, unsigned height, unsigned nSamples)
{
    this->camera = ray;
    this->width = width;
    this->height = height;
    this->nSamples = nSamples;
    
    size_t bufSize = 4 * height * width * sizeof(unsigned char);
    this->renderedData = new unsigned char[bufSize];
    
    bufSize = 4 * height * width * sizeof(float);
    this->rawRenderedData = new float[bufSize];
    
    Sphere s1("Mirror", Vector(-2, 4, -8),        Vector(0.0, 0.0, 0.0),  Vector(1, 1, 1), SPECULAR, 10);
    Sphere s2("Blue", Vector(-9, -3,  3),       Vector(0.0, 0.0, 0.0),  Vector(1.0, 0.05, 0.05), DIFFUSE, 4);
    Sphere s3("Bottom", Vector(0,  -205, 0),    Vector(0.0, 0.0, 0.0),  Vector(0.98, 0.98, 0.98), DIFFUSE, 201);
    
    Sphere s4("Red", Vector(10, 10,  4),    Vector(0.0, 0, 0.0),  Vector(0, 1.0, 0), DIFFUSE, 7);
    
    Sphere s5("Green", Vector( 2, -15, 7),      Vector(0.0, 0.0, 0.0),  Vector(0.8, 1, 0.8), SPECULAR, 12);
    Sphere s6("Yellow", Vector(8, -1,  -1),       Vector(0.0, 0.0, 0.0),  Vector(1, 1, 0), DIFFUSE, 3.6);
    Sphere s7("", Vector( 2,  2, -2),           Vector(0.0, 0.0, 0.0),  Vector(1, 1, 0), DIFFUSE, 1);
    
    /*
     light
     */
    Sphere s8("Sun", Vector( -60,  40,  20), Vector(2, 2, 1.8), Vector(1, 1, 0.9), DIFFUSE, 40);
    
    this->objects.push_back(s1);
    this->objects.push_back(s2);
    this->objects.push_back(s3);
    this->objects.push_back(s4);
    this->objects.push_back(s5);
            this->objects.push_back(s6);
//            this->objects.push_back(s7);
    
    this->objects.push_back(s8);
    
//    this->objects.push_back(s9    );
}

Ray Renderer::coordinateAdjustedRay(long i, long j)
{
    float aspectRatio = width / (float)height;
    float fov = 45.0f; // vertical
    float angle = tanf(fov * 0.5f * M_PI / 180.0f);
    float dx = 2 * aspectRatio / (float)width;
    float dy = 2 / (float)height;
    
    Ray ray = camera;
    ray.direction.x = angle * ( (j + 0.5) * dx - aspectRatio) + (dx * 0.5*(drand48()-1));
    ray.direction.y = angle * (-(i + 0.5) * dy + 1) + (dy * 0.5*(drand48()-1));
    ray.direction.norm();
    
    return ray;
}

bool Renderer::intersect(Ray &ray, float *distance, unsigned int *index)
{
    float inf = 1e20;
    float tempDistance;
    float minDistance = inf;
    unsigned objId = -1;
    
    for(int i = 0; i < objects.size(); i++) {
        bool intersects = objects[i].intersect(ray, &tempDistance);
        if (intersects && tempDistance < minDistance) {
            minDistance = tempDistance;
            objId = i;
        }
    }
    
    if (minDistance < inf) {
        *index = objId;
        *distance = minDistance;
        return true;
    }
    else {
        return false;
    }
}

void Renderer::render()
{
    srand48((long)time(NULL));
    
    size_t jobCount = width*height;
    chrono::time_point<chrono::system_clock> t0;
    t0 = chrono::system_clock::now();
    
    dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_CONCURRENT, QOS_CLASS_BACKGROUND, 0);
    dispatch_apply(jobCount, dispatch_queue_create("vip queue", attr), ^(size_t jobId) {
        long y = jobId / width;
        long x = jobId % width;

        
        long index = 4 * (y*width + x);
        Vector color;
        /*
         2x2 supersampling
         */
        const unsigned ssCount = 2;
        for (unsigned sy = 0; sy < ssCount; sy++) {
            for (unsigned sx = 0; sx < ssCount; sx++) {
                Vector innerColor;
                /*
                 samples in subpixel
                 */
                for (unsigned s = 0; s < nSamples; s++) {
                    Ray ray = coordinateAdjustedRay(y, x);
                    innerColor = innerColor + (trace(ray) * (1.0f / (float)nSamples));
                }
                
                float ratio = 1.0f / (float)(ssCount*ssCount);
                this->rawRenderedData[index+0] += clamp(innerColor.x) * ratio;
                this->rawRenderedData[index+1] += clamp(innerColor.y) * ratio;
                this->rawRenderedData[index+2] += clamp(innerColor.z) * ratio;
                this->rawRenderedData[index+3]  = 1.0f;
            }
        }
        
        pixelsDone++;
        chrono::time_point<chrono::system_clock> t1;
        t1 = chrono::system_clock::now();
        chrono::duration<float> elapsed = t1 - t0;
        
        if (!(pixelsDone % 1000)) {
            float progress = (pixelsDone+1) / (float)jobCount;
            printf("%03.4f %%, average %2.2f Mrays/s\n", 100.0 * progress, (nSamples * pixelsDone) / elapsed.count() / 1e6);
        }
    });
}

Vector Renderer::trace(Ray ray)
{
    float distance;
    unsigned objectId;
    if (!intersect(ray, &distance, &objectId)) {
        return Vector();
    }
    
    if (ray.ttl++ > 20) {
        return Vector();
    }

    Sphere object = objects[objectId];
    
    Ray reflectionRay;
    reflectionRay.origin = ray.origin + (ray.direction * distance);
    reflectionRay.direction = (reflectionRay.origin - object.position).norm();
    
    Vector nl = reflectionRay.direction.dot(ray.direction) < 0? reflectionRay.direction : -reflectionRay.direction;
    Vector color = object.color;
    float reflection = (color.x > color.y && color.x > color.z) ? color.x : (color.y > color.z) ? color.y : color.z; // three-way if, highest color
    
    if (ray.ttl > 5) {
        if (drand48() < reflection) {
            color = color * (1 / reflection);
        }
        else {
            return object.emission;
        }
    }
    
    if (object.material == DIFFUSE) {
        float r1 = 2 * M_PI * drand48(); // random angle
        float r2 = drand48();
        float r2s = sqrtf(r2);
        
        Vector w = nl;
        Vector u = ((fabs(w.x) > 0.1 ? Vector(0, 1) : Vector(1)).cross(w)).norm();
        Vector v = w.cross(u);
        Vector d = (u*cosf(r1)*r2s + v*sinf(r1)*r2s + w*sqrtf(1-r2)).norm();
        
        Vector finalColor = object.emission + color.mul(trace(Ray(reflectionRay.origin, d)));
        return finalColor;
    }
    else if (object.material == SPECULAR) {
        return object.emission + color.mul(trace(Ray(reflectionRay.origin, ray.direction - reflectionRay.direction * 2 * reflectionRay.direction.dot(ray.direction))));
    }
    
    return Vector();
}