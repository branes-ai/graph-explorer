#include "operations/operation_stack.hpp"

#include "editor_context.hpp"
#include "operations/ioperation.hpp"
#include "tools/tool.hpp"

#include "erhe/imgui/imgui_windows.hpp"
#include "erhe/commands/commands.hpp"
#include "erhe/toolkit/profile.hpp"

#if defined(ERHE_GUI_LIBRARY_IMGUI)
#   include <imgui/imgui.h>
#endif

namespace editor
{

IOperation::~IOperation() noexcept
{
}

#pragma region Commands
Undo_command::Undo_command(
    erhe::commands::Commands& commands,
    Editor_context&           context
)
    : Command  {commands, "undo"}
    , m_context{context}
{
}

auto Undo_command::try_call() -> bool
{
    if (m_context.operation_stack->can_undo()) {
        m_context.operation_stack->undo();
        return true;
    } else {
        return false;
    }
}

Redo_command::Redo_command(
    erhe::commands::Commands& commands,
    Editor_context&           context
)
    : Command  {commands, "redo"}
    , m_context{context}
{
}

auto Redo_command::try_call() -> bool
{
    if (m_context.operation_stack->can_redo()) {
        m_context.operation_stack->redo();
        return true;
    } else {
        return false;
    }
}
#pragma endregion Commands

Operation_stack::Operation_stack(
    erhe::commands::Commands&    commands,
    erhe::imgui::Imgui_renderer& imgui_renderer,
    erhe::imgui::Imgui_windows&  imgui_windows,
    Editor_context&              editor_context
)
    : erhe::imgui::Imgui_window{imgui_renderer, imgui_windows, "Operation Stack", "operation_stack"}
    , m_context     {editor_context}
    , m_undo_command{commands, editor_context}
    , m_redo_command{commands, editor_context}
{
    commands.register_command(&m_undo_command);
    commands.register_command(&m_redo_command);
    commands.bind_command_to_key(&m_undo_command, erhe::toolkit::Key_z, true, erhe::toolkit::Key_modifier_bit_ctrl);
    commands.bind_command_to_key(&m_redo_command, erhe::toolkit::Key_y, true, erhe::toolkit::Key_modifier_bit_ctrl);

    m_undo_command.set_host(this);
    m_redo_command.set_host(this);
}

void Operation_stack::push(
    const std::shared_ptr<IOperation>& operation
)
{
    operation->execute(m_context);
    m_executed.push_back(operation);
    m_undone.clear();
}

void Operation_stack::undo()
{
    if (m_executed.empty()) {
        return;
    }
    auto operation = m_executed.back(); // intentionally not a reference, otherwise pop_back() below will invalidate
    m_executed.pop_back();
    operation->undo(m_context);
    m_undone.push_back(operation);
}

void Operation_stack::redo()
{
    if (m_undone.empty()) {
        return;
    }
    auto operation = m_undone.back(); // intentionally not a reference, otherwise pop_back() below will invalidate
    m_undone.pop_back();
    operation->execute(m_context);
    m_executed.push_back(operation);
}

auto Operation_stack::can_undo() const -> bool
{
    return !m_executed.empty();
}

auto Operation_stack::can_redo() const -> bool
{
    return !m_undone.empty();
}

#if defined(ERHE_GUI_LIBRARY_IMGUI)
void Operation_stack::imgui(
    const char*                                     stack_label,
    const std::vector<std::shared_ptr<IOperation>>& operations
)
{
    const ImGuiTreeNodeFlags parent_flags{
        ImGuiTreeNodeFlags_OpenOnArrow       |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanFullWidth
    };
    const ImGuiTreeNodeFlags leaf_flags{
        ImGuiTreeNodeFlags_SpanFullWidth    |
        ImGuiTreeNodeFlags_NoTreePushOnOpen |
        ImGuiTreeNodeFlags_Leaf
    };

    if (ImGui::TreeNodeEx(stack_label, parent_flags)) {
        for (const auto& op : operations) {
            ImGui::TreeNodeEx(op->describe().c_str(), leaf_flags);
        }
        ImGui::TreePop();
    }
}
#endif

void Operation_stack::imgui()
{
#if defined(ERHE_GUI_LIBRARY_IMGUI)
    imgui("Executed", m_executed);
    imgui("Undone", m_undone);
#endif
}

} // namespace editor
