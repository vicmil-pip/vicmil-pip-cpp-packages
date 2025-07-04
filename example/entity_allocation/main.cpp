#include "util_entities.hpp"

struct Position
{
    float x, y;
};

struct Velocity
{
    float dx, dy;
};

void print_component_info(vicmil::ComponentManager &cm, vicmil::EntityManager &em, vicmil::EntityHandle e)
{
    // Access and update
    auto pos = cm.get_component<Position>(e);
    if (pos)
    {
        std::cout << "Position: " << pos->x << ", " << pos->y << std::endl;
    }
    else
    {
        std::cout << "No Position" << std::endl;
    }

    auto vel = cm.get_component<Velocity>(e);
    if (vel)
    {
        std::cout << "Velocity: " << vel->dx << ", " << vel->dy << std::endl;
    }
    else
    {
        std::cout << "No Velocity" << std::endl;
    }

    if (em.is_alive(e))
    {
        std::cout << "Entity is alive!\n";
    }
    else
    {
        std::cout << "Entity destroyed!\n";
    }
}

int main()
{
    // Setup
    vicmil::EntityManager em;
    vicmil::ComponentManager cm;

    cm.register_component<Position>();
    cm.register_component<Velocity>();

    // Create entity
    vicmil::EntityHandle e = em.create_entity();

    cm.add_component<Position>(e, {10.0f, 20.0f});
    cm.add_component<Velocity>(e, {1.0f, 0.0f});

    print_component_info(cm, em, e);

    // Destroy entity
    em.destroy_entity(e);
    cm.remove_components(e);

    print_component_info(cm, em, e);
}