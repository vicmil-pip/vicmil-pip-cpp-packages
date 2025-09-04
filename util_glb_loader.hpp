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
        struct EmbeddedImages
        {
            std::vector<ImageRGBA_UChar> images;
            std::vector<std::string> image_names;
            std::map<std::string, int> index_name_to_index;
        };
        EmbeddedImages embedded_images;
        tinygltf::Model model;

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

            EmbeddedImages *embedded_images = reinterpret_cast<EmbeddedImages *>(user_data);

            std::vector<unsigned char> raw_bytes(bytes, bytes + size);
            ImageRGBA_UChar img;
            if (!decode_glb_image(raw_bytes, img))
                return false;

            std::string name = !image->uri.empty() ? image->uri : "embedded_" + std::to_string(embedded_images->images.size());

            if (vicmil::get_file_extension(name) != "png")
            {
                name += ".png";
            }

            embedded_images->images.push_back(img);
            embedded_images->image_names.push_back(name);
            embedded_images->index_name_to_index[name] = embedded_images->image_names.size() - 1;
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
                &embedded_images);

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
            for (int i = 0; i < embedded_images.images.size(); i++)
            {
                std::string name = embedded_images.image_names[i];
                const ImageRGBA_UChar &image = embedded_images.images[i];
                metadata += "Image: " + name + "\n";
                metadata += "  Width: " + std::to_string(image.w) + "\n";
                metadata += "  Height: " + std::to_string(image.h) + "\n";
            }

            return metadata;
        }
    };

    //
    // Print functions to make sense of everything
    //
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

    /*void PrintImage(const tinygltf::Image &image)
    {
        std::cout << "  Image name: " << image.name
                  << ", width: " << image.width
                  << ", height: " << image.height
                  << ", component: " << image.component
                  << ", bits: " << image.bits
                  << ", uri: " << image.uri
                  << ", bufferView: " << image.bufferView
                  << ", size of image data: " << image.image.size() << "\n";
    }*/

    void PrintMaterial(const tinygltf::Material &mat)
    {
        std::cout << "Material name: " << mat.name << "\n";
        std::cout << "  DoubleSided: " << mat.doubleSided << "\n";
        std::cout << "  AlphaMode: " << mat.alphaMode << "\n";

        // Base color factor
        std::cout << "  BaseColorFactor: ";
        for (auto v : mat.pbrMetallicRoughness.baseColorFactor)
            std::cout << v << " ";
        std::cout << "\n";

        // Metallic and roughness factors
        std::cout << "  MetallicFactor: " << mat.pbrMetallicRoughness.metallicFactor << "\n";
        std::cout << "  RoughnessFactor: " << mat.pbrMetallicRoughness.roughnessFactor << "\n";

        // Helper lambda to print texture index
        auto printTextureIndex = [&](int texIndex, const std::string &slotName)
        {
            if (texIndex >= 0)
                std::cout << "  " << slotName << " index: " << texIndex << "\n";
            else
                std::cout << "  " << slotName << " index: <none>\n";
        };

        // Print all main textures by index
        printTextureIndex(mat.pbrMetallicRoughness.baseColorTexture.index, "BaseColorTexture");
        printTextureIndex(mat.pbrMetallicRoughness.metallicRoughnessTexture.index, "MetallicRoughnessTexture");
        printTextureIndex(mat.normalTexture.index, "NormalTexture");
        printTextureIndex(mat.occlusionTexture.index, "OcclusionTexture");
        printTextureIndex(mat.emissiveTexture.index, "EmissiveTexture");

        // Emissive factor
        std::cout << "  EmissiveFactor: ";
        for (auto v : mat.emissiveFactor)
            std::cout << v << " ";
        std::cout << "\n";
    }

    /*void PrintNode(const tinygltf::Node &node)
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
    }*/

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

    // Note! Each mesh data belongs to one or more nodes, which can be used to get the world position of the mesh
    struct MeshData
    {
        std::string name = "";
        int mesh_id = -1;

        struct PrimitiveData
        {
            std::vector<glm::vec3> normals;        // Note! Specified in local coordinates, not world coordinates
            std::vector<glm::vec3> positions;      // Note! Specified in local coordinates, not world coordinates
            std::vector<glm::vec2> uv_coordinates; // coordinates on texture, Note! All tex coords are inverted in the v axis
            std::vector<glm::uvec3> triangles;     // each uvec3 = one face
            int material_id = -1;
        };

        std::vector<PrimitiveData> primitives;

        std::string to_string(bool print_buffers = false) const
        {
            std::ostringstream oss;
            oss << "Mesh ID: " << mesh_id << "\n";

            oss << "Mesh name: " << name << "\n";

            for (int primitive_idx = 0; primitive_idx < primitives.size(); primitive_idx++)
            {
                const PrimitiveData &primitive = primitives[primitive_idx];

                oss << "Primitive: " << primitive_idx << "\n";

                oss << "Material: " << primitive.material_id << "\n";

                oss << "Positions (" << primitive.positions.size() << "):\n";
                if (print_buffers)
                {
                    for (size_t i = 0; i < primitive.positions.size(); ++i)
                    {
                        const auto &v = primitive.positions[i];
                        oss << "  [" << i << "]: (" << v.x << ", " << v.y << ", " << v.z << ")\n";
                    }
                }

                oss << "Normals (" << primitive.normals.size() << "):\n";
                if (print_buffers)
                {
                    for (size_t i = 0; i < primitive.normals.size(); ++i)
                    {
                        const auto &v = primitive.normals[i];
                        oss << "  [" << i << "]: (" << v.x << ", " << v.y << ", " << v.z << ")\n";
                    }
                }

                oss << "UVs (" << primitive.uv_coordinates.size() << "):\n";
                if (print_buffers)
                {
                    for (size_t i = 0; i < primitive.uv_coordinates.size(); ++i)
                    {
                        const auto &v = primitive.uv_coordinates[i];
                        oss << "  [" << i << "]: (" << v.x << ", " << v.y << ")\n";
                    }
                }

                oss << "Triangles (" << primitive.triangles.size() << "):\n";
                if (print_buffers)
                {
                    for (size_t i = 0; i < primitive.triangles.size(); ++i)
                    {
                        const auto &t = primitive.triangles[i];
                        oss << "  [" << i << "]: (" << t.x << ", " << t.y << ", " << t.z << ")\n";
                    }
                }
            }

            return oss.str();
        }

        // ---------- helpers ----------
        static std::vector<uint32_t> read_indices(const tinygltf::Model &model,
                                                  const tinygltf::Accessor &accessor)
        {
            const auto &bufferView = model.bufferViews[accessor.bufferView];
            const auto &buffer = model.buffers[bufferView.buffer];

            size_t stride = accessor.ByteStride(bufferView);
            const unsigned char *dataPtr =
                buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

            std::vector<uint32_t> indices;
            indices.reserve(accessor.count);

            for (size_t i = 0; i < accessor.count; ++i)
            {
                const unsigned char *elem = dataPtr + i * stride;
                uint32_t idx = 0;

                switch (accessor.componentType)
                {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    idx = *reinterpret_cast<const uint8_t *>(elem);
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    idx = *reinterpret_cast<const uint16_t *>(elem);
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    idx = *reinterpret_cast<const uint32_t *>(elem);
                    break;
                default:
                    assert(false && "Unsupported index type");
                }
                indices.push_back(idx);
            }

            return indices;
        }

        static std::vector<glm::vec3> read_vec3(const tinygltf::Model &model,
                                                const tinygltf::Accessor &accessor)
        {
            const auto &bufferView = model.bufferViews[accessor.bufferView];
            const auto &buffer = model.buffers[bufferView.buffer];

            size_t stride = accessor.ByteStride(bufferView);
            const unsigned char *dataPtr =
                buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

            std::vector<glm::vec3> values;
            values.reserve(accessor.count);

            for (size_t i = 0; i < accessor.count; ++i)
            {
                const float *elem = reinterpret_cast<const float *>(dataPtr + i * stride);
                values.emplace_back(elem[0], elem[1], elem[2]);
            }

            return values;
        }

        static std::vector<glm::vec2> read_vec2(const tinygltf::Model &model,
                                                const tinygltf::Accessor &accessor)
        {
            const auto &bufferView = model.bufferViews[accessor.bufferView];
            const auto &buffer = model.buffers[bufferView.buffer];

            size_t stride = accessor.ByteStride(bufferView);
            const unsigned char *dataPtr =
                buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

            std::vector<glm::vec2> values;
            values.reserve(accessor.count);

            for (size_t i = 0; i < accessor.count; ++i)
            {
                const float *elem = reinterpret_cast<const float *>(dataPtr + i * stride);
                values.emplace_back(elem[0], elem[1]);
            }

            return values;
        }

        // ---------- primitive to triangles ----------
        static void convert_to_triangles(std::vector<glm::uvec3> &triangles,
                                         const std::vector<uint32_t> &indices,
                                         int mode)
        {
            if (mode == TINYGLTF_MODE_TRIANGLES)
            {
                assert(indices.size() % 3 == 0);
                for (size_t i = 0; i < indices.size(); i += 3)
                {
                    triangles.emplace_back(indices[i], indices[i + 1], indices[i + 2]);
                }
            }
            else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP)
            {
                if (indices.size() < 3)
                    return;
                for (size_t i = 0; i + 2 < indices.size(); ++i)
                {
                    if (i % 2 == 0)
                        triangles.emplace_back(indices[i], indices[i + 1], indices[i + 2]);
                    else
                        triangles.emplace_back(indices[i + 1], indices[i], indices[i + 2]);
                }
            }
            else if (mode == TINYGLTF_MODE_TRIANGLE_FAN)
            {
                if (indices.size() < 3)
                    return;
                uint32_t first = indices[0];
                for (size_t i = 1; i + 1 < indices.size(); ++i)
                {
                    triangles.emplace_back(first, indices[i], indices[i + 1]);
                }
            }
            else
            {
                // Ignore other modes (LINES, POINTS) for now
            }
        }

        // ---------- main loader ----------
        void load_model_mesh(const tinygltf::Model &model, int mesh_id_)
        {
            primitives.clear();
            mesh_id = mesh_id_;

            const tinygltf::Mesh &mesh = model.meshes[mesh_id];

            name = mesh.name;

            primitives.resize(mesh.primitives.size());

            for (int primitive_idx = 0; primitive_idx < mesh.primitives.size(); primitive_idx++)
            {
                const tinygltf::Primitive &prim = mesh.primitives[primitive_idx];
                PrimitiveData &new_primitive = primitives[primitive_idx];

                new_primitive.material_id = prim.material;

                // ---- indices -> triangles ----
                if (prim.indices >= 0)
                {
                    const auto &indexAccessor = model.accessors[prim.indices];
                    auto flat_indices = read_indices(model, indexAccessor);
                    convert_to_triangles(new_primitive.triangles, flat_indices, prim.mode);
                }

                // ---- attributes ----
                for (const auto &attr : prim.attributes)
                {
                    const auto &accessor = model.accessors[attr.second];

                    int vertex_count = 0;

                    if (attr.first == "POSITION")
                    {
                        auto vals = read_vec3(model, accessor);
                        new_primitive.positions.insert(new_primitive.positions.end(), vals.begin(), vals.end());
                    }
                    else if (attr.first == "NORMAL")
                    {
                        auto vals = read_vec3(model, accessor);
                        new_primitive.normals.insert(new_primitive.normals.end(), vals.begin(), vals.end());
                    }
                    else if (attr.first == "TEXCOORD_0")
                    {
                        auto vals = read_vec2(model, accessor);
                        new_primitive.uv_coordinates.insert(new_primitive.uv_coordinates.end(), vals.begin(), vals.end());
                    }
                    else
                    {
                        PrintExpr(attr.first);
                    }
                }
            }
        }
    };

    struct NodeData
    {
        glm::mat4 local_matrix = glm::mat4(1.0f);
        std::string node_name = "";
        int mesh_id = -1;
        int node_id = -1;

        // Needs to be populated during a second pass
        std::vector<int> path = {}; // scene_id / <parent path> / node_id
        glm::mat4 world_matrix = glm::mat4(0.0f);

        int get_scene_id() const
        {
            return path.front();
        }
        bool has_mesh() const
        {
            return mesh_id != -1;
        }
        NodeData() {}

        std::string to_string() const
        {
            std::ostringstream oss;

            // Path
            oss << "Path: [";
            for (size_t i = 0; i < path.size(); ++i)
            {
                oss << path[i];
                if (i + 1 < path.size())
                    oss << ", ";
            }
            oss << "]\n";

            // Node name
            oss << "Node name: " << node_name << "\n";

            oss << "Mesh: " << mesh_id << "\n";

            // Helper lambda for printing matrices
            auto mat_to_string = [](const glm::mat4 &mat)
            {
                std::ostringstream mss;
                mss << "[\n";
                for (int row = 0; row < 4; ++row)
                {
                    mss << "  [";
                    for (int col = 0; col < 4; ++col)
                    {
                        mss << mat[col][row]; // glm is column-major
                        if (col < 3)
                            mss << ", ";
                    }
                    mss << "]";
                    if (row < 3)
                        mss << ",";
                    mss << "\n";
                }
                mss << "]";
                return mss.str();
            };

            oss << "Local Matrix:\n"
                << mat_to_string(local_matrix) << "\n";
            oss << "World Matrix:\n"
                << mat_to_string(world_matrix) << "\n";

            return oss.str();
        }

        void load_node_data(const tinygltf::Model &model, int node_id_)
        {
            node_id = node_id_;

            const tinygltf::Node &node = model.nodes[node_id];
            node_name = node.name;
            mesh_id = node.mesh;

            if (!node.matrix.empty())
            {
                local_matrix = vicmil::make_mat4(node.matrix.data());
            }
            else
            {
                glm::vec3 translation(0.0f);
                glm::quat rotation(1.0, 0.0, 0.0, 0.0);
                glm::vec3 scale(1.0f);

                if (!node.translation.empty())
                {
                    translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
                }

                if (!node.rotation.empty())
                {
                    rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]); // w,x,y,z
                }

                if (!node.scale.empty())
                {
                    scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
                }

                local_matrix = glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
            }
        }
    };

    /*void _get_node_data(const tinygltf::Model &model, std::map<int, NodeData> *node_data, std::vector<int> node_path, glm::mat4 parent_matrix)
    {
        int nodeIndex = node_path.back();
        const tinygltf::Node &node = model.nodes[nodeIndex];
        glm::mat4 localMatrix(1.0f);

        glm::mat4 worldMatrix = parent_matrix * localMatrix;

        (*node_data)[nodeIndex] = NodeData(node_path, localMatrix, worldMatrix);

        (*node_data)[nodeIndex].node_name = node.name;

        (*node_data)[nodeIndex].mesh_id = node.mesh;

        for (int child : node.children)
        {
            std::vector<int> child_path = node_path;
            child_path.push_back(child);
            _get_node_data(model, node_data, child_path, worldMatrix);
        }
    }*/

    std::map<std::vector<int>, NodeData> load_model_node_data(const tinygltf::Model &model)
    {
        std::map<std::vector<int>, NodeData> ret_map = {};

        std::vector<std::vector<int>> remaining_node_paths;

        // Initialize root node paths to look at first
        for (int sceneIndex = 0; sceneIndex < model.scenes.size(); sceneIndex++)
        {
            const tinygltf::Scene &scene = model.scenes[sceneIndex];

            for (int rootNodeIndex : scene.nodes)
            {
                remaining_node_paths.push_back({sceneIndex, rootNodeIndex});
            }
        }

        while (remaining_node_paths.size() > 0)
        {
            std::vector<int> node_path = remaining_node_paths.back();
            remaining_node_paths.pop_back();

            int node_id = node_path.back();

            ret_map[node_path] = NodeData();
            NodeData &node_data = ret_map[node_path];
            node_data.load_node_data(model, node_id);

            glm::mat4 parent_matrix = glm::mat4(1.0);

            if (node_path.size() != 2)
            {
                std::vector<int> parent_path = node_path;
                parent_path.pop_back();
                parent_matrix = ret_map[parent_path].world_matrix;
            }

            node_data.world_matrix = node_data.local_matrix * parent_matrix;
            node_data.path = node_path;

            for (int child : model.nodes[node_id].children)
            {
                std::vector<int> child_path = node_path;
                child_path.push_back(child);
                remaining_node_paths.push_back(child_path);
            }
        }

        return ret_map;
    }

    std::vector<MeshData> load_model_meshes(const tinygltf::Model &model)
    {
        std::vector<MeshData> ret_vec = {};
        ret_vec.resize(model.meshes.size());
        for (int i = 0; i < model.meshes.size(); i++)
        {
            ret_vec[i].load_model_mesh(model, i);
        }
        return ret_vec;
    }

    void extract_primitive_data_to_mesh(vicmil::Mesh &mesh, vicmil::MeshData::PrimitiveData &primitive, glm::mat4 world_matrix)
    {
        int index_offset = mesh.vertices.size();

        // Extract vertex data
        for (int vertex_id = 0; vertex_id < primitive.positions.size(); vertex_id++)
        {
            vicmil::Vertex vertex;
            vertex.material_id = primitive.material_id;

            // Setup position
            glm::vec4 pos = glm::vec4(primitive.positions[vertex_id], 1.0) * world_matrix;
            vertex.vertex_cord = vicmil::Coord_XYZ_f(pos.x, pos.y, pos.z);

            // Setup uv coordinates
            if (primitive.uv_coordinates.size() > vertex_id)
            {
                vertex.tex_cord.u = primitive.uv_coordinates[vertex_id].x;
                vertex.tex_cord.v = 1 - primitive.uv_coordinates[vertex_id].y;
            }

            // Setup normals
            if (primitive.normals.size() > vertex_id)
            {
                glm::vec4 normal = glm::vec4(primitive.positions[vertex_id], 0.0) * world_matrix;
                vertex.norm = vicmil::Normal_XYZ_f(pos.x, pos.y, pos.z);
            }

            mesh.vertices.push_back(vertex);
        }

        // Extract face data
        for (int triangle_id = 0; triangle_id < primitive.triangles.size(); triangle_id++)
        {
            glm::uvec3 vert_indices = primitive.triangles[triangle_id];
            vicmil::Face triangle_face;
            triangle_face.vertex_indices[0] = vert_indices.x + index_offset;
            triangle_face.vertex_indices[1] = vert_indices.y + index_offset;
            triangle_face.vertex_indices[2] = vert_indices.z + index_offset;
            mesh.faces.push_back(triangle_face);
        }
    }

    void extract_material_data_to_mesh(vicmil::Mesh &mesh, const vicmil::GLBLoader &glb_loader)
    {
        const tinygltf::Model &model = glb_loader.model;
        const std::vector<std::string> &material_names = glb_loader.embedded_images.image_names;

        for (size_t i = 0; i < model.materials.size(); i++)
        {
            const tinygltf::Material &gltf_material = model.materials[i];
            vicmil::Material material;

            // Material name
            material.name = gltf_material.name;

            // Base color factor -> Diffuse (Kd)
            if (gltf_material.pbrMetallicRoughness.baseColorFactor.size() >= 3)
            {
                material.Kd[0] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor[0]);
                material.Kd[1] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor[1]);
                material.Kd[2] = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor[2]);
            }

            // Emissive factor -> Ke
            if (gltf_material.emissiveFactor.size() >= 3)
            {
                material.Ke[0] = static_cast<float>(gltf_material.emissiveFactor[0]);
                material.Ke[1] = static_cast<float>(gltf_material.emissiveFactor[1]);
                material.Ke[2] = static_cast<float>(gltf_material.emissiveFactor[2]);
            }

            // Metallic & Roughness
            material.metallic = static_cast<float>(gltf_material.pbrMetallicRoughness.metallicFactor);
            material.roughness = static_cast<float>(gltf_material.pbrMetallicRoughness.roughnessFactor);

            // Transparency
            if (gltf_material.alphaMode == "BLEND")
                material.d = 0.5f; // semi-transparent
            else
                material.d = 1.0f;

            // Map texture indices to names
            auto getTextureName = [&](int texIndex) -> std::string
            {
                if (texIndex >= 0 && texIndex < static_cast<int>(material_names.size()))
                    return material_names[texIndex];
                return ""; // no texture
            };

            material.diffuse_texname = getTextureName(gltf_material.pbrMetallicRoughness.baseColorTexture.index);
            material.specular_texname = getTextureName(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index); // Optional mapping
            material.bump_texname = getTextureName(gltf_material.normalTexture.index);
            material.emissive_texname = getTextureName(gltf_material.emissiveTexture.index);

            // Ambient map (map_Ka) is not directly in glTF; usually same as diffuse
            material.ambient_texname = material.diffuse_texname;

            // Add material to mesh
            mesh.materials.push_back(material);
        }
    }

    vicmil::Mesh tiny_gltf_model_to_mesh(const vicmil::GLBLoader &glb_loader)
    {
        const tinygltf::Model &model = glb_loader.model;

        vicmil::Mesh ret_mesh = vicmil::Mesh();
        extract_material_data_to_mesh(ret_mesh, glb_loader);

        // glft structured into nodes, that then contain meshes
        std::vector<MeshData> model_meshes = load_model_meshes(model);
        std::map<std::vector<int>, NodeData> model_node_data = load_model_node_data(model);

        for (auto pair : model_node_data)
        {
            const std::vector<int> &path = pair.first;
            const NodeData &node_data = pair.second;

            if (node_data.has_mesh())
            {
                MeshData &mesh_data = model_meshes[node_data.mesh_id];

                for (int primitive_idx = 0; primitive_idx < mesh_data.primitives.size(); primitive_idx++)
                {
                    vicmil::MeshData::PrimitiveData &primitive = mesh_data.primitives[primitive_idx];
                    extract_primitive_data_to_mesh(ret_mesh, primitive, node_data.world_matrix);
                }
            }
        }

        return ret_mesh;
    }

    void PrintModel(const tinygltf::Model &model)
    {
        /*std::cout << "=== Buffers ===\n";
        for (const auto &buffer : model.buffers)
            PrintBuffer(buffer);

        std::cout << "=== BufferViews ===\n";
        for (const auto &bv : model.bufferViews)
            PrintBufferView(bv);

        std::cout << "=== Accessors ===\n";
        for (const auto &acc : model.accessors)
            PrintAccessor(acc);*/

        // std::cout << "=== Images ===\n";
        // for (const auto &img : model.images)
        //     PrintImage(img);

        std::cout << "=== Materials ===\n";
        for (const auto &mat : model.materials)
            PrintMaterial(mat);

        // std::cout << "=== Meshes ===\n";
        // for (const auto &mesh : model.meshes)
        //     PrintMeshData(model, mesh);

        // std::cout << "=== Nodes ===\n";
        // for (const auto &node : model.nodes)
        //     PrintNode(node);

        std::cout << "=== Scenes ===\n";
        for (const auto &scene : model.scenes)
            PrintScene(scene);

        std::vector<MeshData> model_meshes = load_model_meshes(model);
        for (int i = 0; i < model_meshes.size(); i++)
        {
            std::cout << model_meshes[i].to_string(false) << std::endl;
        }

        std::cout << "Default scene index: " << model.defaultScene << "\n";

        std::map<std::vector<int>, NodeData> model_node_data = load_model_node_data(model);
        for (auto pair : model_node_data)
        {
            const std::vector<int> &path = pair.first;
            const NodeData &node_data = pair.second;

            if (node_data.has_mesh())
            {
                std::cout << node_data.to_string() << std::endl;
            }
        }
    }
}
