#pragma once

namespace explorer {

class Render_context;

class Renderable
{
public:
    virtual ~Renderable() noexcept = default;

    virtual void render(const Render_context& context) = 0;
};

} // namespace explorer
