#pragma once

#include "erhe_commands/command.hpp"
#include "windows/item_tree_window.hpp"

#include "erhe_imgui/imgui_window.hpp"
#include "erhe_item/hierarchy.hpp"

#include <glm/glm.hpp>

#include <filesystem>
#include <vector>

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

class Project_node
    : public erhe::Item<erhe::Item_base, erhe::Hierarchy, Project_node>
{
public:
    explicit Project_node(const Project_node&);
    Project_node& operator=(const Project_node&);
    ~Project_node() noexcept override;
    explicit Project_node(const std::filesystem::path& path);
};

class Project_folder : public erhe::Item<erhe::Item_base, Project_node, Project_folder>
{
public:
    explicit Project_folder(const Project_folder&);
    Project_folder& operator=(const Project_folder&);
    ~Project_folder() noexcept override;

    explicit Project_folder(const std::filesystem::path& path);

    // Implements Item_base
    static constexpr std::string_view static_type_name{"Project_folder"};
    [[nodiscard]] static auto get_static_type() -> uint64_t;
    auto get_type     () const -> uint64_t         override;
    auto get_type_name() const -> std::string_view override;
};

class Project_file_other : public erhe::Item<erhe::Item_base, Project_node, Project_file_other>
{
public:
    explicit Project_file_other(const Project_file_other& src);
    Project_file_other& operator=(const Project_file_other& src);
    ~Project_file_other() noexcept override;

    explicit Project_file_other(const std::filesystem::path& path);

    // Implements Item_base
    static constexpr std::string_view static_type_name{"Project_file_other"};
    [[nodiscard]] static auto get_static_type() -> uint64_t;
    auto get_type     () const -> uint64_t         override;
    auto get_type_name() const -> std::string_view override;
};

class Project_explorer;

class Project_explorer_window : public Item_tree_window
{
public:
    Project_explorer_window(
        Project_explorer&                       Project_explorer,
        erhe::imgui::Imgui_renderer&            imgui_renderer,
        erhe::imgui::Imgui_windows&             imgui_windows,
        Explorer_context&                       context,
        const std::string_view                  window_title,
        const std::string_view                  ini_label,
        const std::shared_ptr<erhe::Hierarchy>& root
    );

    void imgui() override;

private:
    Project_explorer& m_project_explorer;
};

class Project_explorer
{
public:
    Project_explorer(
        erhe::commands::Commands&    commands,
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context
    );

    void create_project();

    void scan();

private:
    void scan(const std::filesystem::path& path, Project_node* parent);

    auto make_node    (const std::filesystem::path& path, Project_node* parent) -> std::shared_ptr<Project_node>;
    auto item_callback(const std::shared_ptr<erhe::Item_base>& item) -> bool;

    Explorer_context& m_context;
    Project_node*     m_popup_node{nullptr};

    erhe::commands::Lambda_command m_create_project_command;

    std::shared_ptr<Project_node>            m_root;
    std::shared_ptr<Project_explorer_window> m_node_tree_window;
};

} // namespace explorer
