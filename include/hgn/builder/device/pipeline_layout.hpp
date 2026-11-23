#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/descriptor.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/rendering.hpp"

namespace hgn {
class pipeline_layout_creator;
class push_constant_adder final {
  public:
    push_constant_adder(
        pipeline_layout_creator& creator, std::vector<VkPushConstantRange>& ref
    ) noexcept
        : creator_{std::ref(creator)}, ref_{std::ref(ref)} {}

    auto stage_flags(const shader_stage_flag_bits flag) noexcept -> push_constant_adder& {
        info_.stageFlags = static_cast<VkShaderStageFlagBits>(flag);
        return *this;
    }

    auto offset(const std::uint32_t n) noexcept -> push_constant_adder& {
        info_.offset = n;
        return *this;
    }

    auto size(const std::uint32_t n) noexcept -> push_constant_adder& {
        info_.size = n;
        return *this;
    }

    auto push() noexcept -> pipeline_layout_creator& {
        ref_.get().emplace_back(info_);
        return creator_;
    }

  private:
    VkPushConstantRange                                      info_{};
    std::reference_wrapper<pipeline_layout_creator>          creator_;
    std::reference_wrapper<std::vector<VkPushConstantRange>> ref_;
};

class pipeline_layout_creator final {
  public:
    explicit pipeline_layout_creator(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    }

    auto add_layout(descriptor_set_layout& layout) noexcept -> pipeline_layout_creator& {
        layouts_.emplace_back(&layout.get());
        return *this;
    }

    auto get_push_constant_adder() noexcept -> push_constant_adder {
        return {*this, push_constants_};
    }

    auto create() noexcept -> std::expected<pipeline_layout, result> {
        info_.setLayoutCount = static_cast<std::uint32_t>(layouts_.size());
        info_.pSetLayouts    = layouts_.data();

        info_.pushConstantRangeCount = static_cast<std::uint32_t>(push_constants_.size());
        info_.pPushConstantRanges    = push_constants_.data();

        auto* raw = static_cast<raw::pipeline_layout*>(nullptr);
        return to_expected(vkCreatePipelineLayout(&dev_.get(), &info_, nullptr, &raw))
            .transform([&]() noexcept -> pipeline_layout { return {*raw, dev_}; });
    }

  private:
    VkPipelineLayoutCreateInfo               info_{};
    std::vector<VkPushConstantRange>         push_constants_;
    std::vector<raw::descriptor_set_layout*> layouts_;
    std::reference_wrapper<raw::device>      dev_;
};
}  // namespace hgn