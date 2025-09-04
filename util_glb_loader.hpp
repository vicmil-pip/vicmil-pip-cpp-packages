#pragma once

#include "util_obj_loader.hpp"
#include "util_stb.hpp"
#include "util_glm.hpp"

// Disable stb_image to avoid double definitions
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE

#define TINYGLTF_IMPLEMENTATION
#include "tinyGLTF/tiny_gltf.h"

namespace vicmil
{
    // --- Helper: Read indices ---
    std::vector<unsigned int> read_indices(const tinygltf::Model &model, const tinygltf::Primitive &primitive)
    {
        std::vector<unsigned int> indices;
        if (primitive.indices < 0)
            return indices;

        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &bufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const unsigned char *dataPtr = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;

        for (size_t i = 0; i < indexAccessor.count; ++i)
        {
            unsigned int index = 0;
            switch (indexAccessor.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                index = static_cast<unsigned int>(*(dataPtr + i));
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                index = static_cast<unsigned int>(
                    *reinterpret_cast<const unsigned short *>(dataPtr + i * 2));
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                index = *reinterpret_cast<const unsigned int *>(dataPtr + i * 4);
                break;
            default:
                std::cerr << "[GLB] Unsupported index component type!" << std::endl;
                break;
            }
            indices.push_back(index);
        }
        return indices;
    }

