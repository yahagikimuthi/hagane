#pragma once

#include <functional>
#include <memory>

#include <hgn/third_parties/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class allocator final {
  public:
    explicit allocator(raw::allocator& raw) noexcept : raw_{&raw, vmaDestroyAllocator} {}

    [[nodiscard]] auto get() noexcept -> raw::allocator& { return *raw_; }

    [[nodiscard]] operator raw::allocator&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::allocator, void (*)(raw::allocator*)> raw_;
};

class command_pool final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept;

        void operator()(raw::command_pool* ptr) noexcept {
            vkDestroyCommandPool(&dev_.get(), ptr, nullptr);
        }

        [[nodiscard]] auto device() noexcept -> raw::device& { return dev_; }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    command_pool(raw::command_pool& raw, raw::device& dev) noexcept  // NOLINT
        : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::command_pool& { return *raw_; }

    [[nodiscard]] operator raw::command_pool&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& {
        return raw_.get_deleter().device();
    }

  private:
    std::unique_ptr<raw::command_pool, deleter> raw_;
};
}  // namespace hgn::raii