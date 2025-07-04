#pragma once
#include <util_std.hpp>

namespace vicmil
{
    template <typename T>
    class PoolAllocator
    {
    public:
        PoolAllocator(size_t capacity = 1024)
        {
            grow(capacity);
        }

        // Allocate an object
        T *allocate()
        {
            if (free_indices.empty())
            {
                grow(capacity * 2); // Double the capacity
            }

            size_t index = free_indices.top();
            free_indices.pop();

            T *ptr = &storage[index];
            new (ptr) T(); // placement new

            return ptr;
        }

        // Deallocate an object
        void deallocate(T *ptr)
        {
            size_t index = ptr - storage.data();
            assert(index < storage.size() && "Invalid pointer deallocation");

            ptr->~T(); // call destructor
            free_indices.push(index);
        }

        // Return raw storage
        std::vector<T> &get_storage()
        {
            return storage;
        }

    private:
        std::vector<T> storage;
        std::stack<size_t> free_indices;
        size_t capacity = 0;

        void grow(size_t new_capacity)
        {
            size_t old_capacity = storage.size();
            storage.resize(new_capacity);
            for (size_t i = new_capacity - 1; i >= old_capacity; --i)
            {
                free_indices.push(i);
                if (i == 0)
                {
                    break; // Prevent underflow
                }
            }
            capacity = new_capacity;
        }
    };

    /*
    ===================================================================
                            Entity management
    ===================================================================
    */

    // Reference to an entity
    struct EntityHandle
    {
        uint64_t id = 0;
        uint64_t generation = 0;

        bool operator==(const EntityHandle &other) const
        {
            return id == other.id && generation == other.generation;
        }

        bool is_valid() const
        {
            return generation > 0; // Optional: use 0 as "null"
        }

        EntityHandle(uint64_t id_, uint64_t generation_) : id(id_), generation(generation_) {}
        EntityHandle() {}

        static EntityHandle null()
        {
            return EntityHandle();
        }
    };
}

namespace std
{
    template <>
    struct hash<vicmil::EntityHandle>
    {
        size_t operator()(const vicmil::EntityHandle &h) const
        {
            return (size_t(h.id) << 32) | h.generation;
        }
    };
}

// Create/remove entity references
namespace vicmil
{
    class EntityManager
    {
    public:
        EntityManager(uint64_t start_id = 0, uint64_t max_count = (uint64_t)1000 * (uint64_t)1000 * (uint64_t)1000 * (uint64_t)1000)
            : start_id(start_id), max_count(max_count)
        {
        }
        EntityHandle create_entity()
        {
            uint64_t id;
            if (!free_ids.empty())
            {
                id = free_ids.back();
                free_ids.pop_back();
                ++generations[id];
            }
            else
            {
                if (generations.size() >= max_count)
                {
                    ThrowError("Allocating outside of range!");
                }
                id = generations.size();
                generations.push_back(1);
            }
            return EntityHandle{id + start_id, generations[id]};
        }

        inline bool handle_in_range(EntityHandle handle) const
        {
            return handle.id >= start_id && handle.id < start_id + max_count;
        }

        bool is_alive(EntityHandle handle) const
        {
            if (!handle_in_range(handle))
            {
                return false;
            }
            return generations[handle.id - start_id] == handle.generation;
        }

        void destroy_entity(EntityHandle handle)
        {
            if (!is_alive(handle))
            {
                return;
            }
            ++generations[handle.id - start_id]; // Invalidate old handles
            free_ids.push_back(handle.id - start_id);
        }

    private:
        uint64_t start_id;
        uint64_t max_count;
        std::vector<uint64_t> generations;
        std::vector<uint64_t> free_ids;
    };

    struct IComponentStorage
    {
        virtual ~IComponentStorage() {}
        virtual void remove(EntityHandle e) {}
        virtual bool has(EntityHandle e) const { return false; }
    };

