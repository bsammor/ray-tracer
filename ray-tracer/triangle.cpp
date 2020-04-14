#include "triangle.h"
#include <iostream>

Triangle::Triangle()
{
    v0 = Vec3();
    v1 = Vec3(1, 1, 1);
    v2 = Vec3(2, 2, 2);
}

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color, Material material)
{
    this->v0 = v0;
    this->v1 = v1;
    this->v2 = v2;
    this->color = color;
    this->material = material;
}

bool Triangle::intersected(Ray* ray, int index)
{
    float t = 0.0;
    float u = 0.0, v = 0.0;
    // compute plane's normal

    Vec3 v0v1 = v1 - v0;
    Vec3 v0v2 = v2 - v0;
    // no need to normalize
    Vec3 N = v0v1.cross_product(v0v2); // N 
    float denom = N.dot_product(N);
    //std::cout << denom << std::endl;
    // Step 1: finding P

    // check if ray and plane are parallel ?
    float NdotRayDirection = N.dot_product(ray->get_direction());
    //std::cout << NdotRayDirection << std::endl;
    if (fabs(NdotRayDirection) < 1e-8) // almost 0 
        return false; // they are parallel so they don't intersect ! 
    // compute d parameter using equation 2
    float d = N.dot_product(v0);
    //std::cout << d << std::endl;

    // compute t (equation 3)
    t = (N.dot_product(ray->get_origin()) + d) / NdotRayDirection;
    //std::cout << t << std::endl;
    // check if the triangle is in behind the ray
    if (t < 0) return false; // the triangle is behind 
    // compute the intersection point using equation 1
    Vec3 P = ray->get_origin() + (ray->get_direction() * t);

    // Step 2: inside-outside test
    Vec3 C; // vector perpendicular to triangle's plane 

    // edge 0
    Vec3 edge0 = v1 - v0;
    Vec3 vp0 = P - v0;
    C = edge0.cross_product(vp0);
    if (N.dot_product(C) < 0) return false; // P is on the right side 
    // edge 1
    Vec3 edge1 = v2 - v1;
    Vec3 vp1 = P - v1;
    C = edge1.cross_product(vp1);
    if ((u = N.dot_product(C)) < 0)  return false; // P is on the right side 
    // edge 2
    Vec3 edge2 = v0 - v2;
    Vec3 vp2 = P - v2;
    C = edge2.cross_product(vp2);
    if ((v = N.dot_product(C)) < 0) return false; // P is on the right side; 
    u /= denom;
    v /= denom;

    ray->set_index(index);
    //std::cout << "yes";
    return true; // this ray hits the triangle 
}

Ray* Triangle::create_shadow_ray(Ray* camera_ray, Light light)
{
	return nullptr;
}

Vec3 Triangle::get_normal(Vec3 point)
{
    return Vec3();
}
