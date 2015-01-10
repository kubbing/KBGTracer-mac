//
//  Geometry.h
//  Tracer
//
//  Created by Jakub Hladík on 02.01.15.
//  Copyright (c) 2015 jakubhladik.pro. All rights reserved.
//

#ifndef __Tracer__Geometry__
#define __Tracer__Geometry__

#include <Accelerate/Accelerate.h>

#include <cstring>
using namespace std;

typedef struct Vector {
    float x, y, z, w;
    
    Vector (float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    
    Vector operator+(const Vector& right) const {
        return Vector(x + right.x, y + right.y, z + right.z);
    }
    Vector operator-(const Vector& right) const {
        return Vector(x - right.x, y - right.y, z - right.z);
    }
    Vector operator-() const {
        return Vector(-x, -y, -z);
    }
    Vector operator*(const float constant) const {
        return Vector(x*constant, y*constant, z*constant);
    }
    Vector operator/(const float constant) const {
        return Vector(x/constant, y/constant, z/constant);
    }
    Vector& operator=(const Vector& right) {
        x = right.x;
        y = right.y;
        z = right.z;
        w = right.w;
        return *this;
    }
    Vector& norm() {
        float l = length();
        return *this = *this / l;
    }
    float length() const {
        return sqrtf(this->x*this->x + this->y*this->y + this->z*this->z);
    }
    Vector mul(const Vector& right) const {
        return Vector(x * right.x, y * right.y, z * right.z);
    }
    float dot(const Vector& right) const {
        return x * right.x + y * right.y + z * right.z;
    }
    Vector cross(const Vector &right) const {
        return Vector(y*right.z - z*right.y, z*right.x - x*right.z, x*right.y - y*right.x);
    }
} Vector;


typedef struct Ray {
    Vector origin;
    Vector direction;
    int ttl;
    
    Ray() : Ray(Vector(0, 0, 0, 1), Vector(0, 0, 0, 1)) {
        
    }
    Ray(Vector origin, Vector direction, int ttl = 0) {
        this->origin = origin;
        this->direction = direction;
        this->ttl = ttl;
    }
    
    static Ray cameraRay() {
        Ray ray(Vector(0, 0, 20), Vector(0, 0, -1));
        return ray;
    }
} Ray;


typedef enum Material {
    NONE,
    DIFFUSE,
    SPECULAR,
    REFRACTIVE
} Material;


typedef struct SceneObject {
    Vector position;
    Vector emission;
    Vector color;
    Material material;
    
    SceneObject() {
        
    }
    
    SceneObject(Vector position, Vector emission, Vector color, Material material) {
        this->position = position;
        this->emission = emission;
        this->color = color;
        this->material = material;
    }
    
    virtual bool intersect(Ray ray, float* distance) = 0;
} SceneObject;


typedef struct Sphere : public SceneObject {
    float radius;
    string str;
    
    Sphere (string str, Vector position, Vector emission, Vector color, Material material, float radius) : SceneObject(position, emission, color, material) {
        this->str = str;
        this->radius = radius;
    }
    
    bool intersect(Ray ray, float* distance) {
        Vector op = (position - ray.origin);
        float b = ray.direction.dot(op);
        float det = b * b - op.dot(op) + radius * radius;
        
        if (det < 0) {
            return false;
        } else {
            det = sqrtf(det);
        }
        
        float t, t0, t1;
        float eps = 1e-4;
        
        t0 = b - det;
        t1 = b + det;
        
        if (t0 < t1) {
            t = t0;
        }
        else {
            t = t1;
        }
        
        if (t > eps) {
            *distance = t;
            return true;
        }
        
        return false;
    }
} Sphere;

#endif /* defined(__Tracer__Geometry__) */
