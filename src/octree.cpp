#include <octree.h>

bool is_overlapping(BBOX tri_bounds, BBOX child_bounds)
{

    bool x = (child_bounds.max.x >= tri_bounds.min.x) && (child_bounds.min.x <= tri_bounds.max.x);
    bool y = (child_bounds.max.y >= tri_bounds.min.y) && (child_bounds.min.y <= tri_bounds.max.y);
    bool z = (child_bounds.max.z >= tri_bounds.min.z) && (child_bounds.min.z <= tri_bounds.max.z);
    return (x && y && z);
}

void get_children_bbox(std::vector<BBOX> &children_bounds, BBOX parent_bounds)
{
    Vec3 min = parent_bounds.min;
    Vec3 max = parent_bounds.max;
    Vec3 center = parent_bounds.get_center();

    // 8 corners of the bbox
    Vec3 corners[8] = {Vec3(min.x, min.y, min.z), Vec3(min.x, min.y, max.z), Vec3(min.x, max.y, min.z), Vec3(min.x, max.y, max.z), 
        Vec3(max.x, min.y, min.z), Vec3(max.x, min.y, max.z), Vec3(max.x, max.y, min.z), Vec3(max.x, max.y, max.z)};

    for (Vec3 v : corners)
    {
        BBOX child;
        child.min = Vec3(std::min(v.x, center.x), std::min(v.y, center.y), std::min(v.z, center.z));
        child.max = Vec3(std::max(v.x, center.x), std::max(v.y, center.y), std::max(v.z, center.z));
        children_bounds.push_back(child);
    }
}

Octree::Octree(const std::vector<std::shared_ptr<Object>> p, BBOX b, unsigned max_depth, unsigned min_prims, unsigned depth)
{
    for (int i = 0; i < 8; i ++)
        children[i] = nullptr;

    bounds = b;

    // check exit conditions
    if (p.size() < 1) return;
    if (p.size() <= min_prims || depth >= max_depth)
    {
        this->primitives = p;
        return;
    }

    std::vector<BBOX> children_bounds;
    get_children_bbox(children_bounds, bounds);
    
    for (int i = 0; i < 8; i++)
    {
        //figure out which primitives belong in this child using bbox overlaps
        std::vector<std::shared_ptr<Object>> childp;
        for (unsigned j = 0; j < p.size(); j++)
        {
            if (is_overlapping(children_bounds[i], p[j]->get_bbox()))
                childp.push_back(p[j]);
        }

        children[i] = new Octree(childp, children_bounds[i], max_depth, min_prims, depth+1);
    }
}

int Octree::first_node(double tx0, double ty0, double tz0, double txm, double tym, double tzm)
{
    unsigned char result = 0;  

    if (tx0 > ty0 && tx0 > tz0)
    {
        if (tym < tx0) result |= 2;
        if (tzm < tx0) result |= 1;
        return (int) result;
    }
    else if (ty0 > tz0) 
    {
        if(txm < ty0) result |= 4;  
        if(tzm < ty0) result |= 1; 
        return (int) result;
    }

    if (txm < tz0) result |= 4;    
    if (tym < tz0) result |= 2;
    
    return (int) result;
}

int Octree::next_node(double txm, int x, double tym, int y, double tzm, int z)
{
    if (txm < tym && txm < tzm) return x;
    else if (tym < tzm) return y;

    return z;
}

