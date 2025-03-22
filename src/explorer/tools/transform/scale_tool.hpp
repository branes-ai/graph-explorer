#pragma once

#include "tools/tool.hpp"
#include "tools/transform/subtool.hpp"

#include <string_view>

namespace explorer {

class Icon_set;
class Transform_tool;

enum class Handle : unsigned int;

class Scale_tool : public Subtool
{
public:
    static constexpr int c_priority{1};

    Scale_tool(Explorer_context& editor_context, Icon_set& icon_set, Tools& tools);

    // Implements Tool
    void handle_priority_update(int old_priority, int new_priority) override;

    // Implemennts Subtool
    void imgui ()                                                       override;
    auto begin (unsigned int axis_mask, Scene_view* scene_view) -> bool override;
    auto update(Scene_view* scene_view) -> bool                         override;

private:
    void update(const glm::vec3 drag_position);
};

} // namespace explorer
