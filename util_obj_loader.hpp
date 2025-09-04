#pragma once

#include "util_std.hpp"
#include "util_std_bonus.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

namespace vicmil
{
    struct Vertex
    {
        vicmil::Coord_XYZ_f vertex_cord;
        vicmil::TexCoord_UV_f tex_cord;
        vicmil::Normal_XYZ_f norm;
        unsigned int material_id;
    };

    typedef TriangleIndices_i3 Face;

    struct Material
    {
        std::string name;
        std::string diffuse_texname;  // Texture map_Kd, this is the main texture that will be drawn on the object
        std::string ambient_texname;  // Texture map_Ka
        std::string specular_texname; // Texture map_Ks
        std::string bump_texname;     // Texture bump
        std::string emissive_texname; // Emissive texture map_Ke

        // Material properties
        float Ka[3] = {0.2f, 0.2f, 0.2f}; // Ambient reflectivity
        float Kd[3] = {0.8f, 0.8f, 0.8f}; // Diffuse reflectivity # This is the default color that will be shown
        float Ks[3] = {1.0f, 1.0f, 1.0f}; // Specular reflectivity
        float Ke[3] = {0.0f, 0.0f, 0.0f}; // Emissive color
        float Kr[3] = {0.0f, 0.0f, 0.0f}; // Reflection color
        float Ns = 0.0f;                  // Specular exponent (shininess)
        float Ni = 1.0f;                  // Index of refraction
        float d = 1.0f;                   // Transparency (0.0 = fully transparent, 1.0 = fully opaque)
        int illum = 2;                    // Illumination model
        float metallic = 0.0f;            // Metallic factor (PBR)
        float roughness = 0.0f;           // Roughness factor (PBR)

        // Helper function to convert a float array to string
        std::string _array_to_string(const float arr[3]) const
        {
            std::ostringstream oss;
            oss << "[" << arr[0] << ", " << arr[1] << ", " << arr[2] << "]";
            return oss.str();
        }

        // Member method to convert Material to string
        std::string to_string() const
        {
            std::ostringstream oss;
            oss << "Material: " << name << "\n";
            oss << "  Textures:\n";
            oss << "    Diffuse: " << diffuse_texname << "\n";
            oss << "    Ambient: " << ambient_texname << "\n";
            oss << "    Specular: " << specular_texname << "\n";
            oss << "    Bump: " << bump_texname << "\n";
            oss << "    Emissive: " << emissive_texname << "\n";
            oss << "  Properties:\n";
            oss << "    Ka: " << _array_to_string(Ka) << "\n";
            oss << "    Kd: " << _array_to_string(Kd) << "\n";
            oss << "    Ks: " << _array_to_string(Ks) << "\n";
            oss << "    Ke: " << _array_to_string(Ke) << "\n";
            oss << "    Kr: " << _array_to_string(Kr) << "\n";
            oss << "    Ns: " << Ns << "\n";
            oss << "    Ni: " << Ni << "\n";
            oss << "    d: " << d << "\n";
            oss << "    illum: " << illum << "\n";
            oss << "    metallic: " << metallic << "\n";
            oss << "    roughness: " << roughness << "\n";
            return oss.str();
        }
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<Face> faces;
        std::vector<Material> materials;
        std::map<std::string, ImageRGBA_UChar> embedded_images; // Map of embedded images by name

