#include "operations/compound_operation.hpp"

#include "explorer_log.hpp"

#include <sstream>

namespace explorer {

Compound_operation::Compound_operation(Parameters&& parameters)
    : m_parameters{std::move(parameters)}
{
}

Compound_operation::~Compound_operation() noexcept
{
}

void Compound_operation::execute(Explorer_context& context)
{
    log_operations->trace("Op Execute Begin {}", describe());

    for (auto& operation : m_parameters.operations) {
        operation->execute(context);
    }

    log_operations->trace("Op Execute End {}", describe());
}

void Compound_operation::undo(Explorer_context& context)
{
    log_operations->trace("Op Undo Begin {}", describe());

    for (auto i = rbegin(m_parameters.operations), end = rend(m_parameters.operations); i < end; ++i) {
        auto& operation = *i;
        operation->undo(context);
    }

    log_operations->trace("Op Undo End {}", describe());
}

auto Compound_operation::describe() const -> std::string
{
    std::stringstream ss;
    ss << fmt::format("[{}] Compound ", get_serial());
    bool first = true;
    for (auto& operation : m_parameters.operations) {
        if (first) {
            first = false;
        } else {
            ss << ", ";
        }
        ss << operation->describe();
    }
    return ss.str();
}

}
