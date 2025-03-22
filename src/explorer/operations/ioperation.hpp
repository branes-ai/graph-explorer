#pragma once

#include "erhe_item/unique_id.hpp"

#include <string>

namespace explorer {

class Explorer_context;

class Operation
{
public:
    virtual ~Operation() noexcept;

    virtual void execute (Explorer_context& context) = 0;
    virtual void undo    (Explorer_context& context) = 0;
    virtual auto describe() const -> std::string = 0;

    [[nodiscard]] inline auto get_serial() const -> std::size_t { return m_id.get_id(); }

private:
    erhe::Unique_id<Operation> m_id{};
};

} // namespace explorer