        std::vector<vicmil::Coord_XYZ_f> get_vertex_coordinates() const
        { // float x, float y, float z
            std::vector<vicmil::Coord_XYZ_f> ret_vec = std::vector<vicmil::Coord_XYZ_f>();
            ret_vec.reserve(vertices.size());
            for (int i = 0; i < vertices.size(); i++)
            {
                ret_vec.push_back(vertices[i].vertex_cord);
            }
            return ret_vec;
        }
        std::vector<vicmil::TexCoord_UV_f> get_vertex_texture_coordinates() const
        { // float x, float y, float z, float u, float v
            std::vector<vicmil::TexCoord_UV_f> ret_vec = std::vector<vicmil::TexCoord_UV_f>();
            ret_vec.reserve(vertices.size());
            for (int i = 0; i < vertices.size(); i++)
            {
                const Vertex &v = vertices[i];
                ret_vec.push_back(vicmil::TexCoord_UV_f(v.tex_cord.u, v.tex_cord.v));
            }
            return ret_vec;
        }
        std::vector<int> get_vertex_material_indices() const
        {
            std::vector<int> ret_vec = std::vector<int>();
            ret_vec.reserve(vertices.size());
            for (int i = 0; i < vertices.size(); i++)
            {
                const Vertex &v = vertices[i];
                ret_vec.push_back(v.material_id);
            }
            return ret_vec;
        }
        std::vector<TriangleIndices_i3> get_vertex_triangle_indicies() const
        { // int vert1, int vert2, int vert3
            std::vector<TriangleIndices_i3> ret_vec = std::vector<TriangleIndices_i3>();
            ret_vec.reserve(faces.size());
            for (int i = 0; i < faces.size(); i++)
            {
                ret_vec.push_back(TriangleIndices_i3(faces[i].vertex_indices[0], faces[i].vertex_indices[1], faces[i].vertex_indices[2]));
            }
            return ret_vec;
        }
        std::vector<std::string> get_material_texture_names() const
        {
            std::vector<std::string> ret_vec = std::vector<std::string>();
            ret_vec.reserve(materials.size());
            for (int i = 0; i < materials.size(); i++)
            {
                ret_vec.push_back(materials[i].diffuse_texname);
            }
            return ret_vec;
        }
        std::vector<ColorRGBA_f> get_material_colors() const
        {
            std::vector<ColorRGBA_f> ret_vec = std::vector<ColorRGBA_f>();
            ret_vec.reserve(materials.size());
            for (int i = 0; i < materials.size(); i++)
            {
                float r = materials[i].Kd[0];
                float g = materials[i].Kd[1];
                float b = materials[i].Kd[2];
                ret_vec.push_back(ColorRGBA_f(r, g, b, 255));
            }
            return ret_vec;
        }
        int get_triangle_count()
        {
            return faces.size();
        }
        void get_center_and_radius(vicmil::Coord_XYZ_f *center, float *radius)
        {
            // Get the center of the model
            double x = 0;
            double y = 0;
            double z = 0;
            for (int i = 0; i < vertices.size(); i++)
            {
                x += vertices[i].vertex_cord.x;
                y += vertices[i].vertex_cord.y;
                z += vertices[i].vertex_cord.z;
            }
            *center = vicmil::Coord_XYZ_f((float)x / vertices.size(), (float)y / vertices.size(), (float)z / vertices.size());

            // Compute the radius of the model
            float radius_sq = 0;
            for (int i = 0; i < vertices.size(); i++)
            {
                float center_dist =
                    (center->x - vertices[i].vertex_cord.x) * (center->x - vertices[i].vertex_cord.x) +
                    (center->y - vertices[i].vertex_cord.y) * (center->y - vertices[i].vertex_cord.y) +
                    (center->z - vertices[i].vertex_cord.z) * (center->z - vertices[i].vertex_cord.z);
                radius_sq = std::max(radius_sq, center_dist);
            }
            *radius = std::sqrt(radius_sq);
        }
        void to_obj_file_in_memory(std::string mtl_file_name,
                                   std::string *obj_file,
                                   std::string *mtl_file)
        {
            std::ostringstream obj;
            std::ostringstream mtl;

            // Link the MTL file
            obj << "mtllib " << mtl_file_name << "\n";

            // Write vertices
            for (const auto &v : vertices)
            {
                obj << "v " << v.vertex_cord.x << " " << v.vertex_cord.y << " " << v.vertex_cord.z << "\n";
            }

            // Write texture coordinates
            for (const auto &v : vertices)
            {
                obj << "vt " << v.tex_cord.u << " " << v.tex_cord.v << "\n";
            }

            // Write normals
            for (const auto &v : vertices)
            {
                obj << "vn " << v.norm.x << " " << v.norm.y << " " << v.norm.z << "\n";
            }

            // Group faces by material
            int current_material = -1;
            for (const auto &f : faces)
            {
                int mat_id = vertices[f.vertex_indices[0]].material_id;
                if (mat_id != current_material)
                {
                    current_material = mat_id;
                    if (mat_id >= 0 && mat_id < (int)materials.size())
                    {
                        obj << "usemtl " << materials[mat_id].name << "\n";
                    }
                }

                // Face indices in OBJ are 1-based
                obj << "f";
                for (int i = 0; i < 3; i++)
                {
                    int idx = f.vertex_indices[i] + 1;
                    obj << " " << idx << "/" << idx << "/" << idx;
                }
                obj << "\n";
            }

            // Write MTL file
            for (const auto &m : materials)
            {
                mtl << "newmtl " << m.name << "\n";
                mtl << "Ka " << m.Ka[0] << " " << m.Ka[1] << " " << m.Ka[2] << "\n";
                mtl << "Kd " << m.Kd[0] << " " << m.Kd[1] << " " << m.Kd[2] << "\n";
                mtl << "Ks " << m.Ks[0] << " " << m.Ks[1] << " " << m.Ks[2] << "\n";
                mtl << "Ke " << m.Ke[0] << " " << m.Ke[1] << " " << m.Ke[2] << "\n";
                mtl << "d " << m.d << "\n";
                mtl << "Ns " << m.Ns << "\n";
                mtl << "illum " << m.illum << "\n";

                if (!m.diffuse_texname.empty())
                {
                    mtl << "map_Kd " << m.diffuse_texname;
                    if (get_file_extension(m.diffuse_texname) != "png")
                    {
                        mtl << ".png";
                    }
                    mtl << "\n";
                }

                if (!m.ambient_texname.empty())
                {
                    mtl << "map_Ka " << m.ambient_texname;
                    if (get_file_extension(m.ambient_texname) != "png")
                    {
                        mtl << ".png";
                    }
                    mtl << "\n";
                }
                if (!m.specular_texname.empty())
                {
                    mtl << "map_Ks " << m.specular_texname;
                    if (get_file_extension(m.specular_texname) != "png")
                    {
                        mtl << ".png";
                    }
                    mtl << "\n";
                }
                if (!m.bump_texname.empty())
                {
                    mtl << "map_Bump " << m.bump_texname;
                    if (get_file_extension(m.bump_texname) != "png")
                    {
                        mtl << ".png";
                    }
                    mtl << "\n";
                }
                if (!m.emissive_texname.empty())
                {
                    mtl << "map_Ke " << m.emissive_texname;
                    if (get_file_extension(m.emissive_texname) != "png")
                    {
                        mtl << ".png";
                    }
                    mtl << "\n";
                }

                mtl << "\n";
            }

            *obj_file = obj.str();
            *mtl_file = mtl.str();
        }
        void to_obj_file(std::string obj_file_name, std::string output_dir)
        {
            // Note! If the model uses any images, then you have to save those yourself
            std::string obj_file_content;
            std::string mtl_file_content;
            std::string mtl_file_name = vicmil::string_replace(obj_file_name, ".obj", ".mtl");
            to_obj_file_in_memory(mtl_file_name, &obj_file_content, &mtl_file_content);
            if (output_dir.size() > 0 && output_dir.back() != '/')
            {
                output_dir = output_dir + "/";
            }
            vicmil::FileManager obj_file_manager = vicmil::FileManager(output_dir + obj_file_name, true);
            obj_file_manager.erase_file_contents();
            obj_file_manager.write_str(obj_file_content);
            vicmil::FileManager mtl_file_manager = vicmil::FileManager(output_dir + mtl_file_name, true);
            mtl_file_manager.erase_file_contents();
            mtl_file_manager.write_str(mtl_file_content);
        }
        std::string get_metadata()
        {
            std::ostringstream meta;

            meta << "Mesh Metadata:\n";
            meta << "-----------------\n";
            meta << "Number of vertices: " << vertices.size() << "\n";
            meta << "Number of faces: " << faces.size() << "\n";
            meta << "Number of materials: " << materials.size() << "\n";
            meta << "Number of embedded images: " << embedded_images.size() << "\n";

            if (!materials.empty())
            {
                meta << "Materials:\n";
                for (const auto &m : materials)
                {
                    meta << "  - " << m.name << "\n";
                }
            }

            if (!vertices.empty())
            {
                // Compute bounding box
                Coord_XYZ_f min(vertices[0].vertex_cord), max(vertices[0].vertex_cord);
                for (const auto &v : vertices)
                {
                    const auto &c = v.vertex_cord;
                    if (c.x < min.x)
                        min.x = c.x;
                    if (c.y < min.y)
                        min.y = c.y;
                    if (c.z < min.z)
                        min.z = c.z;
                    if (c.x > max.x)
                        max.x = c.x;
                    if (c.y > max.y)
                        max.y = c.y;
                    if (c.z > max.z)
                        max.z = c.z;
                }
                meta << "Bounding Box:\n";
                meta << "  Min: (" << min.x << ", " << min.y << ", " << min.z << ")\n";
                meta << "  Max: (" << max.x << ", " << max.y << ", " << max.z << ")\n";
            }

            return meta.str();
        }
    };

