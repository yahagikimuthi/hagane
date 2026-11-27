#pragma once

#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class image final {
  public:
    image(
        raw::image&              raw,
        raw::allocator&          allocator,
        raw::allocation&         alloc,
        const VkImageCreateInfo& info
    ) noexcept
        : raw_{raw, allocator, alloc}, info_{info} {}

    [[nodiscard]] auto format() const noexcept -> format {
        return static_cast<enum format>(info_.format);
    }

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

  private:
    raii::image       raw_;
    VkImageCreateInfo info_;
};

class borrowed_image final {
  public:
    explicit borrowed_image(raw::image& raw) noexcept : raw_{std::ref(raw)} {}

    borrowed_image(image& img) noexcept : raw_{std::ref(img.get())} {}

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

  private:
    std::reference_wrapper<raw::image> raw_;
};

class image_view final {
  public:
    image_view(raw::image_view& raw, raw::device& dev, const VkImageViewCreateInfo& info) noexcept
        : raw_{raw, dev}, info_{info} {}

    [[nodiscard]] auto format() const noexcept -> format {
        return static_cast<enum format>(info_.format);
    }

    [[nodiscard]] auto get() noexcept -> raw::image_view& { return raw_; }

  private:
    raii::image_view      raw_;
    VkImageViewCreateInfo info_;
};

class sampler final {
  public:
    sampler(raw::sampler& raw, raw::device& dev) noexcept : raw_{raw, dev} {}

    [[nodiscard]] auto get() noexcept -> raw::sampler& { return raw_; }

  private:
    raii::sampler raw_;
};
}  // namespace hgn