    // Manage single entity component
    template <typename T>
    class ComponentStorage : public IComponentStorage
    {
    public:
        void add(EntityHandle e, const T &component)
        {
            _data[e] = component;
        }

        void remove(EntityHandle e) override
        {
            _data.erase(e);
        }

        T *get(EntityHandle e)
        {
            auto it = _data.find(e);
            return it != _data.end() ? &it->second : nullptr;
        }

        bool has(EntityHandle e) const override
        {
            return _data.count(e) > 0;
        }

        std::unordered_map<EntityHandle, T> _data;
    };

    // Manage all entity components
    class ComponentManager
    {
    public:
        template <typename T>
        void register_component()
        {
            std::type_index index(typeid(T));
            assert(component_map.find(index) == component_map.end());
            ComponentStorage<T> *new_component_storage = new ComponentStorage<T>();
            component_map[index] = new_component_storage;
            component_interface_map[index] = new_component_storage;
        }

        template <typename T>
        ComponentStorage<T> *get_storage()
        {
            std::type_index index(typeid(T));
            auto it = component_map.find(index);
            assert(it != component_map.end() && "Component not registered!");
            void *storage_ptr = it->second;
            return static_cast<ComponentStorage<T> *>(storage_ptr);
        }

        template <typename T>
        void add_component(EntityHandle e, const T &component)
        {
            get_storage<T>()->add(e, component);
        }

        template <typename T>
        T *get_component(EntityHandle e)
        {
            return get_storage<T>()->get(e);
        }

        template <typename T>
        void remove_component(EntityHandle e)
        {
            get_storage<T>()->remove(e);
        }

        void remove_components(EntityHandle e)
        {
            // Remove all references to the entity
            for (auto &pair : component_interface_map)
            {
                IComponentStorage *component_storage = pair.second;
                if (component_storage->has(e))
                {
                    component_storage->remove(e);
                }
            }
        }

        ~ComponentManager()
        {
            for (auto &pair : component_interface_map)
            {
                IComponentStorage *component_storage = pair.second;
                delete component_storage;
            }
        }

    private:
        std::unordered_map<std::type_index, void *> component_map;
        std::unordered_map<std::type_index, IComponentStorage *> component_interface_map;
    };

    /*
    ===================================================================
                           Chunk allocation in 2d
    ===================================================================
    */

    struct ChunkKey2D
    {
        int x, y;
        bool operator==(const ChunkKey2D &other) const
        {
            return x == other.x && y == other.y;
        }
    };
}
namespace std
{
    template <>
    struct hash<vicmil::ChunkKey2D>
    {
        size_t operator()(const vicmil::ChunkKey2D &k) const
        {
            return hash<int>()(k.x) ^ (hash<int>()(k.y) << 1);
        }
    };
}

namespace vicmil
{
    class GridChunk2D
    {
    public:
        int _x, _y, _w, _h;
        std::vector<EntityHandle> obj_id;

        GridChunk2D(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h)
        {
            obj_id.resize(w * h, EntityHandle::null()); // -1 means empty
        }

        GridChunk2D() {}

        EntityHandle get_obj(int x, int y)
        {
            int local_x = x - _x * _w;
            int local_y = y - _y * _h;
            return obj_id[local_y * _w + local_x];
        }

        void set_obj(int x, int y, EntityHandle val)
        {
            int local_x = x - _x * _w;
            int local_y = y - _y * _h;
            obj_id[local_y * _w + local_x] = val;
        }
    };

    class GridChunkHashmap2D
    {
    public:
        std::unordered_map<ChunkKey2D, GridChunk2D> chunk_hashmap;
        int chunk_size;

        GridChunkHashmap2D(int chunk_size_) : chunk_size(chunk_size_) {}

        void get_chunk_pos(int x, int y, int *chunk_x, int *chunk_y)
        {
            *chunk_x = x / chunk_size;
            *chunk_y = y / chunk_size;
        }

        GridChunk2D *get_chunk(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            auto it = chunk_hashmap.find(key);
            if (it != chunk_hashmap.end())
            {
                return &it->second;
            }
            return nullptr;
        }

