#pragma once

#include "erhe_graph/node.hpp"

#include <imgui/imgui.h>

#include <vector>

namespace erhe::scene          { class Node; }
namespace erhe::scene_renderer { class Cube_instance_buffer; }
namespace ax::NodeEditor       { class EditorContext; }

namespace explorer {

class Explorer_context;
class Sheet;
class Graph;
class Graph_window;

class Node_edge
{
public:
    static constexpr int left   = 0;
    static constexpr int right  = 1;
    static constexpr int top    = 2;
    static constexpr int bottom = 3;
    static constexpr int count  = 4;
};

auto get_node_edge_name(int direction) -> const char*;

class Wavefront_frame
{
public:
    glm::vec4                                                   color_bias {0.0f, 0.0f, 0.0f, 0.0f};
    glm::vec4                                                   color_scale{1.0f, 1.0f, 1.0f, 1.0f};
    std::shared_ptr<erhe::scene_renderer::Cube_instance_buffer> cube_instance_buffer{};
    int                                                         time{0};
};

class Graph_node : public erhe::graph::Node
{
public:
    Graph_node(const std::string_view label, std::size_t payload);

    void make_input_pin (std::size_t key, std::string_view name);
    void make_output_pin(std::size_t key, std::string_view name);

    void node_editor(Explorer_context& context, ax::NodeEditor::EditorContext& node_editor, Graph_window& graph_window);

    [[nodiscard]] auto get_payload() const -> size_t;
    [[nodiscard]] auto get_convex_hull_visualization() -> std::shared_ptr<erhe::scene::Node>;
    [[nodiscard]] auto get_index_space_node() -> std::shared_ptr<erhe::scene::Node>;
    [[nodiscard]] auto wavefront_frames() -> std::vector<Wavefront_frame>&;
    void set_convex_hull_visualization(const std::shared_ptr<erhe::scene::Node>& node, const glm::vec3& index_space_offset);

    [[nodiscard]] auto get_wavefront_time_offset() const -> int;
    [[nodiscard]] auto show_wavefront() const -> bool;
    void set_wavefront_time_offset(int offset);
    void get_time_range(int& first, int& last) const;
    void set_earliest_max_times(glm::ivec3 earliest_times);
    [[nodiscard]] auto get_earliest_max_times() const -> glm::ivec3;

    virtual void imgui();

protected:
    friend class Node_properties_window;

    struct Node_context
    {
        Explorer_context&              context;
        ax::NodeEditor::EditorContext& node_editor;
        ImDrawList*                    draw_list      {nullptr};
        float                          pin_width;
        float                          pin_label_width;
        float                          side_width     {0.0f};
        float                          center_width;
        ImVec2                         pin_table_size {};
        ImVec2                         node_table_size{};
        ImFont*                        icon_font      {nullptr};
        int                            pin_edge       {0};
        float                          edge_x         {0.0f};
    };
    void show_pins(Node_context& context, std::vector<erhe::graph::Pin>& pins);

    void text_unformatted_edge(int edge, const char* text);

    static constexpr std::size_t pin_key_todo = 1;

    std::size_t                        m_payload;
    int                                m_input_pin_edge {Node_edge::left};
    int                                m_output_pin_edge{Node_edge::right};
    std::shared_ptr<erhe::scene::Node> m_convex_hull_visualization;
    std::shared_ptr<erhe::scene::Node> m_index_space_node;
    int                                m_wavefront_time_offset{};
    std::vector<Wavefront_frame>       m_wavefront_frames;
    glm::ivec3                         m_earliest_times{0, 0, 0};
    bool                               m_show_wavefront{true};
};

} // namespace explorer
