#include "windows/project_explorer.hpp"

#include "explorer_context.hpp"
#include "explorer_log.hpp"
#include "graph/graph_window.hpp"
#include "graph/graph_node.hpp"
#include "windows/item_tree_window.hpp"

#include "erhe_commands/commands.hpp"
#include "erhe_file/file.hpp"
#include "erhe_imgui/imgui_windows.hpp"
#include "erhe_imgui/imgui_node_editor.h"

#include <dfa/dfa.hpp>
#include <util/data_file.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#if defined(ERHE_WINDOW_LIBRARY_SDL)
# include <SDL3/SDL_dialog.h>
#endif

namespace explorer {


//
Project_node::Project_node(const Project_node&)            = default;
Project_node& Project_node::operator=(const Project_node&) = default;
Project_node::~Project_node() noexcept                     = default;

Project_node::Project_node(const std::filesystem::path& path) 
    : Item{erhe::file::to_string(path.filename())}
{
    set_source_path(path);
}


auto Project_folder::get_static_type()       -> uint64_t        { return erhe::Item_type::asset_folder; }
auto Project_folder::get_type       () const -> uint64_t        { return get_static_type(); }
auto Project_folder::get_type_name  () const -> std::string_view{ return static_type_name; }
Project_folder::Project_folder(const Project_folder&)            = default;
Project_folder& Project_folder::operator=(const Project_folder&) = default;
Project_folder::~Project_folder() noexcept                     = default;
Project_folder::Project_folder(const std::filesystem::path& path) : Item{path} {}

auto Domain_flow_graph_file::get_static_type()       -> uint64_t        { return erhe::Item_type::asset_file_other; }
auto Domain_flow_graph_file::get_type       () const -> uint64_t        { return get_static_type(); }
auto Domain_flow_graph_file::get_type_name  () const -> std::string_view{ return static_type_name; }
Domain_flow_graph_file::Domain_flow_graph_file(const Domain_flow_graph_file&)            = default;
Domain_flow_graph_file& Domain_flow_graph_file::operator=(const Domain_flow_graph_file&) = default;
Domain_flow_graph_file::~Domain_flow_graph_file() noexcept                     = default;
Domain_flow_graph_file::Domain_flow_graph_file(const std::filesystem::path& path) : Item{path} {}

auto Project_explorer::make_node(const std::filesystem::path& path, const std::shared_ptr<Project_node>& parent) -> std::shared_ptr<Project_node>
{
    std::error_code error_code;
    bool is_directory{false};
    const bool is_directory_test = std::filesystem::is_directory(path, error_code);
    if (!error_code) {
        is_directory = is_directory_test;
    }

    const bool is_dfg = path.extension() == std::filesystem::path{".dfg"};

    std::shared_ptr<Project_node> new_node{};
    if (is_directory) {
        new_node = std::make_shared<Project_folder>(path);
    } else if (is_dfg) {
        new_node = std::make_shared<Domain_flow_graph_file>(path);
    }
    // TODO Handle more file types

    if (new_node) {
        new_node->enable_flag_bits(erhe::Item_flags::visible);
        if (parent) {
            new_node->set_parent(parent);
        }
    }
    return new_node;
}

Project_explorer_window::Project_explorer_window(
    Project_explorer&            project_explorer,
    erhe::imgui::Imgui_renderer& imgui_renderer,
    erhe::imgui::Imgui_windows&  imgui_windows,
    Explorer_context&            context,
    const std::string_view       window_title,
    const std::string_view       ini_label
)
    : Item_tree_window  {imgui_renderer, imgui_windows, context, window_title, ini_label}
    , m_project_explorer{project_explorer}
    , m_explorer_context{context}
{
}

#if defined(ERHE_WINDOW_LIBRARY_SDL)
static void s_select_folder_callback(void* userdata, const char* const* filelist, int filter)
{
    static_cast<void>(filter);
    if (userdata == nullptr) {
        return;
    }
    Project_explorer* project_explorer = static_cast<Project_explorer*>(userdata);
    if (filelist == nullptr) {
        return; // error TODO SDL_GetError();
    }
    if (*filelist == nullptr) {
        return; // canceled / nothing selected
    }
    project_explorer->set_path(*filelist);
    project_explorer->scan();
}
#endif

void Project_explorer_window::imgui()
{
#if defined(ERHE_WINDOW_LIBRARY_SDL)
    if (ImGui::Button("Select Folder:")) {
        SDL_Window* window = static_cast<SDL_Window*>(m_explorer_context.context_window->get_sdl_window());
        SDL_ShowOpenFolderDialog(s_select_folder_callback, &m_project_explorer, window, nullptr, false);
    }
    ImGui::SameLine();
    ImGui::InvisibleButton("##space", ImVec2{5.0f, 5.0f});
    ImGui::SameLine();
    std::string path;
    try {
        path = m_project_explorer.get_path().string();
    } catch (...) {
    }
    ImGui::TextUnformatted(path.c_str());
#endif
    if (ImGui::Button("Scan")) {
        m_project_explorer.scan();
    }
    Item_tree_window::imgui();
}

Project_explorer::Project_explorer(
    erhe::commands::Commands&    commands,
    erhe::imgui::Imgui_renderer& imgui_renderer,
    erhe::imgui::Imgui_windows&  imgui_windows,
    Explorer_context&            explorer_context
)
    : m_context               {explorer_context}
    , m_create_project_command{commands, "File.Create Project", [this]() -> bool { create_project(); return true; } }
    , m_root_path             {std::filesystem::path("../../data")}
{
    scan();

    m_node_tree_window = std::make_shared<Project_explorer_window>(
        *this,
        imgui_renderer, 
        imgui_windows, 
        explorer_context,
        "Project Explorer",
        "project_explorer"
    );
    m_node_tree_window->set_item_filter(
        erhe::Item_filter{
            .require_all_bits_set           = 0,
            .require_at_least_one_bit_set   = 0,
            .require_all_bits_clear         = 0,
            .require_at_least_one_bit_clear = 0
        }
    );
    m_node_tree_window->set_item_callback(
        [&](const std::shared_ptr<erhe::Item_base>& item) -> bool {
            return item_callback(item);
        }
    );
    m_node_tree_window->set_root(m_root);

    commands.register_command(&m_create_project_command);

    commands.bind_command_to_menu(&m_create_project_command, "File. Create Project");
}

void Project_explorer::create_project()
{
    // TODO
}

void Project_explorer::scan(const std::filesystem::path& path, const std::shared_ptr<Project_node>& parent)
{
    log_project_explorer->trace("Scanning {}", erhe::file::to_string(path));

    std::error_code error_code;
    auto directory_iterator = std::filesystem::directory_iterator{path, error_code};
    if (error_code) {
        log_project_explorer->warn(
            "Scanning {}: directory_iterator() failed with error {} - {}",
            erhe::file::to_string(path), error_code.value(), error_code.message()
        );
        return;
    }
    for (const auto& entry : directory_iterator) {
        const bool is_directory = std::filesystem::is_directory(entry, error_code);
        if (error_code) {
            log_project_explorer->warn(
                "Scanning {}: is_directory() failed with error {} - {}",
                erhe::file::to_string(path), error_code.value(), error_code.message()
            );
            continue;
        }

        const bool is_regular_file = std::filesystem::is_regular_file(entry, error_code);
        if (error_code) {
            log_project_explorer->warn(
                "Scanning {}: is_regular_file() failed with error {} - {}",
                erhe::file::to_string(path), error_code.value(), error_code.message()
            );
            continue;
        }
        if (!is_directory && !is_regular_file) {
            log_project_explorer->warn(
                "Scanning {}: is neither regular file nor directory",
                erhe::file::to_string(path)
            );
            continue;
        }

        std::shared_ptr<Project_node> project_node = make_node(entry, parent);
        if (is_directory) {
            scan(entry, project_node);
        }
    }
}

void Project_explorer::set_path(std::filesystem::path path)
{
    m_root_path = path;
    scan();
    m_node_tree_window->set_root(m_root);
}

auto Project_explorer::get_path() const -> std::filesystem::path
{
    return m_root_path;
}

void Project_explorer::scan()
{
    m_root = make_node(m_root_path, nullptr);
    scan(m_root_path, m_root);
}

auto Domain_flow_graph_file::load() -> bool
{
    using namespace sw::dfa;

    try {
        std::string file_name = erhe::file::to_string(get_source_path());
        m_dfg = std::make_shared<DomainFlowGraph>(file_name);
        m_dfg->graph.load(file_name);
        return true;
    } catch (...) {
        return false;
    }
}

void Domain_flow_graph_file::show_in_graph_window(Graph_window* graph_window)
{
    using namespace sw::dfa;

    erhe::graph::Graph&            ui_graph    = graph_window->get_ui_graph();
    ax::NodeEditor::EditorContext* node_editor = graph_window->get_node_editor();

    ui_graph.clear();
    node_editor->ClearSelection(); // TODO graph_window.clear()

    for (auto i : m_dfg->graph.nodes()) {
        const std::size_t     node_id = i.first;
        const DomainFlowNode& node    = i.second;

        std::shared_ptr<Graph_node> ui_node = std::make_shared<Graph_node>(node.getName());
        constexpr uint64_t flags = erhe::Item_flags::visible | erhe::Item_flags::content | erhe::Item_flags::show_in_ui;
        ui_node->enable_flag_bits(flags);

        m_ui_nodes.insert({node_id, ui_node});

        for (std::size_t j = 0, end = node.getNrInputs(); j < end; ++j) {
            ui_node->make_input_pin(0, node.operandType.at(j));
        }
        for (std::size_t j = 0, end = node.getNrOutputs(); j < end; ++j) {
            ui_node->make_output_pin(0, node.resultType.at(j));
        }
        ui_graph.register_node(ui_node.get());
    }

    //for (auto i : m_dfg->graph.edges()) {
    //    const std::shared_ptr<Graph_node>& from = m_ui_nodes.at(i.first.first);
    //    const std::shared_ptr<Graph_node>& to   = m_ui_nodes.at(i.first.second);
    //    const DomainFlowEdge& edge = i.second;
    //}

}

//void Domain_flow_graph_file::make_link(Graph_node* from, Graph_node* to)
//{
//    erhe::graph::Pin*  from_pin = &(from->get_input_pins().at(0));
//    erhe::graph::Pin*  to_pin   = &(to  ->get_input_pins().at(0));
//    erhe::graph::Link* link     = m_graph.connect(from_pin, to_pin);
//}

auto Project_explorer::try_show(Domain_flow_graph_file& dfg) -> bool
{
    std::string import_label = fmt::format("Show'{}'", erhe::file::to_string(dfg.get_source_path()));
    if (ImGui::MenuItem(import_label.c_str())) {
        dfg.load();
        dfg.show_in_graph_window(m_context.graph_window);
        ImGui::CloseCurrentPopup();
        return true;
    }
    return false;
}

auto Project_explorer::item_callback(const std::shared_ptr<erhe::Item_base>& item) -> bool
{
    const auto domain_flow_graph_file = std::dynamic_pointer_cast<Domain_flow_graph_file>(item);
    if (domain_flow_graph_file) {

        const ImGuiID popup_id{ImGui::GetID("project_explorer_node_popup")};

        if (
            ImGui::IsMouseReleased(ImGuiMouseButton_Right) &&
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
            m_popup_node == nullptr
        ) {
            m_popup_node = domain_flow_graph_file.get();
            ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_MouseButtonRight);
        }

        if (m_popup_node == domain_flow_graph_file.get()) {
            if (ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None)) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
                const bool begin_popup_context_item = ImGui::BeginPopupEx(
                    popup_id,
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings
                );
                if (begin_popup_context_item) {
                    if (try_show(*domain_flow_graph_file.get())) {
                        m_popup_node = nullptr;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
            } else {
                m_popup_node = nullptr;
            }
            return true;
        }
    }
    return false;
}

} // namespace explorer