        GridChunk2D *create_chunk(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            chunk_hashmap[key] = GridChunk2D(chunk_x, chunk_y, chunk_size, chunk_size);
            return &chunk_hashmap[key];
        }

        EntityHandle get_obj(int x, int y)
        {
            int chunk_x, chunk_y;
            get_chunk_pos(x, y, &chunk_x, &chunk_y);
            auto chunk = get_chunk(chunk_x, chunk_y);
            if (!chunk)
            {
                return EntityHandle::null();
            }
            return chunk->get_obj(x, y);
        }

        void set_obj(int x, int y, EntityHandle val)
        {
            int chunk_x, chunk_y;
            get_chunk_pos(x, y, &chunk_x, &chunk_y);
            ChunkKey2D key{chunk_x, chunk_y};
            if (chunk_hashmap.find(key) == chunk_hashmap.end())
            {
                chunk_hashmap[key] = GridChunk2D(chunk_x, chunk_y, chunk_size, chunk_size);
            }
            chunk_hashmap[key].set_obj(x, y, val);
        }

        void delete_chunk(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            chunk_hashmap.erase(key);
        }
    };

    // Used for free-positioned objects
    class FreeChunk2D
    {
    public:
        int _x, _y, _w, _h;
        std::set<EntityHandle> objects;

        FreeChunk2D(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h) {}
    };

    class FreeChunkHashmap2D
    {
    public:
        std::unordered_map<ChunkKey2D, FreeChunk2D> chunk_hashmap;
        int chunk_size;

        FreeChunkHashmap2D(int chunk_size) : chunk_size(chunk_size) {}

        void get_chunk_pos(int x, int y, int *chunk_x, int *chunk_y)
        {
            *chunk_x = x / chunk_size;
            *chunk_y = y / chunk_size;
        }

        std::set<EntityHandle> *get_chunk_objects(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            auto it = chunk_hashmap.find(key);
            if (it != chunk_hashmap.end())
                return &it->second.objects;
            return nullptr;
        }

        FreeChunk2D *get_chunk(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            auto it = chunk_hashmap.find(key);
            if (it != chunk_hashmap.end())
                return &it->second;
            return nullptr;
        }

        void new_chunk(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            chunk_hashmap.emplace(key, FreeChunk2D(chunk_x, chunk_y, chunk_size, chunk_size));
        }

        void delete_chunk(int chunk_x, int chunk_y)
        {
            ChunkKey2D key{chunk_x, chunk_y};
            chunk_hashmap.erase(key);
        }
    };

    /*
    ===================================================================
                           Chunk allocation in 3d
    ===================================================================
    */

    struct ChunkKey3D
    {
        int x, y, z;
        bool operator==(const ChunkKey3D &other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };

}
namespace std
{
    template <>
    struct hash<vicmil::ChunkKey3D>
    {
        size_t operator()(const vicmil::ChunkKey3D &k) const
        {
            return ((hash<int>()(k.x) ^ (hash<int>()(k.y) << 1)) >> 1) ^ (hash<int>()(k.z) << 1);
        }
    };
}
namespace vicmil
{

    class GridChunk3D
    {
    public:
        int _x, _y, _z, _w, _h, _d;
        std::vector<EntityHandle> obj_id;

        GridChunk3D(int x, int y, int z, int w, int h, int d) : _x(x), _y(y), _z(z), _w(w), _h(h), _d(d)
        {
            obj_id.resize(w * h * d, EntityHandle::null());
        }
        GridChunk3D() {}

        EntityHandle get_obj(int x, int y, int z)
        {
            int lx = x - _x * _w;
            int ly = y - _y * _h;
            int lz = z - _z * _d;
            return obj_id[lz * _w * _h + ly * _w + lx];
        }

        void set_obj(int x, int y, int z, EntityHandle val)
        {
            int lx = x - _x * _w;
            int ly = y - _y * _h;
            int lz = z - _z * _d;
            obj_id[lz * _w * _h + ly * _w + lx] = val;
        }
    };

