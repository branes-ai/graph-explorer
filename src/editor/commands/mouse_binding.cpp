#include "commands/mouse_binding.hpp"
#include "commands/command.hpp"

namespace editor {

Mouse_binding::Mouse_binding(Command* const command)
    : Command_binding{command}
{
}

auto Mouse_binding::on_button(
    Command_context&                  context,
    const erhe::toolkit::Mouse_button button,
    const int                         count
) -> bool
{
    static_cast<void>(context);
    static_cast<void>(button);
    static_cast<void>(count);
    return false;
}

auto Mouse_binding::on_motion(Command_context& context) -> bool
{
    static_cast<void>(context);
    return false;
}

} // namespace Editor

