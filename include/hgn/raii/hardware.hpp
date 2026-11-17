#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class instance final {
    class deleter final {
      public:
        void operator()(raw::instance* ptr) noexcept { vkDestroyInstance(ptr, nullptr); }
    };

  public:
    explicit instance(raw::instance& raw) noexcept : raw_{&raw} {}

    [[nodiscard]] auto get() noexcept -> raw::instance& { return *raw_; }

    [[nodiscard]] operator raw::instance&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::instance, deleter> raw_;
};

class surface_khr final {
    class deleter final {
      public:
        explicit deleter(raw::instance& instance) noexcept : instance_{std::ref(instance)} {}

        void operator()(raw::surface_khr* ptr) noexcept {
            vkDestroySurfaceKHR(&instance_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::instance> instance_;
    };

  public:
    surface_khr(raw::surface_khr& raw, raw::instance& instance) noexcept
        : raw_{&raw, deleter{instance}} {}

    [[nodiscard]] auto get() noexcept -> raw::surface_khr& { return *raw_; }

    [[nodiscard]] operator raw::surface_khr&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::surface_khr, deleter> raw_;
};

class device final {
    class deleter final {
      public:
        void operator()(raw::device* ptr) noexcept { vkDestroyDevice(ptr, nullptr); }
    };

  public:
    explicit device(raw::device& raw) noexcept : raw_{&raw} {}

    [[nodiscard]] auto get() noexcept -> raw::device& { return *raw_; }

    [[nodiscard]] operator raw::device&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::device, deleter> raw_;
};
}  // namespace hgn::raii