    class GridChunkHashmap3D
    {
    public:
        std::unordered_map<ChunkKey3D, GridChunk3D> chunk_hashmap;
        int chunk_size;

        GridChunkHashmap3D(int chunk_size) : chunk_size(chunk_size) {}

        void get_chunk_pos(int x, int y, int z, int *cx, int *cy, int *cz)
        {
            *cx = x / chunk_size;
            *cy = y / chunk_size;
            *cz = z / chunk_size;
        }

        GridChunk3D *get_chunk(int cx, int cy, int cz)
        {
            ChunkKey3D key{cx, cy, cz};
            auto it = chunk_hashmap.find(key);
            if (it != chunk_hashmap.end())
                return &it->second;
            return nullptr;
        }

        GridChunk3D *create_chunk(int chunk_x, int chunk_y, int chunk_z)
        {
            ChunkKey3D key{chunk_x, chunk_y, chunk_z};
            chunk_hashmap[key] = GridChunk3D(chunk_x, chunk_y, chunk_z, chunk_size, chunk_size, chunk_size);
            return &chunk_hashmap[key];
        }

        EntityHandle get_obj(int x, int y, int z)
        {
            int cx, cy, cz;
            get_chunk_pos(x, y, z, &cx, &cy, &cz);
            auto chunk = get_chunk(cx, cy, cz);
            if (!chunk)
                return EntityHandle::null();
            return chunk->get_obj(x, y, z);
        }

        void set_obj(int x, int y, int z, EntityHandle val)
        {
            int cx, cy, cz;
            get_chunk_pos(x, y, z, &cx, &cy, &cz);
            ChunkKey3D key{cx, cy, cz};
            if (chunk_hashmap.find(key) == chunk_hashmap.end())
            {
                chunk_hashmap[key] = GridChunk3D(cx, cy, cz, chunk_size, chunk_size, chunk_size);
            }
            chunk_hashmap[key].set_obj(x, y, z, val);
        }

        void delete_chunk(int cx, int cy, int cz)
        {
            chunk_hashmap.erase(ChunkKey3D{cx, cy, cz});
        }
    };

    class FreeChunk3D
    {
    public:
        int _x, _y, _z, _w, _h, _d;
        std::set<EntityHandle> objects;

        FreeChunk3D(int x, int y, int z, int w, int h, int d) : _x(x), _y(y), _z(z), _w(w), _h(h), _d(d) {}
    };

    class FreeChunkHashmap3D
    {
    public:
        std::unordered_map<ChunkKey3D, FreeChunk3D> chunk_hashmap;
        int chunk_size;

        FreeChunkHashmap3D(int chunk_size) : chunk_size(chunk_size) {}

        void get_chunk_pos(int x, int y, int z, int *cx, int *cy, int *cz)
        {
            *cx = x / chunk_size;
            *cy = y / chunk_size;
            *cz = z / chunk_size;
        }

        std::set<EntityHandle> *get_chunk_objects(int cx, int cy, int cz)
        {
            ChunkKey3D key{cx, cy, cz};
            auto it = chunk_hashmap.find(key);
            if (it != chunk_hashmap.end())
                return &it->second.objects;
            return nullptr;
        }

        FreeChunk3D *get_chunk(int cx, int cy, int cz)
        {
            ChunkKey3D key{cx, cy, cz};
            auto it = chunk_hashmap.find(key);
            if (it != chunk_hashmap.end())
                return &it->second;
            return nullptr;
        }

        void new_chunk(int cx, int cy, int cz)
        {
            ChunkKey3D key{cx, cy, cz};
            chunk_hashmap.emplace(key, FreeChunk3D(cx, cy, cz, chunk_size, chunk_size, chunk_size));
        }

        void delete_chunk(int cx, int cy, int cz)
        {
            chunk_hashmap.erase(ChunkKey3D{cx, cy, cz});
        }
    };
}