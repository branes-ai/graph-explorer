#include "renderers/composer.hpp"
#include "explorer_log.hpp"
#include "renderers/renderpass.hpp"

#include "erhe_profile/profile.hpp"

#include <imgui/imgui.h>

namespace explorer {

// TODO Do deep copy instead / use Hierarchy

Composer::Composer(const Composer& other)
    : renderpasses{other.renderpasses}
{
}

Composer& Composer::operator=(const Composer& other)
{
    renderpasses = other.renderpasses;
    return *this;
}

Composer::Composer(Composer&& old)
    : renderpasses{std::move(old.renderpasses)}
{
}

Composer& Composer::operator=(Composer&& old)
{
    if (this != &old) {
        renderpasses = std::move(old.renderpasses);
    }
    return *this;
}

Composer::~Composer() noexcept
{
}

Composer::Composer(const std::string_view name)
    : Item{name}
{
}

auto Composer::get_type() const -> uint64_t
{
    return get_static_type();
}

auto Composer::get_type_name() const -> std::string_view
{
    return static_type_name;
}

void Composer::render(const Render_context& context)
{
    log_composer->trace("Composer::render()");
    std::lock_guard<ERHE_PROFILE_LOCKABLE_BASE(std::mutex)> scene_lock{mutex};

    for (const auto& renderpass : renderpasses) {
        log_composer->trace("  rp: {}", renderpass->describe());
        renderpass->render(context);
    }
}

void Composer::imgui()
{
    if (!ImGui::TreeNodeEx("Composer", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    int renderpass_index = 0;
    std::lock_guard<ERHE_PROFILE_LOCKABLE_BASE(std::mutex)> scene_lock{mutex};

    for (const auto& renderpass : renderpasses) {
        ImGui::PushID(renderpass_index++);
        if (ImGui::TreeNodeEx(renderpass->describe().c_str(), ImGuiTreeNodeFlags_Framed)) {
            renderpass->imgui();
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    ImGui::TreePop();
}

} // namespace explorer
