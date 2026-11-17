#pragma once

#include <cstdint>
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

class physical_device final {
  public:
    explicit physical_device(raw::physical_device&& raw) noexcept  // NOLINT
        : raw_{raw} {}

    [[nodiscard]] auto get() noexcept -> raw::physical_device& { return raw_; }
    [[nodiscard]] auto get() const noexcept -> raw::physical_device& { return raw_; }

    [[nodiscard]] operator raw::physical_device&() noexcept { return get(); }
    [[nodiscard]] operator raw::physical_device&() const noexcept { return get(); }

  private:
    std::reference_wrapper<raw::physical_device> raw_;
};

class queue_family final {
  public:
    queue_family(std::uint32_t index, raw::physical_device& physical) noexcept
        : index_{index}, physical_{std::ref(physical)} {}

    [[nodiscard]] auto index() const noexcept -> std::uint32_t { return index_; }

    [[nodiscard]] auto init_physical() noexcept -> raw::physical_device& { return physical_; }

  private:
    std::uint32_t                                index_;
    std::reference_wrapper<raw::physical_device> physical_;
};

class queue final {
  public:
    explicit queue(raw::queue&& raw) noexcept : raw_{raw} {}  // NOLINT

    [[nodiscard]] auto get() noexcept -> raw::queue& { return raw_; }

    [[nodiscard]] operator raw::queue&() noexcept { return get(); }

  private:
    std::reference_wrapper<raw::queue> raw_;
};
}  // namespace hgn::raii