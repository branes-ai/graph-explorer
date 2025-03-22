#pragma once

#include "erhe_commands/command.hpp"
#include "erhe_imgui/imgui_window.hpp"

#include <memory>
#include <vector>

namespace erhe::commands {
    class CommandS;
}
namespace erhe::imgui {
    class Imgui_windows;
}
namespace tf {
    class Executor;
}

namespace explorer {

class Explorer_context;
class Explorer_message_bus;
class Operation;
class Explorer_context;
class Operation_stack;
class Selection_tool;

class Undo_command : public erhe::commands::Command
{
public:
    Undo_command(erhe::commands::Commands& commands, Explorer_context& context);
    auto try_call() -> bool override;

private:
    Explorer_context& m_context;
};

class Redo_command : public erhe::commands::Command
{
public:
    Redo_command(erhe::commands::Commands& commands, Explorer_context& context);
    auto try_call() -> bool override;

private:
    Explorer_context& m_context;
};

class Operation_stack
    : public erhe::imgui::Imgui_window
    , public erhe::commands::Command_host
{
public:
    Operation_stack(
        tf::Executor&                executor,
        erhe::commands::Commands&    commands,
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&              editor_context
    );
    ~Operation_stack();

    [[nodiscard]] auto can_undo() const -> bool;
    [[nodiscard]] auto can_redo() const -> bool;
    void queue(const std::shared_ptr<Operation>& operation);
    void undo();
    void redo();

    void update();

    // Implements Window
    void imgui() override;

    [[nodiscard]] auto get_executor() -> tf::Executor&;

private:
    void imgui(const char* stack_label, const std::vector<std::shared_ptr<Operation>>& operations);

    Explorer_context& m_context;
    tf::Executor&   m_executor;

    Undo_command m_undo_command;
    Redo_command m_redo_command;

    std::vector<std::shared_ptr<Operation>> m_executed;
    std::vector<std::shared_ptr<Operation>> m_undone;
    std::vector<std::shared_ptr<Operation>> m_queued;
};

} // namespace explorer
