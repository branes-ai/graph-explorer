#pragma once

#include "erhe_profile/profile.hpp"

#include <mutex>
#include <vector>

namespace explorer {

class Explorer_context;
class Scene_root;
class Time_context;

class Explorer_scenes
{
public:
    Explorer_scenes(Explorer_context& editor_context);

    void register_scene_root                 (Scene_root* scene_root);
    void unregister_scene_root               (Scene_root* scene_root);
    void sanity_check                        ();

    void before_physics_simulation_steps     ();
    void update_physics_simulation_fixed_step(const Time_context& time);
    void after_physics_simulation_steps      ();
    void update_node_transforms              ();

    [[nodiscard]] auto get_scene_roots() -> const std::vector<Scene_root*>&;
    [[nodiscard]] auto scene_combo(const char* label, Scene_root*& in_out_selected_entry, bool empty_option) const -> bool;

    void imgui();

private:
    Explorer_context&              m_context;
    ERHE_PROFILE_MUTEX(std::mutex, m_mutex);
    std::vector<Scene_root*>       m_scene_roots;
};

} // namespace explorer