    // --- Helper: Read positions ---
    std::vector<Coord_XYZ_f> read_positions(const tinygltf::Model &model, const tinygltf::Primitive &primitive, bool flip_y)
    {
        std::vector<Coord_XYZ_f> positions;
        auto itPos = primitive.attributes.find("POSITION");
        if (itPos == primitive.attributes.end())
            return positions;

        const tinygltf::Accessor &accessor = model.accessors[itPos->second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const float *buf = reinterpret_cast<const float *>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        positions.resize(accessor.count);
        for (size_t i = 0; i < accessor.count; ++i)
        {
            float x = buf[i * 3 + 0];
            float y = buf[i * 3 + 1];
            float z = buf[i * 3 + 2];

            // Y-up → Z-up
            float vx = x;
            float vy = z;
            float vz = y;
            if (flip_y)
                vy = -vy;

            positions[i] = Coord_XYZ_f(vx, vy, vz);
        }
        return positions;
    }

    // --- Helper: Read normals ---
    std::vector<Normal_XYZ_f> read_normals(const tinygltf::Model &model, const tinygltf::Primitive &primitive, bool flip_y)
    {
        std::vector<Normal_XYZ_f> normals;
        auto itNorm = primitive.attributes.find("NORMAL");
        if (itNorm == primitive.attributes.end())
            return normals;

        const tinygltf::Accessor &accessor = model.accessors[itNorm->second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const float *buf = reinterpret_cast<const float *>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        normals.resize(accessor.count);
        for (size_t i = 0; i < accessor.count; ++i)
        {
            float nx = buf[i * 3 + 0];
            float ny = buf[i * 3 + 1];
            float nz = buf[i * 3 + 2];

            float nnx = nx;
            float nny = nz;
            float nnz = ny;
            if (flip_y)
                nny = -nny;

            normals[i] = Normal_XYZ_f(nnx, nny, nnz);
        }
        return normals;
    }

    // --- Helper: Read texture coordinates ---
    std::vector<TexCoord_UV_f> read_texcoords(const tinygltf::Model &model, const tinygltf::Primitive &primitive)
    {
        std::vector<TexCoord_UV_f> uvs;
        auto itUV = primitive.attributes.find("TEXCOORD_0");
        if (itUV == primitive.attributes.end())
            return uvs;

        const tinygltf::Accessor &accessor = model.accessors[itUV->second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        const float *buf = reinterpret_cast<const float *>(
            buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        uvs.resize(accessor.count);
        for (size_t i = 0; i < accessor.count; ++i)
        {
            uvs[i] = TexCoord_UV_f(buf[i * 2 + 0], buf[i * 2 + 1]);
        }
        return uvs;
    }

    // --- Main: Convert meshes ---
    void convert_meshes(const tinygltf::Model &model, Mesh &mesh, bool flip_y = true)
    {
        for (const auto &gltfMesh : model.meshes)
        {
            for (const auto &primitive : gltfMesh.primitives)
            {
                auto indices = read_indices(model, primitive);
                auto positions = read_positions(model, primitive, flip_y);
                auto normals = read_normals(model, primitive, flip_y);
                auto uvs = read_texcoords(model, primitive);

                // Expand vertices per index to match the triangles
                size_t baseVertex = mesh.vertices.size();
                for (size_t i = 0; i < indices.size(); ++i)
                {
                    unsigned int idx = indices[i];
                    Vertex v;
                    v.vertex_cord = (idx < positions.size()) ? positions[idx] : Coord_XYZ_f();
                    v.norm = (idx < normals.size()) ? normals[idx] : Normal_XYZ_f();
                    v.tex_cord = (idx < uvs.size()) ? uvs[idx] : TexCoord_UV_f();
                    v.material_id = primitive.material;

                    mesh.vertices.push_back(v);
                }

                // Create faces
                for (size_t i = 0; i < indices.size(); i += 3)
                {
                    Face f;
                    f.vertex_indices[0] = static_cast<int>(baseVertex + i + 0);
                    f.vertex_indices[1] = static_cast<int>(baseVertex + i + 1);
                    f.vertex_indices[2] = static_cast<int>(baseVertex + i + 2);
                    mesh.faces.push_back(f);
                }
            }
        }
    }

    // ----------------------------------------
    // Decode raw image bytes to ImageRGBA_UChar
    // ----------------------------------------
    bool decode_glb_image(const std::vector<unsigned char> &data, ImageRGBA_UChar &out_image)
    {
        int w, h, comp;
        unsigned char *img = stbi_load_from_memory(data.data(), data.size(), &w, &h, &comp, 4);
        if (!img)
            return false;

        out_image.resize(w, h);
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                int idx = 4 * (y * w + x);
                out_image.pixels[y * w + x] = ColorRGBA_UChar(
                    img[idx + 0], img[idx + 1], img[idx + 2], img[idx + 3]);
            }
        }
        stbi_image_free(img);
        return true;
    }

    // ----------------------------------------
    // GLB image loader callback
    // ----------------------------------------
    bool glb_image_loader_callback(tinygltf::Image *image, const unsigned char *bytes, int size, void *user_data)
    {
        if (!image || !bytes || size == 0)
            return false;

        Mesh *mesh = reinterpret_cast<Mesh *>(user_data);

        std::vector<unsigned char> raw_bytes(bytes, bytes + size);
        ImageRGBA_UChar img;
        if (!decode_glb_image(raw_bytes, img))
            return false;

        std::string name = !image->uri.empty() ? image->uri : "embedded_" + std::to_string(mesh->embedded_images.size());

        mesh->embedded_images[name] = std::move(img);
        return true;
    }

    // ----------------------------------------
    // Convert materials from TinyGLTF to Mesh
    // ----------------------------------------
    void convert_materials(const tinygltf::Model &model, Mesh &mesh)
    {
        mesh.materials.resize(model.materials.size());

        for (size_t i = 0; i < model.materials.size(); ++i)
        {
            const auto &mat = model.materials[i];
            Material material;
            material.name = mat.name;

            // --- Base Color Factor (diffuse color) ---
            if (mat.values.find("baseColorFactor") != mat.values.end())
            {
                const auto &factor = mat.values.at("baseColorFactor").ColorFactor();
                material.Kd[0] = factor[0];
                material.Kd[1] = factor[1];
                material.Kd[2] = factor[2];
                if (factor.size() > 3)
                    material.d = factor[3]; // alpha
            }

            // --- Metallic / Roughness ---
            if (mat.additionalValues.find("metallicFactor") != mat.additionalValues.end())
                material.metallic = static_cast<float>(mat.additionalValues.at("metallicFactor").Factor());
            if (mat.additionalValues.find("roughnessFactor") != mat.additionalValues.end())
                material.roughness = static_cast<float>(mat.additionalValues.at("roughnessFactor").Factor());

            // --- Diffuse Texture (baseColorTexture) ---
            if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
            {
                int texIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
                const tinygltf::Texture &tex = model.textures[texIndex];
                if (tex.source >= 0)
                {
                    const tinygltf::Image &image = model.images[tex.source];
                    material.diffuse_texname = image.uri; // external file path (if available)
                    if (material.diffuse_texname.empty())
                        material.diffuse_texname = "embedded_" + std::to_string(tex.source); // fallback for embedded textures
                }
            }

            // --- Normal Map (bump) ---
            if (mat.normalTexture.index >= 0)
            {
                int texIndex = mat.normalTexture.index;
                const tinygltf::Texture &tex = model.textures[texIndex];
                if (tex.source >= 0)
                {
                    const tinygltf::Image &image = model.images[tex.source];
                    material.bump_texname = image.uri;
                    if (material.bump_texname.empty())
                        material.bump_texname = "embedded_" + std::to_string(tex.source);
                }
            }

            // --- Emissive Texture ---
            if (mat.emissiveTexture.index >= 0)
            {
                int texIndex = mat.emissiveTexture.index;
                const tinygltf::Texture &tex = model.textures[texIndex];
                if (tex.source >= 0)
                {
                    const tinygltf::Image &image = model.images[tex.source];
                    material.emissive_texname = image.uri;
                    if (material.emissive_texname.empty())
                        material.emissive_texname = "embedded_" + std::to_string(tex.source);
                }
            }

            // --- Specular/Glossiness (if extensions are used) ---
            // Note: In glTF 2.0 core PBR, there's no direct specular map, but
            // some extensions (like KHR_materials_pbrSpecularGlossiness) may add it.

            mesh.materials[i] = material;
        }
    }

    std::vector<unsigned char> read_file_bytes(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            // Failed to open the file -> return empty vector
            return {};
        }

        std::ifstream::pos_type fileSize = file.tellg();
        std::vector<unsigned char> buffer(fileSize);

        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(buffer.data()), fileSize);

        return buffer;
    }

