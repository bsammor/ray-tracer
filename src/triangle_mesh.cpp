#include <triangle_mesh.h>

TriangleMesh::TriangleMesh(const char* file, Color color, Material material)
{
	std::string inputfile = file;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());

	if (!warn.empty()) 
    {
		std::cout << warn << std::endl;
	}

	if (!err.empty()) 
    {
		std::cerr << err << std::endl;
	}

	if (!ret) 
    {
		exit(1);
	}

    this->color = color;
    this->material = material;
    this->calculate_bbox();
}

bool TriangleMesh::intersected(Ray* ray, int index, double& u, double& v, double& t)
{
    bool polygon_hit = false;
    for (auto& shape : shapes) 
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
        {
            Vec3 vn0, vn1, vn2, vt0, vt1, vt2;

            int fv = shape.mesh.num_face_vertices[f];
            tinyobj::index_t& idx0 = shape.mesh.indices[index_offset + 0];  // v0
            tinyobj::index_t& idx1 = shape.mesh.indices[index_offset + 1];  // v1
            tinyobj::index_t& idx2 = shape.mesh.indices[index_offset + 2];  // v2

            Vec3 v0 = Vec3(attrib.vertices[3 * idx0.vertex_index + 0],
                attrib.vertices[3 * idx0.vertex_index + 1],
                attrib.vertices[3 * idx0.vertex_index + 2]);
            Vec3 v1 = Vec3(attrib.vertices[3 * idx1.vertex_index + 0],
                attrib.vertices[3 * idx1.vertex_index + 1],
                attrib.vertices[3 * idx1.vertex_index + 2]);
            Vec3 v2 = Vec3(attrib.vertices[3 * idx2.vertex_index + 0],
                attrib.vertices[3 * idx2.vertex_index + 1],
                attrib.vertices[3 * idx2.vertex_index + 2]);

            if (attrib.normals.size() > 0) {
                vn0 = Vec3(attrib.normals[3 * idx0.normal_index + 0],
                    attrib.normals[3 * idx0.normal_index + 1],
                    attrib.normals[3 * idx0.normal_index + 2]);
                vn1 = Vec3(attrib.normals[3 * idx1.normal_index + 0],
                    attrib.normals[3 * idx1.normal_index + 1],
                    attrib.normals[3 * idx1.normal_index + 2]);
                vn2 = Vec3(attrib.normals[3 * idx2.normal_index + 0],
                    attrib.normals[3 * idx2.normal_index + 1],
                    attrib.normals[3 * idx2.normal_index + 2]);
            }

            if (attrib.texcoords.size() > 0) {
                vt0 = Vec3(attrib.texcoords[2 * idx0.texcoord_index + 0],
                    attrib.texcoords[2 * idx0.texcoord_index + 1],
                    0.0);
                vt1 = Vec3(attrib.texcoords[2 * idx1.texcoord_index + 0],
                    attrib.texcoords[2 * idx1.texcoord_index + 1],
                    0.0);
                vt2 = Vec3(attrib.texcoords[2 * idx2.texcoord_index + 0],
                    attrib.texcoords[2 * idx2.texcoord_index + 1],
                    0.0);
            }

            double x = 0.0, y = 0.0;

            Triangle face = Triangle(v0, v1, v2, Color(1, 1, 1), diffuse);
            if (face.intersected(ray, index, x, y, t))
            {
                ray->set_tmax(t);
                u = x;
                v = y;
                polygon_hit = true;
                tri = face;
                tri_vnormal = (vn0 * (1 - x - y) + vn1 * x + vn2 * y).normalize();
                tri_tex_coordinates = vt0 * (1 - x - y) + vt1 * x + vt2 * y;
                tri_fnormal = face.get_normal(Vec3());
                //int o = shape.mesh.material_ids[f];
                //tex_name = materials[o].diffuse_texname;
            }
            index_offset += fv;
        }
    }
    return polygon_hit;
}

Vec3 TriangleMesh::get_normal(Vec3 point)
{
    return tri_vnormal;
}

void TriangleMesh::calculate_bbox() 
{
    std::cout << attrib.vertices.size();

    for (auto& shape : shapes) 
    {
        for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
            tinyobj::index_t& idx0 = shape.mesh.indices[i];  // v0
            double x = attrib.vertices[3 * idx0.vertex_index + 0];
            double y = attrib.vertices[3 * idx0.vertex_index + 1];
            double z = attrib.vertices[3 * idx0.vertex_index + 2];

            bbox.min = Vec3(std::max(bbox.min.x, x), std::max(bbox.min.y, y),std::max(bbox.min.z, z));
            bbox.max = Vec3(std::max(bbox.max.x, x), std::max(bbox.max.y, y),std::max(bbox.max.z, z));
        }

    }
    /*for (auto& shape : shapes) 
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
        {
            int fv = shape.mesh.num_face_vertices[f];
            tinyobj::index_t& idx0 = shape.mesh.indices[index_offset + 0];  // v0
            tinyobj::index_t& idx1 = shape.mesh.indices[index_offset + 1];  // v1
            tinyobj::index_t& idx2 = shape.mesh.indices[index_offset + 2];  // v2

            Vec3 v0 = Vec3(attrib.vertices[3 * idx0.vertex_index + 0],
                attrib.vertices[3 * idx0.vertex_index + 1],
                attrib.vertices[3 * idx0.vertex_index + 2]);
            Vec3 v1 = Vec3(attrib.vertices[3 * idx1.vertex_index + 0],
                attrib.vertices[3 * idx1.vertex_index + 1],
                attrib.vertices[3 * idx1.vertex_index + 2]);
            Vec3 v2 = Vec3(attrib.vertices[3 * idx2.vertex_index + 0],
                attrib.vertices[3 * idx2.vertex_index + 1],
                attrib.vertices[3 * idx2.vertex_index + 2]);

            if ((v0.x + v0.y + v0.z) > (bbox.max.x + bbox.max.y + bbox.max.z))
                bbox.max = v0;
            else if ((v0.x + v0.y + v0.z) < (bbox.min.x + bbox.min.y + bbox.min.z))
                bbox.min = v0;

            if ((v1.x + v1.y + v1.z) > (bbox.max.x + bbox.max.y + bbox.max.z))
                bbox.max = v1;
            else if ((v1.x + v1.y + v1.z) < (bbox.min.x + bbox.min.y + bbox.min.z))
                bbox.min = v1;

            if ((v2.x + v2.y + v2.z) > (bbox.max.x + bbox.max.y + bbox.max.z))
                bbox.max = v2;
            else if ((v2.x + v2.y + v2.z) < (bbox.min.x + bbox.min.y + bbox.min.z))
                bbox.min = v2;

            index_offset += fv;
        }
    }*/
}