void Octree::proc_subtree(double tx0, double ty0, double tz0, double tx1, double ty1, double tz1, Octree *node, std::vector<Octree*> &hit_nodes, std::shared_ptr<Ray> ray)
{
    float txm, tym, tzm;
    int curr_node;

    if (tx1 < 0 || ty1 < 0 || tz1 < 0) return;
    if (node->children[0] == nullptr)
    {
        hit_nodes.push_back(node);
        return;
    }

    txm = (tx0 + tx1)/2;
    tym = (ty0 + ty1)/2;
    tzm = (tz0 + tz1)/2;

    curr_node = first_node(tx0, ty0, tz0, txm, tym, tzm);
    do
    {
        switch (curr_node)
        {
            case 0: 
            { 
                proc_subtree(tx0, ty0, tz0, txm, tym, tzm, node->children[ray->a], hit_nodes, ray);
                curr_node = next_node(txm, 4, tym, 2, tzm, 1);
                break;
            }
            case 1: 
            { 
                proc_subtree(tx0, ty0, tzm, txm, tym, tz1, node->children[1^ray->a], hit_nodes, ray);
                curr_node = next_node(txm, 5, tym, 3, tz1, 8);
                break;
            }
            case 2: 
            { 
                proc_subtree(tx0, tym, tz0, txm, ty1, tzm, node->children[2^ray->a], hit_nodes, ray);
                curr_node = next_node(txm, 6, ty1, 8, tzm, 3);
                break;
            }
            case 3: 
            { 
                proc_subtree(tx0, tym, tzm, txm, ty1, tz1, node->children[3^ray->a], hit_nodes, ray);
                curr_node = next_node(txm, 7, ty1, 8, tz1, 8);
                break;
            }
            case 4: 
            { 
                proc_subtree(txm, ty0, tz0, tx1, tym, tzm, node->children[4^ray->a], hit_nodes, ray);
                curr_node = next_node(tx1, 8, tym, 6, tzm, 5);
                break;
            }
            case 5: 
            { 
                proc_subtree(txm, ty0, tzm, tx1, tym, tz1, node->children[5^ray->a], hit_nodes, ray);
                curr_node = next_node(tx1, 8, tym, 7, tz1, 8);
                break;
            }
            case 6: 
            { 
                proc_subtree(txm, tym, tz0, tx1, ty1, tzm, node->children[6^ray->a], hit_nodes, ray);
                curr_node = next_node(tx1, 8, ty1, 8, tzm, 7);
                break;
            }
            case 7: 
            { 
                proc_subtree(txm, tym, tzm, tx1, ty1, tz1, node->children[7^ray->a], hit_nodes, ray);
                curr_node = 8;
                break;
            }
        }
    } while (curr_node < 8);
}

bool Octree::intersect_tree(std::shared_ptr<Ray> ray)
{
    ray->a = 0;
    bool intersected = false;
    std::vector<Octree*> hit_nodes;

    if (ray->get_direction()[0] < 0) ray->a |= 4 ;
    if (ray->get_direction()[1] < 0) ray->a |= 2 ; 
    if (ray->get_direction()[2] < 0) ray->a |= 1 ; 
 
    Vec3 inv_dir = Vec3(1 / ray->get_direction().x, 1 / ray->get_direction().y, 1 / ray->get_direction().z); 
    int dir_is_neg[3];
    dir_is_neg[0] = (inv_dir.x < 0); 
    dir_is_neg[1] = (inv_dir.y < 0); 
    dir_is_neg[2] = (inv_dir.z < 0); 

    double tx0 = (bounds[dir_is_neg[0]].x - ray->get_origin().x) * inv_dir.x;
    double tx1 = (bounds[1 - dir_is_neg[0]].x - ray->get_origin().x) * inv_dir.x;
    double ty0 = (bounds[dir_is_neg[1]].y - ray->get_origin().y) * inv_dir.y;
    double ty1 = (bounds[1 - dir_is_neg[1]].y - ray->get_origin().y) * inv_dir.y;
    double tz0 = (bounds[dir_is_neg[2]].z - ray->get_origin().z) * inv_dir.z;
    double tz1 = (bounds[1 - dir_is_neg[2]].z - ray->get_origin().z) * inv_dir.z;

	if (bounds.intersected(ray, inv_dir, dir_is_neg)) proc_subtree(tx0, ty0, tz0, tx1, ty1, tz1, this, hit_nodes, ray);

    for (unsigned i = 0; i < hit_nodes.size(); i++)
        for (unsigned j = 0; j < hit_nodes[i]->primitives.size(); j++)
            if (hit_nodes[i]->primitives[j]->intersected(ray, j)) intersected = true;

    return intersected;
}