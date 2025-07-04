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
        uint64_t id;
        uint64_t generation;

        bool operator==(const EntityHandle &other) const
        {
            return id == other.id && generation == other.generation;
        }

        bool is_valid() const
        {
            return generation > 0; // Optional: use 0 as "null"
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
                generations.push_back(0);
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
}