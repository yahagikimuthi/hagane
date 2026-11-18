#pragma once

#include <vulkan/vulkan.h>

#include "hgn/builder/allocator/buffer.hpp"
#include "hgn/builder/allocator/image.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class allocator final {
  public:
    explicit allocator(raw::allocator& raw) noexcept : raw_{raw} {}

    [[nodiscard]] auto get_buffer_creator() noexcept -> buffer_creator {
        return buffer_creator{get()};
    }

    [[nodiscard]] auto get_image_creator() noexcept -> image_creator {
        return image_creator{get()};
    }

    [[nodiscard]] auto get() noexcept -> raw::allocator& { return raw_; }

  private:
    raii::allocator raw_;
};
}  // namespace hgn