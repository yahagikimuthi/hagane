#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ranges>

#include <hgn/third_parties/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "hgn/raii/buffer.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class buffer final {
  public:
    buffer(
        raw::buffer& raw, raw::allocator& allocator, raw::allocation& alloc, std::byte* data
    ) noexcept
        : raw_{raw, allocator, alloc}, data_{data} {}

    [[nodiscard]] auto size() noexcept -> std::uint64_t { return info().size; }

    template <std::ranges::contiguous_range R>
    void cpy(const R& range) noexcept {
        assert(data_ != nullptr);
        std::memcpy(
            data_,
            std::ranges::data(range),
            sizeof(std::ranges::range_value_t<R>) * std::ranges::size(range)
        );
    }

    template <typename T>
    void cpy(const T& data) noexcept {
        std::memcpy(data_, &data, sizeof(T));
    }

    void cpy(std::byte* data, std::size_t size) noexcept { std::memcpy(data_, data, size); }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

  private:
    [[nodiscard]] auto info() noexcept -> VmaAllocationInfo {
        auto info = VmaAllocationInfo{};
        vmaGetAllocationInfo(&raw_.init_allocator(), &raw_.init_allocation(), &info);
        return info;
    }

    raii::buffer raw_;
    std::byte*   data_;
};
}  // namespace hgn