#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

struct FrameData;

/// @brief プレイヤーの移動・ジャンプ・重力・アニメーションクリップ選択システム
class PlayerMovementSystem {
 public:
  /// @brief Player コンポーネントを持つエンティティの移動処理を更新する
  ///
  /// 水平速度の決定・位置への速度適用・ジャンプ・重力・地面クランプ・
  /// アニメーションクリップ選択・向き更新を行う。
  ///
  /// @param attackClip
  /// 現在の攻撃クリップ名。空文字列でない場合は水平移動をロックし、
  ///                   クリップ選択も攻撃クリップを優先する
  static void Update(entt::registry& registry, const FrameData& frameData,
                     const s3d::String& attackClip = {});
};
