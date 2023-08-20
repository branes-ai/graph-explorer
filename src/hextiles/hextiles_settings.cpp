#include "hextiles_settings.hpp"
#include "erhe/configuration/configuration.hpp"

#include "erhe/toolkit/window.hpp"

namespace hextiles {

Hextiles_settings::Hextiles_settings(
    erhe::toolkit::Context_window& context_window
)
{
    using namespace erhe::configuration;
    mINI::INIFile file{"settings.ini"};
    mINI::INIStructure ini;
    if (file.read(ini)) {
        if (ini.has("imgui")) {
            const auto& section = ini["imgui"];
            ini_get(section, "primary_font", imgui.primary_font);
            ini_get(section, "mono_font",    imgui.mono_font);
            ini_get(section, "font_size",    imgui.font_size);
            ini_get(section, "vr_font_size", imgui.vr_font_size);
        }
    }

    imgui.font_size *= context_window.get_scale_factor();
}

}
