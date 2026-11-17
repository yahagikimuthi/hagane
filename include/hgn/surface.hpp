#pragma once

#include <vulkan/vulkan.h>

#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class surface_khr final {
  public:
    surface_khr(raw::surface_khr& raw, raw::instance& instance) noexcept : raw_{raw, instance} {}

    [[nodiscard]] auto get() noexcept -> raw::surface_khr& { return raw_; }

  private:
    raii::surface_khr raw_;
};
}  // namespace hgn