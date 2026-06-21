#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief Player専用のモーション（行動状態）
namespace PlayerMotion {

/// @brief 通常状態（待機・移動・ジャンプ可能）
struct Neutral {};

/// @brief 近距離攻撃中
struct Melee {
  /// コンボ段数（1始まり）
  int stage = 1;
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// 攻撃判定の子エンティティ
  entt::entity hitboxEntity = entt::null;
};

/// @brief 遠距離攻撃中
struct Ranged {
  /// 攻撃モーション残り時間（秒）
  double timer = 0.0;
};

}  // namespace PlayerMotion
