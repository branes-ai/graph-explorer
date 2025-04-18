#pragma once

#include "erhe_commands/command.hpp"
#include "windows/item_tree_window.hpp"

#include "erhe_imgui/imgui_window.hpp"
#include "erhe_item/hierarchy.hpp"
#include "erhe_window/window.hpp"

#include <filesystem>
#include <map>
#include <memory>

namespace sw::dfa {
    struct DomainFlowGraph;
}

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;
class Graph_node;
class Graph_window;

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

class Domain_flow_graph_file : public erhe::Item<erhe::Item_base, Project_node, Domain_flow_graph_file>
{
public:
    explicit Domain_flow_graph_file(const Domain_flow_graph_file& src);
    Domain_flow_graph_file& operator=(const Domain_flow_graph_file& src);
    ~Domain_flow_graph_file() noexcept override;

    explicit Domain_flow_graph_file(const std::filesystem::path& path);

    // Implements Item_base
    static constexpr std::string_view static_type_name{"Domain_flow_graph_file"};
    [[nodiscard]] static auto get_static_type() -> uint64_t;
    auto get_type     () const -> uint64_t         override;
    auto get_type_name() const -> std::string_view override;

    auto load                () -> bool;
    void show_in_graph_window(Graph_window* graph_window);

private:
    std::shared_ptr<sw::dfa::DomainFlowGraph>          m_dfg;
    std::map<std::size_t, std::shared_ptr<Graph_node>> m_ui_nodes;
};

class Project_explorer;

class Project_explorer_window : public Item_tree_window
{
public:
    Project_explorer_window(
        Project_explorer&            Project_explorer,
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            context,
        const std::string_view       window_title,
        const std::string_view       ini_label
    );

    void imgui() override;

private:
    Project_explorer& m_project_explorer;
    Explorer_context& m_explorer_context;
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

    void scan    ();
    void set_path(std::filesystem::path path);
    [[nodiscard]] auto get_path() const -> std::filesystem::path;

private:
    auto try_show  (Domain_flow_graph_file& dfg) -> bool;
    auto open_graph(const std::shared_ptr<Domain_flow_graph_file>& Domain_flow_graph_file) -> bool;

    void scan(const std::filesystem::path& path, const std::shared_ptr<Project_node>& parent);

    auto make_node    (const std::filesystem::path& path, const std::shared_ptr<Project_node>& parent) -> std::shared_ptr<Project_node>;
    auto item_callback(const std::shared_ptr<erhe::Item_base>& item) -> bool;

    std::filesystem::path                    m_root_path;
    Explorer_context&                        m_context;
    erhe::commands::Lambda_command           m_create_project_command;

    std::shared_ptr<Project_node>            m_root;
    std::shared_ptr<Project_explorer_window> m_node_tree_window;

    Project_node*                            m_popup_node{nullptr};
};

} // namespace explorer
