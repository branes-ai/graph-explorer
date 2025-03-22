#pragma once

#include "erhe_imgui/imgui_window.hpp"
#include "erhe_net/client.hpp"
#include "erhe_net/server.hpp"

#include <vector>

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

class Network_window
    : public erhe::imgui::Imgui_window
{
public:
    Network_window(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context
    );

    // Implements Imgui_window
    void imgui() override;

    void update_network();

private:

    Explorer_context&          m_context;
    erhe::net::Client        m_client;
    erhe::net::Server        m_server;

    // Network client
    std::string              m_upstream_address;
    int                      m_upstream_port{0};
    std::vector<std::string> m_upstream_messages;

    // Network server
    std::string              m_downstream_address;
    int                      m_downstream_port{0};
    std::vector<std::string> m_downstream_messages;
};

} // namespace explorer

