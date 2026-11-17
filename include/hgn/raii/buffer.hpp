#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class buffer final {
    class deleter final {
      public:
        deleter(raw::allocator& allocator, raw::allocation& allocation) noexcept
            : allocator_{std::ref(allocator)}, allocation_{std::ref(allocation)} {}

        void operator()(raw::buffer* ptr) noexcept {
            vmaDestroyBuffer(&allocator_.get(), ptr, &allocation_.get());
        }

        [[nodiscard]] auto allocator() noexcept -> raw::allocator& { return allocator_; }
        [[nodiscard]] auto allocation() noexcept -> raw::allocation& { return allocation_; }

      private:
        std::reference_wrapper<raw::allocator>  allocator_;
        std::reference_wrapper<raw::allocation> allocation_;
    };

  public:
    buffer(raw::buffer& native, raw::allocator& allocator, raw::allocation& alloc) noexcept
        : raw_{&native, {allocator, alloc}} {}

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return *raw_; }

    [[nodiscard]] operator raw::buffer&() noexcept { return get(); }

    [[nodiscard]] auto init_allocator() noexcept -> raw::allocator& {
        return raw_.get_deleter().allocator();
    }
    [[nodiscard]] auto init_allocation() noexcept -> raw::allocation& {
        return raw_.get_deleter().allocation();
    }

  private:
    std::unique_ptr<raw::buffer, deleter> raw_;
};
}  // namespace hgn::raii