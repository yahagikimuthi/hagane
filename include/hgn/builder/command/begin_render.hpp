#pragma once

#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class command_begin_rendering_executor final {
  public:
    explicit command_begin_rendering_executor(raw::command_buffer& buffer) noexcept
        : buffer_{std::ref(buffer)} {
        info_.sType             = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info_.renderArea.offset = {.x = 0, .y = 0};  // 画面描画範囲のオフセット
        info_.layerCount        = 1;

        color_attachment_.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        // 書いている最中の画像のレイアウト
        color_attachment_.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // 書き始める前にそのキャンバスをどうするか。以下なら最初に指定した色で塗りつぶす
        color_attachment_.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // 書き終わった後にその結果をどう扱うか。以下なら次の処理のためVRAMに保存
        color_attachment_.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_.clearValue.color = {{1.f, 1.f, 1.f, 1.f}};

        depth_attachment_.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_attachment_.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment_.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // 開始時に深度をクリア
        depth_attachment_.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // 描画後は保持しない
        depth_attachment_.clearValue.depthStencil = {.depth = 1.f, .stencil = 0};  // 初期値
    }

    auto color_image_view(image_view& view) noexcept -> command_begin_rendering_executor& {
        color_attachment_.imageView = &view.get();
        return *this;
    }

    auto clear_color(const float r, const float g, const float b, const float a) noexcept
        -> command_begin_rendering_executor& {
        color_attachment_.clearValue.color = {{r, g, b, a}};
        return *this;
    }

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept
        -> command_begin_rendering_executor& {
        info_.renderArea.extent = {.width = width, .height = height};
        return *this;
    }

    auto depth_image_view(image_view& view) noexcept -> command_begin_rendering_executor& {
        depth_attachment_.imageView = &view.get();
        return *this;
    }

    void begin() noexcept {
        info_.colorAttachmentCount = 1;
        info_.pColorAttachments    = &color_attachment_;

        if (depth_attachment_.imageView != nullptr) info_.pDepthAttachment = &depth_attachment_;

        vkCmdBeginRendering(&buffer_.get(), &info_);
    }

  private:
    std::reference_wrapper<raw::command_buffer> buffer_;
    VkRenderingInfo                             info_{};
    VkRenderingAttachmentInfo                   color_attachment_{};
    VkRenderingAttachmentInfo                   depth_attachment_{};
};
}  // namespace hgn