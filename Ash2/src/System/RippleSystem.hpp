#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Config/TitleConfig.hpp"
#include "System/BubbleSystem.hpp"

/// @brief 泡が割れた所に波紋を出し、広げ、寿命が来たら消すシステム
class RippleSystem {
 public:
  /// @brief 割れた泡ごとに波紋を1つ作る
  /// @note 経過 0 秒・始まりの半径・不透明度 1 で作る。奥（上）の波紋ほど
  /// 小さい。生まれたフレームで時間を進めないよう、`Update` の後に呼ぶ
  static void Spawn(
      entt::registry& registry, const Array<BubblePop>& pops,
      const RippleConfig& config
  );

  /// @brief 波紋の時間を進め、半径と不透明度を変え、寿命が来たものを消す
  static void Update(entt::registry& registry, double dt);
};
