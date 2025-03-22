#pragma once

#include "explorer_message.hpp"

#include "erhe_message_bus/message_bus.hpp"

namespace explorer {

class Explorer_message_bus : public erhe::message_bus::Message_bus<Explorer_message>
{
public:
    Explorer_message_bus();
};

} // namespace explorer
