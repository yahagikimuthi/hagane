#pragma once

#include <functional>
#include <memory>

#include <hgn/third_parties/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class swapchain_khr final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::swapchain_khr* ptr) noexcept {
            vkDestroySwapchainKHR(&dev_.get(), ptr, nullptr);
        }

        [[nodiscard]] auto device() noexcept -> raw::device& { return dev_; }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    swapchain_khr(raw::swapchain_khr& raw, raw::device& dev) noexcept : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::swapchain_khr& { return *raw_; }

    [[nodiscard]] operator raw::swapchain_khr&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& {
        return raw_.get_deleter().device();
    }

  private:
    std::unique_ptr<raw::swapchain_khr, deleter> raw_;
};

class image final {
    class deleter final {
      public:
        deleter(raw::allocator& allocator, raw::allocation& allocation) noexcept
            : allocator_{std::ref(allocator)}, allocation_{std::ref(allocation)} {}

        void operator()(raw::image* ptr) noexcept {
            vmaDestroyImage(&allocator_.get(), ptr, &allocation_.get());
        }

      private:
        std::reference_wrapper<raw::allocator>  allocator_;
        std::reference_wrapper<raw::allocation> allocation_;
    };

  public:
    image(raw::image& raw, raw::allocator& allocator, raw::allocation& alloc) noexcept
        : raw_{&raw, {allocator, alloc}} {}

    [[nodiscard]] auto get() noexcept -> raw::image& { return *raw_; }

    [[nodiscard]] operator raw::image&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::image, deleter> raw_;
};

class image_view final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::image_view* ptr) noexcept {
            vkDestroyImageView(&dev_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    image_view(raw::image_view& raw, raw::device& dev) noexcept : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::image_view& { return *raw_; }

    [[nodiscard]] operator raw::image_view&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::image_view, deleter> raw_;
};

class sampler final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::sampler* ptr) noexcept { vkDestroySampler(&dev_.get(), ptr, nullptr); }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    sampler(raw::sampler& raw, raw::device& dev) noexcept : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::sampler& { return *raw_; }

    [[nodiscard]] operator raw::sampler&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::sampler, deleter> raw_;
};
}  // namespace hgn::raii