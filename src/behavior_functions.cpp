// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "state.hpp"
#include "behavior_functions.hpp"


void
behavior_functions::test(entities::Handle entity_handle)
{
    spatial::Component *spatial_component =
        engine::get_spatial_component(entity_handle);
    if (!spatial_component) {
        logs::error("Could not get spatial::Component for behavior::Component");
        return;
    }

    spatial_component->rotation =
        glm::angleAxis((f32)sin(1.0f - (engine::get_t())), v3(0.0f, 1.0f, 0.0f)) *
        glm::angleAxis((f32)cos(1.0f - (engine::get_t())), v3(1.0f, 0.0f, 0.0f));
}


void
behavior_functions::char_movement_test(entities::Handle entity_handle)
{
    spatial::Component *spatial_component =
        engine::get_spatial_component(entity_handle);
    if (!spatial_component) {
        logs::error("Could not get spatial::Component for behavior::Component");
        return;
    }

    physics::Component *physics_component =
        engine::get_physics_component(entity_handle);
    if (!physics_component) {
        logs::error("Could not get physics::Component for behavior::Component");
        return;
    }
    spatial::Obb *obb = &physics_component->transformed_obb;

    // Update position
    spatial_component->position.x =
        (f32)sin((engine::get_t()) * 1.0f) * 4.0f +
        (f32)sin((engine::get_t()) * 2.0f) * 0.1f +
        (f32)cos((engine::get_t()) * 3.0f) * 0.3f;
    spatial_component->position.z =
        (f32)cos((engine::get_t()) * 1.0f) * 4.0f +
        (f32)cos((engine::get_t()) * 2.0f) * 0.3f +
        (f32)sin((engine::get_t()) * 3.0f) * 0.1f;
    spatial_component->rotation =
        glm::angleAxis(
            (f32)sin((engine::get_t()) * 3.0f) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)
        ) *
        glm::angleAxis(
            (f32)cos((engine::get_t()) * 2.0f) / 3.0f, v3(0.0f, 1.0f, 0.0f)
        ) *
        glm::angleAxis((f32)cos((engine::get_t()) * 2.0f), v3(1.0f, 0.0f, 0.0f)) *
        glm::angleAxis((f32)sin((engine::get_t()) * 1.5f) / 2.0f, v3(1.0f, 0.0f, 0.0f)) *
        glm::angleAxis((f32)sin((engine::get_t()) * 2.5f) / 1.5f, v3(0.5f, 0.5f, 0.2f));
#if 0
    spatial_component->position.x = -5.0f;
    spatial_component->position.z = -5.0f;
    spatial_component->rotation =
        glm::angleAxis((f32)sin((engine::get_t())) + radians(70.0f), v3(0.0f, 1.0f, 0.0f)) *
        glm::angleAxis(radians(90.0f), v3(1.0f, 0.0f, 0.0f));
#endif

    // Check collision with other entities
    {
        physics::CollisionManifold manifold = physics::find_collision(
            physics_component, spatial_component);

        if (manifold.did_collide) {
            v4 color;
            if (manifold.axis <= 5) {
                color = v4(1.0f, 0.0f, 0.0f, 1.0f);
            } else {
                color = v4(1.0f, 1.0f, 0.0f, 1.0f);
            }
            debugdraw::draw_obb(obb, color);
            debugdraw::draw_obb(&manifold.collidee->transformed_obb, color);
            debugdraw::draw_line(obb->center,
                obb->center + manifold.normal * 100.0f, color);
            gui::log("manifold.axis = %d", manifold.axis);
            gui::log("manifold.sep_max = %f", manifold.sep_max);
            gui::log("manifold.normal = (%f, %f, %f)",
                manifold.normal.x, manifold.normal.y, manifold.normal.z);
            gui::log("length(manifold.normal) = %f", length(manifold.normal));
            gui::log("---");
        } else {
            debugdraw::draw_obb(obb, v4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }

    // Check ray collision
#if 0
    {
        spatial::Ray ray = {
            .origin = obb->center + obb->y_axis * obb->extents[1],
            .direction = obb->y_axis,
        };
        RayCollisionResult ray_collision_result = physics::find_ray_collision(
            &ray, physics_component);

        if (ray_collision_result.did_intersect) {
            debugdraw::draw_ray(&ray, ray_collision_result.distance,
                v4(1.0f, 0.0f, 0.0f, 0.0f));
            debugdraw::draw_obb(&ray_collision_result.collidee->transformed_obb,
                v4(1.0f, 0.0f, 0.0f, 1.0f));
        } else {
            debugdraw::draw_ray(&ray, 500.0f,
                v4(1.0f, 1.0f, 1.0f, 0.0f));
        }
    }
#endif
}