    struct _RawMesh
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
    };

    Mesh _load_obj_file(_RawMesh &raw_mesh)
    {
        tinyobj::attrib_t &attrib = raw_mesh.attrib;
        std::vector<tinyobj::shape_t> &shapes = raw_mesh.shapes;
        std::vector<tinyobj::material_t> &materials = raw_mesh.materials;

        Mesh mesh;

        // Load materials from the .mtl file
        mesh.materials.resize(materials.size());
        for (size_t i = 0; i < materials.size(); ++i)
        {
            const auto &mat = materials[i];
            Material material;
            material.name = mat.name;

            material.Ka[0] = mat.ambient[0];
            material.Ka[1] = mat.ambient[1];
            material.Ka[2] = mat.ambient[2];

            material.Kd[0] = mat.diffuse[0];
            material.Kd[1] = mat.diffuse[1];
            material.Kd[2] = mat.diffuse[2];

            material.Ks[0] = mat.specular[0];
            material.Ks[1] = mat.specular[1];
            material.Ks[2] = mat.specular[2];

            material.Ns = mat.shininess;

            material.Ni = mat.ior;

            material.d = mat.dissolve;

            material.illum = mat.illum;

            material.metallic = mat.metallic;
            material.roughness = mat.roughness;

            if (!mat.diffuse_texname.empty())
            {
                material.diffuse_texname = mat.diffuse_texname;
            }
            if (!mat.ambient_texname.empty())
            {
                material.ambient_texname = mat.ambient_texname;
            }
            if (!mat.specular_texname.empty())
            {
                material.specular_texname = mat.specular_texname;
            }
            if (!mat.bump_texname.empty())
            {
                material.bump_texname = mat.bump_texname;
            }
            if (!mat.emissive_texname.empty())
            {
                material.emissive_texname = mat.emissive_texname;
            }

            mesh.materials[i] = material;
        }

        // Load verticies from the .obj file
        for (const auto &shape : shapes)
        {
            size_t index_offset = 0;

            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                int fv = shape.mesh.num_face_vertices[f];
                if (fv != 3)
                { // Should only be three due to triangulation
                    ThrowError("number of verticies per face should always be three!");
                }
                int material_id = shape.mesh.material_ids[f];

                Face face;

                for (size_t v = 0; v < fv; v++)
                {
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    face.vertex_indices[v] = mesh.vertices.size();

                    Vertex vertex;
                    vertex.material_id = material_id;

                    vertex.vertex_cord = vicmil::Coord_XYZ_f(
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]);

                    if (idx.normal_index >= 0)
                    {
                        vertex.norm = vicmil::Normal_XYZ_f(
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]);
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        vertex.tex_cord = vicmil::TexCoord_UV_f(
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]);
                    }

                    mesh.vertices.push_back(vertex);
                }
                mesh.faces.push_back(face);
                index_offset += fv;
            }
        }

        return mesh;
    }

    Mesh load_obj_file(const std::string &obj_file_path, const std::string &base_dir)
    {
        _RawMesh raw_mesh;
        std::string warn, err;

        bool ret = tinyobj::LoadObj(&raw_mesh.attrib, &raw_mesh.shapes, &raw_mesh.materials, &warn, &err, obj_file_path.c_str(), base_dir.c_str(), true, true);

        if (!warn.empty())
        {
            std::cout << "Warning: " << warn << std::endl;
        }
        if (!err.empty())
        {
            std::cerr << "Error: " << err << std::endl;
        }
        if (!ret)
        {
            std::cerr << "Failed to load OBJ file!" << std::endl;
            return {};
        }
        return _load_obj_file(raw_mesh);
    }

    // ============================================================
    //            Custom loading of obj files from memory
    // ============================================================

    class _CustomMaterialFileReader : public tinyobj::MaterialReader
    {
    public:
        _CustomMaterialFileReader(std::map<std::string, std::vector<unsigned char>> *m_file_map_)
            : m_file_map(m_file_map_) {}
        virtual ~_CustomMaterialFileReader() {}
        bool operator()(const std::string &mat_filename,
                        std::vector<tinyobj::material_t> *materials,
                        std::map<std::string, int> *matMap,
                        std::string *warn, std::string *err) override
        {
            // Iterate through all the files in the filemap
            // See if we can find the material file
            for (auto my_file : *m_file_map)
            {
                std::string raw_filename = vicmil::split_string(my_file.first, '/').back();
                if (raw_filename == mat_filename)
                {
                    // Bingo! We found our file
                    // Extract the content and return the result
                    std::string file_contents = std::string((char *)(&my_file.second[0]), my_file.second.size());
                    std::istringstream matIStream(file_contents);

                    LoadMtl(matMap, materials, &matIStream, warn, err);

                    if (!warn->empty())
                    {
                        if (err)
                        {
                            (*err) += *warn;
                        }
                    }

                    return true;
                }
            }

            // Could not locate a file with a matching name
            return false;
        }

    private:
        std::map<std::string, std::vector<unsigned char>> *m_file_map;
    };

    vicmil::Mesh load_obj_file_from_memory(std::map<std::string, std::vector<unsigned char>> file_map)
    {
        // file_map: map<file_path, file_content>
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        _CustomMaterialFileReader mtl_file_reader = _CustomMaterialFileReader(&file_map);

        // Read the contents of an .obj file
        std::string file_contents = "";
        for (auto my_file : file_map)
        {
            std::string extension = vicmil::split_string(my_file.first, '.').back();
            if (extension == "obj")
            {
                file_contents = std::string((char *)(&my_file.second[0]), my_file.second.size());
                break;
            }
        }
        if (file_contents.size() == 0)
        {
            std::cerr << "Failed to parse .obj" << std::endl;
            return vicmil::Mesh();
        }

        // Load the model of the file into a format used by tinyobjloader
        vicmil::_RawMesh raw_mesh;
        std::istringstream inStream(file_contents);
        bool ret = tinyobj::LoadObj(&raw_mesh.attrib, &raw_mesh.shapes, &raw_mesh.materials, &warn, &err, &inStream, &mtl_file_reader, true, true);

        // Check for errors
        if (!warn.empty())
        {
            std::cout << "WARN: " << warn << std::endl;
        }
        if (!err.empty())
        {
            std::cerr << err << std::endl;
        }
        if (!ret)
        {
            std::cerr << "Failed to parse .obj" << std::endl;
            return vicmil::Mesh();
        }

        // Convert the model into another format
        vicmil::Mesh mesh = vicmil::_load_obj_file(raw_mesh);
        return mesh;
    }

    // ============================================================
    //            Translating a mesh to buffers
    // ============================================================
    class MeshDefaultVertexIndexBuffers
    {
    public:
        // Vertex buffer for colors
        std::vector<CoordColor_XYZRGBA_f> color_vertex_buffer = {};
        std::vector<TriangleIndices_i3> color_index_buffer = {};

        // Vertex buffer for texture coordinates, for each image
        std::vector<std::vector<CoordTexCoord_XYZUV_f>> texture_coord_buffer = {};
        std::vector<std::vector<TriangleIndices_i3>> texture_index_buffer = {};
        std::vector<std::string> texture_names = {};

        void from_mesh(const Mesh &mesh)
        {
            Assert(color_index_buffer.size() == 0);
            Assert(texture_coord_buffer.size() == 0);

            std::vector<vicmil::Coord_XYZ_f> vertex_coordinates = mesh.get_vertex_coordinates();
            std::vector<int> vertex_material_indices = mesh.get_vertex_material_indices();
            std::vector<TriangleIndices_i3> vertex_triangle_indicies = mesh.get_vertex_triangle_indicies();
            std::vector<vicmil::TexCoord_UV_f> vertex_texture_coordinates = mesh.get_vertex_texture_coordinates();
            std::vector<std::string> material_texture_names = mesh.get_material_texture_names();
            std::vector<ColorRGBA_f> material_colors = mesh.get_material_colors();

            std::vector<bool> material_has_texture;
            for (int i = 0; i < material_texture_names.size(); i++)
            {
                material_has_texture.push_back(!material_texture_names[i].empty());
            }

            std::vector<std::vector<CoordTexCoord_XYZUV_f>> tmp_texture_coords;
            tmp_texture_coords.resize(material_texture_names.size());

            // Split buffer into color vertices, and texture indices
            std::map<int, int> old_index_to_new_index = {};
            for (int i = 0; i < vertex_coordinates.size(); i++)
            {
                int material_id = vertex_material_indices[i];
                if (!material_has_texture[material_id])
                {
                    color_vertex_buffer.push_back(
                        CoordColor_XYZRGBA_f(
                            vertex_coordinates[i].x,
                            vertex_coordinates[i].y,
                            vertex_coordinates[i].z,
                            material_colors[i].r,
                            material_colors[i].g,
                            material_colors[i].b,
                            material_colors[i].a));
                    old_index_to_new_index[i] = color_vertex_buffer.size() - 1;
                }
                else
                {
                    tmp_texture_coords[material_id].push_back(
                        CoordTexCoord_XYZUV_f(
                            vertex_coordinates[i].x,
                            vertex_coordinates[i].y,
                            vertex_coordinates[i].z,
                            vertex_texture_coordinates[i].u,
                            vertex_texture_coordinates[i].v));
                    old_index_to_new_index[i] = tmp_texture_coords[material_id].size() - 1;
                }
            }

            std::vector<std::vector<TriangleIndices_i3>> tmp_texture_indices;
            tmp_texture_indices.resize(material_texture_names.size());
            for (int i = 0; i < vertex_triangle_indicies.size(); i++)
            {
                int material_id = vertex_material_indices[vertex_triangle_indicies[i].v0];
                if (!material_has_texture[material_id])
                {
                    color_index_buffer.push_back(TriangleIndices_i3(
                        old_index_to_new_index[vertex_triangle_indicies[i].v0],
                        old_index_to_new_index[vertex_triangle_indicies[i].v1],
                        old_index_to_new_index[vertex_triangle_indicies[i].v2]));
                }
                else
                {
                    tmp_texture_indices[material_id].push_back(TriangleIndices_i3(
                        old_index_to_new_index[vertex_triangle_indicies[i].v0],
                        old_index_to_new_index[vertex_triangle_indicies[i].v1],
                        old_index_to_new_index[vertex_triangle_indicies[i].v2]));
                }
            }
            for (int i = 0; i < material_texture_names.size(); i++)
            {
                if (material_has_texture[i])
                {
                    texture_names.push_back(material_texture_names[i]);
                    texture_coord_buffer.push_back(tmp_texture_coords[i]);
                    texture_index_buffer.push_back(tmp_texture_indices[i]);
                }
            }
        }
    };
    class MeshDefaultTextureBuffers
    {
        std::map<std::string, ImageRGBA_UChar> image_buffer;
        void add_mesh_embedded_images(const Mesh &mesh)
        {
            for (auto pair : mesh.embedded_images)
            {
                std::string image_name = pair.first;
                if (image_buffer.count(image_name) == 0)
                {
                    image_buffer[image_name] = pair.second;
                }
            }
        }
        void add_image(std::string image_name, ImageRGBA_UChar image)
        {
            if (image_buffer.count(image_name) == 0)
            {
                image_buffer[image_name] = image;
            }
        }
    };
}