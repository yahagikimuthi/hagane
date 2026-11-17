#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class semaphore final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::semaphore* ptr) noexcept {
            vkDestroySemaphore(&dev_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    semaphore(raw::semaphore& raw, raw::device& dev) noexcept : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::semaphore& { return *raw_; }

    [[nodiscard]] operator raw::semaphore&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::semaphore, deleter> raw_;
};

class fence final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::fence* ptr) noexcept { vkDestroyFence(&dev_.get(), ptr, nullptr); }

        [[nodiscard]] auto device() noexcept -> raw::device& { return dev_; }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    fence(raw::fence& raw, raw::device& dev) noexcept : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::fence& { return *raw_; }

    [[nodiscard]] operator raw::fence&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& {
        return raw_.get_deleter().device();
    }

  private:
    std::unique_ptr<raw::fence, deleter> raw_;
};
}  // namespace hgn::raii