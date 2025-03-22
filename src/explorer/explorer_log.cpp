#include "explorer_log.hpp"
#include "erhe_log/log.hpp"

namespace explorer {

std::shared_ptr<spdlog::logger> log_asset_browser;
std::shared_ptr<spdlog::logger> log_brush;
std::shared_ptr<spdlog::logger> log_composer;
std::shared_ptr<spdlog::logger> log_controller_ray;
std::shared_ptr<spdlog::logger> log_debug_visualization;
std::shared_ptr<spdlog::logger> log_draw;
std::shared_ptr<spdlog::logger> log_fly_camera;
std::shared_ptr<spdlog::logger> log_framebuffer;
std::shared_ptr<spdlog::logger> log_headset;
std::shared_ptr<spdlog::logger> log_hud;
std::shared_ptr<spdlog::logger> log_id_render;
std::shared_ptr<spdlog::logger> log_input;
std::shared_ptr<spdlog::logger> log_input_frame;
std::shared_ptr<spdlog::logger> log_materials;
std::shared_ptr<spdlog::logger> log_net;
std::shared_ptr<spdlog::logger> log_node_properties;
std::shared_ptr<spdlog::logger> log_operations;
std::shared_ptr<spdlog::logger> log_parsers;
std::shared_ptr<spdlog::logger> log_physics;
std::shared_ptr<spdlog::logger> log_physics_frame;
std::shared_ptr<spdlog::logger> log_pointer;
std::shared_ptr<spdlog::logger> log_post_processing;
std::shared_ptr<spdlog::logger> log_programs;
std::shared_ptr<spdlog::logger> log_raytrace;
std::shared_ptr<spdlog::logger> log_raytrace_frame;
std::shared_ptr<spdlog::logger> log_render;
std::shared_ptr<spdlog::logger> log_rendertarget_imgui_windows;
std::shared_ptr<spdlog::logger> log_scene;
std::shared_ptr<spdlog::logger> log_scene_view;
std::shared_ptr<spdlog::logger> log_selection;
std::shared_ptr<spdlog::logger> log_startup;
std::shared_ptr<spdlog::logger> log_svg;
std::shared_ptr<spdlog::logger> log_textures;
std::shared_ptr<spdlog::logger> log_tools;
std::shared_ptr<spdlog::logger> log_tree;
std::shared_ptr<spdlog::logger> log_tree_frame;
std::shared_ptr<spdlog::logger> log_trs_tool;
std::shared_ptr<spdlog::logger> log_trs_tool_frame;
std::shared_ptr<spdlog::logger> log_xr;
std::shared_ptr<spdlog::logger> log_graph_window;

void initialize_logging()
{
    using namespace erhe::log;
    log_asset_browser              = make_logger      ("explorer.asset_browser"             );
    log_startup                    = make_logger      ("explorer.startup"                   );
    log_brush                      = make_logger      ("explorer.brush"                     );
    log_composer                   = make_frame_logger("explorer.composer"                  );
    log_controller_ray             = make_frame_logger("explorer.controller_ray"            );
    log_debug_visualization        = make_frame_logger("explorer.debug_visualization"       );
    log_draw                       = make_logger      ("explorer.draw"                      );
    log_fly_camera                 = make_logger      ("explorer.fly_camera"                );
    log_input_frame                = make_frame_logger("explorer.input_frame"               );
    log_framebuffer                = make_frame_logger("explorer.framebuffer"               );
    log_headset                    = make_logger      ("explorer.headset"                   );
    log_hud                        = make_logger      ("explorer.hud"                       );
    log_id_render                  = make_logger      ("explorer.id_render"                 );
    log_materials                  = make_logger      ("explorer.materials"                 );
    log_net                        = make_logger      ("explorer.net"                       );
    log_node_properties            = make_logger      ("explorer.node_properties"           );
    log_operations                 = make_logger      ("explorer.operations"                );
    log_parsers                    = make_logger      ("explorer.parsers"                   );
    log_physics                    = make_logger      ("explorer.physics"                   );
    log_physics_frame              = make_frame_logger("explorer.physics_frame"             );
    log_pointer                    = make_logger      ("explorer.pointer"                   );
    log_post_processing            = make_frame_logger("explorer.post_processing"           );
    log_programs                   = make_logger      ("explorer.programs"                  );
    log_raytrace                   = make_logger      ("explorer.raytrace"                  );
    log_raytrace_frame             = make_frame_logger("explorer.raytrace_frame"            );
    log_render                     = make_frame_logger("explorer.render"                    );
    log_rendertarget_imgui_windows = make_frame_logger("explorer.rendertarget_imgui_windows");
    log_input                      = make_logger      ("explorer.input"                     );
    log_scene                      = make_logger      ("explorer.scene"                     );
    log_scene_view                 = make_frame_logger("explorer.scene_view"                );
    log_selection                  = make_logger      ("explorer.selection"                 );
    log_svg                        = make_logger      ("explorer.svg"                       );
    log_textures                   = make_logger      ("explorer.textures"                  );
    log_tools                      = make_logger      ("explorer.tools"                     );
    log_trs_tool                   = make_logger      ("explorer.transform_tool"            );
    log_trs_tool_frame             = make_frame_logger("explorer.transform_tool_frame"      );
    log_xr                         = make_logger      ("explorer.xr"                        );
    log_tree                       = make_logger      ("explorer.tree"                      );
    log_tree_frame                 = make_frame_logger("explorer.tree_frame"                );
    log_graph_window               = make_frame_logger("explorer.graph_window"              );
}

}
