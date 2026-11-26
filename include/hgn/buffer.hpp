#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <span>
#include <utility>

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

    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : raw_{std::move(other.raw_)}, data_{other.data_}, current_ptr_{data_} {}

    [[nodiscard]] auto size() noexcept -> std::uint64_t { return info().size; }

    template <std::ranges::contiguous_range R>
    void cpy(const R& range) noexcept {
        assert(current_ptr_ != nullptr);

        const auto bytes = std::as_bytes(std::span{range});
        if (bytes.empty()) return;

        std::memcpy(current_ptr_, bytes.data(), bytes.size());
        current_ptr_ += bytes.size();
    }

    template <typename T>
    void cpy(const T& data) noexcept {
        std::memcpy(current_ptr_, &data, sizeof(T));
        current_ptr_ += sizeof(T);
    }

    void cpy(std::byte* data, std::size_t size) noexcept {
        std::memcpy(current_ptr_, data, size);
        current_ptr_ += size;
    }

    void reset() noexcept { current_ptr_ = data_; }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

  private:
    [[nodiscard]] auto info() noexcept -> VmaAllocationInfo {
        auto info = VmaAllocationInfo{};
        vmaGetAllocationInfo(&raw_.init_allocator(), &raw_.init_allocation(), &info);
        return info;
    }

    raii::buffer raw_;
    std::byte*   data_;
    std::byte*   current_ptr_{data_};
};
}  // namespace hgn