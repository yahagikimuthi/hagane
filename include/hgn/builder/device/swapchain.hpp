#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <functional>
#include <limits>
#include <ranges>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/queue.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/swapchain.hpp"

namespace hgn {
class swapchain_khr_creator final {
  public:
    swapchain_khr_creator(
        raw::physical_device& physical, raw::device& dev, raw::surface_khr& surface
    ) noexcept
        : physical_{std::ref(physical)}, dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        // 描画先としてメモリを確保する場合は以下の定数を設定
        // 他にもコピー先/コピー元として扱う、読み書き用などがある
        info_.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        info_.imageArrayLayers = 1;  // 各画像の層数. 立体視やVRを除き通常は1

        // アルファチャンネル(通常は不透明値のこの定数を設定)
        info_.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        info_.clipped = VK_TRUE;  // ウィンドウに隠れたピクセルを省略するかを設定
        info_.oldSwapchain = nullptr;  // リサイズ時などに古いswapchainを破棄, 再利用するため設定

        info_.surface = &surface;

        auto capabilities = get_capabilities();
        auto image_count  = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 and image_count > capabilities.maxImageCount)
            image_count = capabilities.maxImageCount;

        info_.minImageCount = image_count;
    }

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept
        -> swapchain_khr_creator& {
        auto capabilities = get_capabilities();
        auto extent       = capabilities.currentExtent;
        if (extent.width == std::numeric_limits<std::uint32_t>::max()) {
            extent.width = std::clamp(
                width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width
            );
            extent.height = std::clamp(
                height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height
            );
        }
        info_.imageExtent = extent;
        return *this;
    }

    auto surface_format(const format fmt, const color_space_khr color_space) noexcept
        -> swapchain_khr_creator& {
        auto formats = get_formats();
        if (formats.empty()) return *this;

        auto aligned =
            formats | std::views::filter([&](VkSurfaceFormatKHR& surface_fmt) noexcept -> bool {
                return surface_fmt.format == static_cast<VkFormat>(fmt) and
                       surface_fmt.colorSpace == static_cast<VkColorSpaceKHR>(color_space);
            });

        auto set              = aligned.empty() ? formats.front() : aligned.front();
        info_.imageFormat     = set.format;
        info_.imageColorSpace = set.colorSpace;
        return *this;
    }

    auto present_mode(const present_mode_khr desired) noexcept -> swapchain_khr_creator& {
        auto modes = get_present_modes();
        if (modes.empty()) return *this;

        auto aligned = modes | std::views::filter([&](VkPresentModeKHR mode) -> bool {
                           return mode == static_cast<VkPresentModeKHR>(desired);
                       });
        auto set     = aligned.empty() ? VK_PRESENT_MODE_FIFO_KHR : aligned.front();

        info_.presentMode = set;

        return *this;
    }

    auto queue_families(const queue_family& graphics, const queue_family& present) noexcept
        -> swapchain_khr_creator& {
        queue_family_indices_[0] = graphics.index();
        queue_family_indices_[1] = present.index();

        if (graphics == present) {
            info_.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        } else {
            info_.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        }

        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<swapchain_khr, result> {
        auto capabilities = VkSurfaceCapabilitiesKHR{};  // NOLINT
        if (auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                &physical_.get(), info_.surface, &capabilities
            );
            res != VK_SUCCESS)
            return make_unexpected(res);

        info_.preTransform = capabilities.currentTransform;
        auto count         = std::uint32_t{};
        if (auto res = vkGetPhysicalDeviceSurfaceFormatsKHR(
                &physical_.get(), info_.surface, &count, nullptr
            );
            res != VK_SUCCESS)
            return make_unexpected(res);
        count = 0;

        if (auto res = vkGetPhysicalDeviceSurfacePresentModesKHR(
                &physical_.get(), info_.surface, &count, nullptr
            );
            res != VK_SUCCESS)
            return make_unexpected(res);

        if (info_.imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
            info_.queueFamilyIndexCount = 2;
            info_.pQueueFamilyIndices   = queue_family_indices_.data();
        }

        auto* raw = static_cast<raw::swapchain_khr*>(nullptr);
        return make_expected(vkCreateSwapchainKHR(&dev_.get(), &info_, nullptr, &raw))
            .transform([&]() noexcept -> swapchain_khr {
                return swapchain_khr{*raw, dev_.get(), static_cast<format>(info_.imageFormat)};
            });
    }

  private:
    [[nodiscard]] auto get_capabilities() noexcept -> VkSurfaceCapabilitiesKHR {
        auto capabilities = VkSurfaceCapabilitiesKHR{};  // NOLINT
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&physical_.get(), info_.surface, &capabilities);
        return capabilities;
    }

    [[nodiscard]] auto get_formats() noexcept -> std::vector<VkSurfaceFormatKHR> {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(&physical_.get(), info_.surface, &count, nullptr);

        auto out = std::vector<VkSurfaceFormatKHR>(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(&physical_.get(), info_.surface, &count, out.data());
        return out;
    }

    [[nodiscard]] auto get_present_modes() noexcept -> std::vector<VkPresentModeKHR> {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical_.get(), info_.surface, &count, nullptr);

        auto out = std::vector<VkPresentModeKHR>(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            &physical_.get(), info_.surface, &count, out.data()
        );
        return out;
    }

    VkSwapchainCreateInfoKHR                     info_{};  // NOLINT
    std::array<std::uint32_t, 2>                 queue_family_indices_{};
    std::reference_wrapper<raw::physical_device> physical_;
    std::reference_wrapper<raw::device>          dev_;
};
}  // namespace hgn