#pragma once

#include <cstdint>
#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class image_creator final {
  public:
    explicit image_creator(raw::allocator& allocator) noexcept : allocator_{std::ref(allocator)} {
        info_.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info_.imageType     = VK_IMAGE_TYPE_2D;
        info_.extent.depth  = 1;
        info_.mipLevels     = 1;
        info_.arrayLayers   = 1;
        info_.tiling        = VK_IMAGE_TILING_OPTIMAL;
        info_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info_.samples       = VK_SAMPLE_COUNT_1_BIT;
        info_.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    }

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept -> image_creator& {
        info_.extent.width  = width;
        info_.extent.height = height;
        return *this;
    }

    auto format(const format fmt) noexcept -> image_creator& {
        info_.format = static_cast<VkFormat>(fmt);
        return *this;
    }

    auto usage(const image_usage_flag_bits usage) noexcept -> image_creator& {
        info_.usage = static_cast<VkImageUsageFlagBits>(usage);
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<image, result> {
        auto image_alloc_info  = VmaAllocationCreateInfo{};
        image_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        auto* allocation = static_cast<raw::allocation*>(nullptr);
        auto* raw        = static_cast<raw::image*>(nullptr);

        return to_expected(
                   vmaCreateImage(
                       &allocator_.get(), &info_, &image_alloc_info, &raw, &allocation, nullptr
                   )
        )
            .transform([&]() noexcept -> image {
                return image{
                    *raw, allocator_.get(), *allocation, static_cast<enum format>(info_.format)
                };
            });
    }

  private:
    VkImageCreateInfo                      info_{};  // NOLINT
    std::reference_wrapper<raw::allocator> allocator_;
};
}  // namespace hgn