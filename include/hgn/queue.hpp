#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/builder/queue.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class queue_family final {
  public:
    queue_family(std::uint32_t index, raw::physical_device& physical) noexcept
        : index_{index}, physical_{std::ref(physical)} {}

    [[nodiscard]] auto index() const noexcept -> std::uint32_t { return index_; }

    [[nodiscard]] auto flags() noexcept -> queue_flag_bits {
        const auto out = get_properties().queueFlags;
        return static_cast<queue_flag_bits>(out);
    }

    [[nodiscard]] auto is_supporting_surface(surface_khr& surface
    ) noexcept -> std::expected<bool, result> {
        auto supporting = VkBool32{};
        return to_expected(vkGetPhysicalDeviceSurfaceSupportKHR(
                               &physical_.get(), index(), &surface.get(), &supporting
                           ))
            .transform([&]() noexcept -> bool { return supporting != VK_FALSE; });
    }

    [[nodiscard]] auto operator==(const queue_family& other) const noexcept -> bool {
        return index() == other.index();
    }

  private:
    [[nodiscard]] auto get_properties() noexcept -> VkQueueFamilyProperties {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceQueueFamilyProperties(&physical_.get(), &count, nullptr);

        auto properties = std::vector<VkQueueFamilyProperties>(count);
        vkGetPhysicalDeviceQueueFamilyProperties(&physical_.get(), &count, properties.data());

        return properties[index()];
    }

    std::uint32_t                                index_;
    std::reference_wrapper<raw::physical_device> physical_;
};

class queue final {
  public:
    explicit queue(raw::queue& raw) noexcept : raw_{std::ref(raw)} {}

    [[nodiscard]] auto get_present_executor() noexcept -> present_executor {
        return present_executor{get()};
    }

    [[nodiscard]] auto get_submitter() noexcept -> submitter { return submitter{get()}; }

    [[nodiscard]] auto get() noexcept -> raw::queue& { return raw_; }

  private:
    std::reference_wrapper<raw::queue> raw_;
};
}  // namespace hgn