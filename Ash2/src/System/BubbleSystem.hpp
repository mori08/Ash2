#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Config/TitleConfig.hpp"
#include "FrameData.hpp"

/// @brief 泡が割れたことの知らせ
struct BubblePop {
  /// 割れた位置（画面座標、px）
  Vec2 pos;
  /// 割れた位置の奥行き。水面で 0、画面の下端で 1
  double depthRate;
};

/// @brief 泡を生み、昇らせ、割れたら消すシステム
class BubbleSystem {
 public:
  /// @brief 泡を1つ作る
  /// @param area 沼の範囲（上端が水面、下端が画面の下端）
  /// @note 割れる位置は乱数で選ぶ。奥（上）の泡ほど遅く、短く昇る
  static void Spawn(
      entt::registry& registry, const FrameData& frameData,
      const BubbleConfig& config, const RectF& area
  );

  /// @brief 泡を昇らせ、濃さを変え、割れた泡を消す
  /// @return このフレームで割れた泡の知らせ
  [[nodiscard]] static Array<BubblePop> Update(
      entt::registry& registry, const FrameData& frameData
  );
};
