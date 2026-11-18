#pragma once

#include <cassert>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class image_memory_barrier_executor final {
  public:
    explicit image_memory_barrier_executor(raw::command_buffer& buffer) noexcept
        : buffer_{std::ref(buffer)} {
        info_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        // キューファミリー間の所有権行こうを行う場合の移行元のインデックスを指定
        info_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // 移行先のインデックス。なお、同じキューファミリ内で処理するならIGNOREDを指定
        info_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // 例えば、Transferで読み込んだ画像をGraphicsに移行するときに使う

        // 画像に対し、どのミップマップレベル、どのレイヤー、どのカラー深度アスペクト
        // に対してバリアを適用するかを指定する
        info_.subresourceRange.baseMipLevel = 0;  // 基準ミップマップ(元画像サイズは0)
        info_.subresourceRange.levelCount = 1;    // ミップマップレベルの指定
        info_.subresourceRange.baseArrayLayer = 0;  // 画像が配列なら開始インデックスを指定
        info_.subresourceRange.layerCount = 1;  // いくつのレイヤーを対象にするかを指定
    }

    auto layout(const image_layout olds, const image_layout news) noexcept
        -> image_memory_barrier_executor& {
        info_.oldLayout = static_cast<VkImageLayout>(olds);
        info_.newLayout = static_cast<VkImageLayout>(news);
        return *this;
    }

    auto aspect_mask(const image_aspect_flag_bits aspect
    ) noexcept -> image_memory_barrier_executor& {
        info_.subresourceRange.aspectMask = static_cast<VkImageAspectFlagBits>(aspect);
        return *this;
    }

    auto image(borrowed_image img) noexcept -> image_memory_barrier_executor& {
        info_.image = &img.get();
        return *this;
    }

    void barrier() noexcept {
        auto source      = VkPipelineStageFlagBits{};
        auto destination = VkPipelineStageFlagBits{};

        if (info_.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and
            info_.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {  // 画像初期化時の場合
            info_.srcAccessMask = 0;  // 初期化時なのでバリアの前には何もされていない
            info_.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // 書き込みが終わったらバリアを終了
            source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // 初期化時なため何も待たない
            destination =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // ピクセルが出力された瞬間
        } else if (info_.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and
                   info_.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {  // 書き込み終了時の場合
            info_.srcAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // 書き込みが終了したらバリアを開始
            info_.dstAccessMask = 0;  // 終わるまでは特に何もされないため
            source = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // ピクセルが出力された瞬間
            destination = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;  // 全コマンドが終了するまで
            // TODO BOTTOM OF PIPE BITは非推奨化されているらしいので要修正
        } else if (info_.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and
                   info_.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            info_.srcAccessMask = 0;
            info_.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            source              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination         = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (info_.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and
                   info_.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            info_.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            info_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            source              = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination         = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (info_.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and
                   info_.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            info_.srcAccessMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            info_.dstAccessMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            source              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination         = static_cast<VkPipelineStageFlagBits>(
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
            );
        } else if (info_.oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL and
                   info_.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            info_.srcAccessMask = 0;
            info_.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            source      = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            assert(false and "unsupported layout transition.");
        }

        // sourceはバリアより前に完了していなければならないパイプラインステージ
        // destinationはバリアが完了するまで待機させられるパイプラインステージ
        vkCmdPipelineBarrier(
            &buffer_.get(), source, destination, 0, 0, nullptr, 0, nullptr, 1, &info_
        );
    }

  private:
    VkImageMemoryBarrier                        info_{};
    std::reference_wrapper<raw::command_buffer> buffer_;
};
}  // namespace hgn