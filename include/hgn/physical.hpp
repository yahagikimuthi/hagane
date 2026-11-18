#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/queue.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class physical_device final {
  public:
    explicit physical_device(raw::physical_device& raw) noexcept : raw_{std::ref(raw)} {}

    [[nodiscard]] auto name() const noexcept -> std::string {
        const auto property = get_property();
        return std::string{static_cast<const char*>(property.deviceName)};
    }

    [[nodiscard]] auto is_supporting_depth_format(const format fmt) noexcept -> bool {
        auto properties = VkFormatProperties{};
        vkGetPhysicalDeviceFormatProperties(&get(), static_cast<VkFormat>(fmt), &properties);
        constexpr auto features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        return (properties.optimalTilingFeatures & features) == features;
    }

    [[nodiscard]] auto get_queue_families() noexcept -> std::vector<queue_family> {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceQueueFamilyProperties(&get(), &count, nullptr);

        return std::views::indices(count) |
               std::views::transform([this](std::uint32_t index) noexcept -> queue_family {
                   return queue_family{index, get()};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto create_device(const queue_family& family
    ) noexcept -> std::expected<device, result> {
        auto           device_queue   = VkDeviceQueueCreateInfo{};
        constexpr auto priority       = 1.f;
        device_queue.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue.queueFamilyIndex = family.index();

        device_queue.queueCount = 1;  // キューファミリーから生成するキューの数
        // 優先順位(2つ作る場合はstd::array{1.f, 0.5f}.data()などを期待)
        device_queue.pQueuePriorities = &priority;

        auto dynamic_rendering  = VkPhysicalDeviceDynamicRenderingFeatures{};
        dynamic_rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamic_rendering.dynamicRendering = VK_TRUE;  // 使用する

        auto features =
            VkPhysicalDeviceFeatures{};  // 物理デバイス機能(ゼロ初期化では何も指定しない)

        auto info                  = VkDeviceCreateInfo{};
        info.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext                 = &dynamic_rendering;  // 動的レンダリング拡張の仕様
        info.queueCreateInfoCount  = 1;                   // 実質キューファミリーの数
        info.pQueueCreateInfos     = &device_queue;
        info.pEnabledFeatures      = &features;
        info.enabledExtensionCount = 1;

        // 拡張機能名
        constexpr auto* extension    = "VK_KHR_swapchain";
        info.ppEnabledExtensionNames = &extension;

        auto* raw = static_cast<raw::device*>(nullptr);
        return to_expected(vkCreateDevice(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> device { return device{*raw, get()}; });
    }

    [[nodiscard]] auto get() noexcept -> raw::physical_device& { return raw_; }

    [[nodiscard]] auto get() const noexcept -> raw::physical_device& { return raw_; }

  private:
    [[nodiscard]] auto get_property() const noexcept -> VkPhysicalDeviceProperties {
        auto property = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(&get(), &property);
        return property;
    }

    std::reference_wrapper<raw::physical_device> raw_;
};
}  // namespace hgn