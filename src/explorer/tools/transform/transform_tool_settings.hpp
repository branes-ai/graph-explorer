#pragma once

namespace explorer {

class Transform_tool_settings
{
public:
    bool  cast_rays            {false};
    bool  show_translate       {true};
    bool  show_rotate          {false};
    bool  show_scale           {false};
    bool  hide_inactive        {true};
    bool  local                {false};
    bool  translate_snap_enable{false};
    float translate_snap       {0.1f};
    bool  rotate_snap_enable   {true};
    float rotate_snap          {15.0f};
};

} // namespace explorer
