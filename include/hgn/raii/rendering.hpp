#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class pipeline_layout final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::pipeline_layout* ptr) noexcept {
            vkDestroyPipelineLayout(&dev_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    pipeline_layout(raw::pipeline_layout& raw, raw::device& dev) noexcept
        : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::pipeline_layout& { return *raw_; }

    [[nodiscard]] operator raw::pipeline_layout&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::pipeline_layout, deleter> raw_;
};

class pipeline final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::pipeline* ptr) noexcept {
            vkDestroyPipeline(&dev_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    pipeline(raw::pipeline& raw, raw::device& dev) noexcept : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::pipeline& { return *raw_; }

    [[nodiscard]] operator raw::pipeline&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::pipeline, deleter> raw_;
};

class shader_module final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::shader_module* ptr) noexcept {
            vkDestroyShaderModule(&dev_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    shader_module(raw::shader_module& native, raw::device& dev) noexcept
        : raw_{&native, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::shader_module& { return *raw_; }

    [[nodiscard]] operator raw::shader_module&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::shader_module, deleter> raw_;
};

class descriptor_set_layout final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::descriptor_set_layout* ptr) noexcept {
            vkDestroyDescriptorSetLayout(&dev_.get(), ptr, nullptr);
        }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    descriptor_set_layout(raw::descriptor_set_layout& raw, raw::device& dev) noexcept
        : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::descriptor_set_layout& { return *raw_; }

    [[nodiscard]] operator raw::descriptor_set_layout&() noexcept { return get(); }

  private:
    std::unique_ptr<raw::descriptor_set_layout, deleter> raw_;
};

class descriptor_pool final {
    class deleter final {
      public:
        explicit deleter(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        void operator()(raw::descriptor_pool* ptr) noexcept {
            vkDestroyDescriptorPool(&dev_.get(), ptr, nullptr);
        }

        [[nodiscard]] auto device() noexcept -> raw::device& { return dev_; }

      private:
        std::reference_wrapper<raw::device> dev_;
    };

  public:
    descriptor_pool(raw::descriptor_pool& raw, raw::device& dev) noexcept
        : raw_{&raw, deleter{dev}} {}

    [[nodiscard]] auto get() noexcept -> raw::descriptor_pool& { return *raw_; }

    [[nodiscard]] operator raw::descriptor_pool&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& {
        return raw_.get_deleter().device();
    }

  private:
    std::unique_ptr<raw::descriptor_pool, deleter> raw_;
};
}  // namespace hgn::raii