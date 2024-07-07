#pragma once

#include "operations/ioperation.hpp"

#include "erhe_item/item.hpp"

#include <memory>
#include <vector>

namespace erhe {
    class Hierarchy;
}

namespace editor {

class Item_reposition_in_parent_operation : public IOperation
{
public:
    Item_reposition_in_parent_operation();
    Item_reposition_in_parent_operation(
        const std::shared_ptr<erhe::Hierarchy>& child,
        const std::shared_ptr<erhe::Hierarchy>  place_before,
        const std::shared_ptr<erhe::Hierarchy>  palce_after
    );

    // Implements IOperation
    auto describe() const -> std::string override;
    void execute(Editor_context& context) override;
    void undo   (Editor_context& context) override;

private:
    std::shared_ptr<erhe::Hierarchy> m_child;
    std::size_t                      m_before_index{0};
    std::shared_ptr<erhe::Hierarchy> m_place_before{};
    std::shared_ptr<erhe::Hierarchy> m_place_after {};
};

}