    // ----------------------------------------
    // Main GLB loader
    // ----------------------------------------

    class GLBLoader
    {
    public:
        tinygltf::Model model;
        std::map<std::string, ImageRGBA_UChar> images;

        // ----------------------------------------
        // Decode raw image bytes to ImageRGBA_UChar
        // ----------------------------------------
        static bool decode_glb_image(const std::vector<unsigned char> &data, ImageRGBA_UChar &out_image)
        {
            int w, h, comp;
            unsigned char *img = stbi_load_from_memory(data.data(), data.size(), &w, &h, &comp, 4);
            if (!img)
                return false;

            out_image.resize(w, h);
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    int idx = 4 * (y * w + x);
                    out_image.pixels[y * w + x] = ColorRGBA_UChar(
                        img[idx + 0], img[idx + 1], img[idx + 2], img[idx + 3]);
                }
            }
            stbi_image_free(img);
            return true;
        }

        // ----------------------------------------
        // GLB image loader callback
        // ----------------------------------------
        static bool glb_image_loader_callback(tinygltf::Image *image, const unsigned char *bytes, int size, void *user_data)
        {
            if (!image || !bytes || size == 0)
                return false;

            std::map<std::string, ImageRGBA_UChar> *embedded_images = reinterpret_cast<std::map<std::string, ImageRGBA_UChar> *>(user_data);

            std::vector<unsigned char> raw_bytes(bytes, bytes + size);
            ImageRGBA_UChar img;
            if (!decode_glb_image(raw_bytes, img))
                return false;

            std::string name = !image->uri.empty() ? image->uri : "embedded_" + std::to_string(embedded_images->size());

            (*embedded_images)[name] = std::move(img);
            return true;
        }

        static void glb_magic_test(const std::vector<unsigned char> &data)
        {
            // Test if file is correct type by looking at the first few bytes
            if (data.size() < 4)
            {
                throw std::runtime_error("File too small to be a valid GLB.");
            }

            const char *magic = reinterpret_cast<const char *>(data.data());
            std::cout << "First 4 bytes (magic): ";
            for (int i = 0; i < 4; ++i)
            {
                std::cout << std::hex << static_cast<int>(static_cast<unsigned char>(magic[i])) << " ";
            }
            std::cout << std::dec << std::endl;

            if (magic[0] != 'g' || magic[1] != 'l' || magic[2] != 'T' || magic[3] != 'F')
            {
                throw std::runtime_error("Invalid GLB file: magic number mismatch.");
            }
        }

        void load_from_memory(const std::vector<unsigned char> &data)
        {
            // --- Magic test ---
            glb_magic_test(data);

            tinygltf::TinyGLTF loader;
            std::string err, warn;

            // Set image loader callback
            loader.SetImageLoader(
                [](tinygltf::Image *image, const int /*image_idx*/,
                   std::string *err, std::string *warn,
                   int width, int height,
                   const unsigned char *bytes, int size,
                   void *user_data) -> bool
                {
                    return glb_image_loader_callback(image, bytes, size, user_data);
                },
                &images);

            bool ret = loader.LoadBinaryFromMemory(&model, &err, &warn,
                                                   data.data(), static_cast<int>(data.size()), "");

            if (!warn.empty())
                std::cout << "[GLB] Warning: " << warn << std::endl;
            if (!err.empty())
                std::cerr << "[GLB] Error: " << err << std::endl;
            if (!ret)
                std::cerr << "[GLB] Failed to load GLB from memory" << std::endl;
        }
        void load_from_file(const std::string &glb_file_path)
        {
            std::vector<unsigned char> file_content = read_file_bytes(glb_file_path);

            std::cout << "file size: " << file_content.size() << std::endl;

            load_from_memory(file_content);
        }

        std::string get_metadata() const
        {
            std::string metadata;

            // Model info
            metadata += "Asset Version: " + model.asset.version + "\n";
            metadata += "Asset Generator: " + model.asset.generator + "\n";

            // Scene info
            metadata += "Scene Count: " + std::to_string(model.scenes.size()) + "\n";
            metadata += "Node Count: " + std::to_string(model.nodes.size()) + "\n";
            metadata += "Mesh Count: " + std::to_string(model.meshes.size()) + "\n";
            metadata += "Material Count: " + std::to_string(model.materials.size()) + "\n";
            metadata += "Image Count: " + std::to_string(model.images.size()) + "\n";

            // Image info
            for (const auto pair : images)
            {
                std::string name = pair.first;
                const ImageRGBA_UChar &image = pair.second;
                metadata += "Image: " + name + "\n";
                metadata += "  Width: " + std::to_string(image.w) + "\n";
                metadata += "  Height: " + std::to_string(image.h) + "\n";
            }

            return metadata;
        }
    };

    void PrintBuffer(const tinygltf::Buffer &buffer)
    {
        std::cout << "  Buffer name: " << buffer.name
                  << ", size: " << buffer.data.size() << " bytes\n";
    }

    void PrintBufferView(const tinygltf::BufferView &bv)
    {
        std::cout << "  BufferView buffer: " << bv.buffer
                  << ", byteOffset: " << bv.byteOffset
                  << ", byteLength: " << bv.byteLength
                  << ", target: " << bv.target << "\n";
    }

    void PrintAccessor(const tinygltf::Accessor &acc)
    {
        std::cout << "  Accessor name: " << acc.name << "\n";
        std::cout << "  BufferView index: " << acc.bufferView << "\n";
        std::cout << "  Byte offset: " << acc.byteOffset << "\n";

        // Print the component type of each element in the accessor
        // Common values:
        //   TINYGLTF_COMPONENT_TYPE_BYTE
        //   TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE
        //   TINYGLTF_COMPONENT_TYPE_SHORT
        //   TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
        //   TINYGLTF_COMPONENT_TYPE_INT
        //   TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
        //   TINYGLTF_COMPONENT_TYPE_FLOAT
        std::cout << "  Component type: " << acc.componentType << "\n";

        // Examples: "SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"
        std::cout << "  Type: " << acc.type << "\n";
        std::cout << "  Count: " << acc.count << "\n";

        // Optionally, you could also print min/max values if needed:
        if (!acc.minValues.empty())
        {
            std::cout << "  Min values: ";
            for (auto v : acc.minValues)
                std::cout << v << " ";
            std::cout << "\n";
        }

        if (!acc.maxValues.empty())
        {
            std::cout << "  Max values: ";
            for (auto v : acc.maxValues)
                std::cout << v << " ";
            std::cout << "\n";
        }
    }

    void PrintImage(const tinygltf::Image &image)
    {
        std::cout << "  Image name: " << image.name
                  << ", width: " << image.width
                  << ", height: " << image.height
                  << ", component: " << image.component
                  << ", bits: " << image.bits
                  << ", uri: " << image.uri
                  << ", bufferView: " << image.bufferView
                  << ", size of image data: " << image.image.size() << "\n";
    }

    void PrintMaterial(const tinygltf::Material &mat)
    {
        std::cout << "  Material name: " << mat.name << "\n";
        std::cout << "    DoubleSided: " << mat.doubleSided << "\n";
        std::cout << "    AlphaMode: " << mat.alphaMode << "\n";
        std::cout << "    BaseColorFactor: ";
        for (auto v : mat.pbrMetallicRoughness.baseColorFactor)
            std::cout << v << " ";
        std::cout << "\n";
    }

    void PrintNode(const tinygltf::Node &node)
    {
        std::cout << "  Node name: " << node.name
                  << ", mesh: " << node.mesh
                  << ", camera: " << node.camera
                  << ", skin: " << node.skin << "\n";
        if (!node.translation.empty())
        {
            std::cout << "    Translation: ";
            for (auto v : node.translation)
                std::cout << v << " ";
            std::cout << "\n";
        }
        if (!node.rotation.empty())
        {
            std::cout << "    Rotation: ";
            for (auto v : node.rotation)
                std::cout << v << " ";
            std::cout << "\n";
        }
        if (!node.scale.empty())
        {
            std::cout << "    Scale: ";
            for (auto v : node.scale)
                std::cout << v << " ";
            std::cout << "\n";
        }
        if (!node.children.empty())
        {
            std::cout << "    Children: ";
            for (auto c : node.children)
                std::cout << c << " ";
            std::cout << "\n";
        }
    }

    void PrintScene(const tinygltf::Scene &scene)
    {
        std::cout << "  Scene name: " << scene.name << ", nodes: ";
        for (auto n : scene.nodes)
            std::cout << n << " ";
        std::cout << "\n";
    }

    template <typename T>
    void PrintData(const std::string &name, const T *data, size_t count, size_t stride)
    {
        std::cout << "    " << name << " data:\n";
        for (size_t i = 0; i < count; ++i)
        {
            std::cout << "      [" << i << "]: ";
            for (size_t j = 0; j < stride; ++j)
            {
                std::cout << data[i * stride + j] << " ";
            }
            std::cout << "\n";
        }
    }

    std::string AccessorTypeToString(int type)
    {
        switch (type)
        {
        case TINYGLTF_TYPE_SCALAR:
            return "SCALAR";
        case TINYGLTF_TYPE_VEC2:
            return "VEC2";
        case TINYGLTF_TYPE_VEC3:
            return "VEC3";
        case TINYGLTF_TYPE_VEC4:
            return "VEC4";
        case TINYGLTF_TYPE_MAT2:
            return "MAT2";
        case TINYGLTF_TYPE_MAT3:
            return "MAT3";
        case TINYGLTF_TYPE_MAT4:
            return "MAT4";
        default:
            return "UNKNOWN";
        }
    }

    void PrintAccessorData(const tinygltf::Model &model, const tinygltf::Accessor &acc)
    {
        if (acc.bufferView < 0)
            return;

        const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bv.buffer];
        const unsigned char *dataPtr = buffer.data.data() + bv.byteOffset + acc.byteOffset;

        size_t componentSize = 0;
        switch (acc.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            componentSize = 1;
            break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            componentSize = 2;
            break;
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            componentSize = 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            componentSize = 4;
            break;
        default:
            std::cout << "Unknown component type\n";
            return;
        }

        size_t numComponents = 0;
        std::string acc_type = AccessorTypeToString(acc.type);
        if (acc_type == "SCALAR")
            numComponents = 1;
        else if (acc_type == "VEC2")
            numComponents = 2;
        else if (acc_type == "VEC3")
            numComponents = 3;
        else if (acc_type == "VEC4")
            numComponents = 4;
        else if (acc_type == "MAT2")
            numComponents = 4;
        else if (acc_type == "MAT3")
            numComponents = 9;
        else if (acc_type == "MAT4")
            numComponents = 16;

        size_t stride = bv.byteStride > 0 ? bv.byteStride / componentSize : numComponents;

        std::cout << "    Accessor data (" << acc.name << "):\n";

        switch (acc.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
        {
            const float *fdata = reinterpret_cast<const float *>(dataPtr);
            PrintData("float", fdata, acc.count, numComponents);
            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        {
            const uint16_t *sdata = reinterpret_cast<const uint16_t *>(dataPtr);
            PrintData("ushort", sdata, acc.count, numComponents);
            break;
        }
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        {
            const uint32_t *idata = reinterpret_cast<const uint32_t *>(dataPtr);
            PrintData("uint", idata, acc.count, numComponents);
            break;
        }
        default:
            std::cout << "      Unsupported component type for printing\n";
            break;
        }
    }

    // Helper to read a single index value from an accessor
    int GetIndexValue(const tinygltf::Model &model, const tinygltf::Accessor &accessor, size_t idx)
    {
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        const unsigned char *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return static_cast<int>(reinterpret_cast<const uint8_t *>(dataPtr)[idx]);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return static_cast<int>(reinterpret_cast<const uint16_t *>(dataPtr)[idx]);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return static_cast<int>(reinterpret_cast<const uint32_t *>(dataPtr)[idx]);
        default:
            std::cerr << "Unsupported index component type: " << accessor.componentType << "\n";
            return -1;
        }
    }

    // Helper to print a single element from an attribute accessor
    void PrintAccessorElement(const tinygltf::Model &model, const tinygltf::Accessor &accessor, size_t idx)
    {
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        const unsigned char *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

        size_t numComponents = tinygltf::GetNumComponentsInType(accessor.type);

        std::cout << "          [";
        for (size_t c = 0; c < numComponents; ++c)
        {
            switch (accessor.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
                std::cout << static_cast<int>(reinterpret_cast<const int8_t *>(dataPtr)[idx * numComponents + c]);
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                std::cout << static_cast<int>(reinterpret_cast<const uint8_t *>(dataPtr)[idx * numComponents + c]);
                break;
            case TINYGLTF_COMPONENT_TYPE_SHORT:
                std::cout << reinterpret_cast<const int16_t *>(dataPtr)[idx * numComponents + c];
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                std::cout << reinterpret_cast<const uint16_t *>(dataPtr)[idx * numComponents + c];
                break;
            case TINYGLTF_COMPONENT_TYPE_INT:
                std::cout << reinterpret_cast<const int32_t *>(dataPtr)[idx * numComponents + c];
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                std::cout << reinterpret_cast<const uint32_t *>(dataPtr)[idx * numComponents + c];
                break;
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                std::cout << reinterpret_cast<const float *>(dataPtr)[idx * numComponents + c];
                break;
            case TINYGLTF_COMPONENT_TYPE_DOUBLE:
                std::cout << reinterpret_cast<const double *>(dataPtr)[idx * numComponents + c];
                break;
            default:
                std::cout << "?";
                break;
            }
            if (c + 1 < numComponents)
                std::cout << ", ";
        }
        std::cout << "]\n";
    }

    void PrintMeshData(const tinygltf::Model &model, const tinygltf::Mesh &mesh)
    {
        std::cout << "  Mesh name: " << mesh.name << "\n";

        for (size_t i = 0; i < mesh.primitives.size(); ++i)
        {
            const auto &prim = mesh.primitives[i];
            std::cout << "    Primitive " << i << ", mode: " << prim.mode << "\n";

            // Print raw attribute data
            for (const auto &attr : prim.attributes)
            {
                std::cout << "      Attribute: " << attr.first << "\n";
                PrintAccessorData(model, model.accessors[attr.second]);
            }

            // Print indices and optionally map attribute data via indices
            if (prim.indices >= 0)
            {
                const tinygltf::Accessor &indexAccessor = model.accessors[prim.indices];
                std::cout << "      Indices:\n";
                PrintAccessorData(model, indexAccessor);

                // Optionally: Print attribute data per index
                std::cout << "      Indexed attribute data:\n";
                for (const auto &attr : prim.attributes)
                {
                    const tinygltf::Accessor &attrAccessor = model.accessors[attr.second];
                    std::cout << "        Attribute " << attr.first << " values per index:\n";

                    for (size_t idx = 0; idx < indexAccessor.count; ++idx)
                    {
                        int indexValue = GetIndexValue(model, indexAccessor, idx); // helper to read index
                        PrintAccessorElement(model, attrAccessor, indexValue);     // helper to print one element
                    }
                }
            }
        }
    }

    // Recursive function to compute and print world matrix of a node
    void PrintNodeWorldMatrix(const tinygltf::Model &model, int nodeIndex, const glm::mat4 &parentMatrix = glm::mat4(1.0f))
    {
        const tinygltf::Node &node = model.nodes[nodeIndex];

        glm::mat4 localMatrix(1.0f);

        if (!node.matrix.empty())
        {
            localMatrix = vicmil::make_mat4(node.matrix.data());
        }
        else
        {
            glm::vec3 translation(0.0f);
            glm::quat rotation(1.0, 0.0, 0.0, 0.0);
            glm::vec3 scale(1.0f);

            if (!node.translation.empty())
                translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
            if (!node.rotation.empty())
                rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]); // w,x,y,z
            if (!node.scale.empty())
                scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

            localMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
        }

        glm::mat4 worldMatrix = parentMatrix * localMatrix;

        std::cout << "Node " << nodeIndex << " (" << node.name << ") world matrix:\n";
        for (int i = 0; i < 4; ++i)
        {
            std::cout << "  ";
            for (int j = 0; j < 4; ++j)
                std::cout << worldMatrix[j][i] << " "; // column-major
            std::cout << "\n";
        }

        for (int child : node.children)
            PrintNodeWorldMatrix(model, child, worldMatrix);
    }

    // Helper to start recursion from root nodes of a scene
    void PrintSceneNodeWorldMatrices(const tinygltf::Model &model, int sceneIndex = 0)
    {
        if (sceneIndex < 0 || sceneIndex >= model.scenes.size())
        {
            std::cerr << "Invalid scene index: " << sceneIndex << "\n";
            return;
        }

        const tinygltf::Scene &scene = model.scenes[sceneIndex];
        std::cout << "=== Scene " << sceneIndex << " Node World Matrices ===\n";

        for (int rootNodeIndex : scene.nodes)
        {
            PrintNodeWorldMatrix(model, rootNodeIndex, glm::mat4(1.0f));
        }
    }

    void PrintModel(const tinygltf::Model &model)
    {
        std::cout << "=== Buffers ===\n";
        for (const auto &buffer : model.buffers)
            PrintBuffer(buffer);

        std::cout << "=== BufferViews ===\n";
        for (const auto &bv : model.bufferViews)
            PrintBufferView(bv);

        std::cout << "=== Accessors ===\n";
        for (const auto &acc : model.accessors)
            PrintAccessor(acc);

        std::cout << "=== Images ===\n";
        for (const auto &img : model.images)
            PrintImage(img);

        std::cout << "=== Materials ===\n";
        for (const auto &mat : model.materials)
            PrintMaterial(mat);

        std::cout << "=== Meshes ===\n";
        for (const auto &mesh : model.meshes)
            PrintMeshData(model, mesh);

        std::cout << "=== Nodes ===\n";
        for (const auto &node : model.nodes)
            PrintNode(node);

        std::cout << "=== Scenes ===\n";
        for (const auto &scene : model.scenes)
            PrintScene(scene);

        std::cout << "=== Node World Matrices ===\n";
        PrintSceneNodeWorldMatrices(model, model.defaultScene);

        std::cout << "Default scene index: " << model.defaultScene << "\n";
    }
}
