#pragma once

#include <expected>

#include "hgn/descriptor.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_pool final {
  public:
    descriptor_pool(raw::descriptor_pool& raw, raw::device& dev) noexcept : raw_{raw, dev} {}

    [[nodiscard]] auto get() noexcept -> raw::descriptor_pool& { return raw_; }

    [[nodiscard]] auto get_descriptor_set(descriptor_set_layout& layout
    ) noexcept -> std::expected<descriptor_set, result> {
        auto info           = VkDescriptorSetAllocateInfo{};
        info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = &get();

        auto* ptr               = &layout.get();
        info.descriptorSetCount = 1;
        info.pSetLayouts        = &ptr;

        auto* raw = static_cast<raw::descriptor_set*>(nullptr);
        return to_expected(vkAllocateDescriptorSets(&raw_.init_device(), &info, &raw))
            .transform([&]() noexcept -> descriptor_set {
                return descriptor_set{*raw, raw_.init_device()};
            });
    }

  private:
    raii::descriptor_pool raw_;
};
}  // namespace hgn