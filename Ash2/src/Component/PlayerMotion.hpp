#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief Player専用のモーション（行動状態）
namespace PlayerMotion {

/// @brief 通常状態（待機・移動・ジャンプ可能）
struct Neutral {};

/// @brief 近距離攻撃1段目
struct Melee1 {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// 攻撃判定の子エンティティ
  entt::entity hitboxEntity = entt::null;
  /// 後隙中の次段への遷移予約（windup/active中の入力で立つ）
  bool comboQueued = false;
};

/// @brief 近距離攻撃2段目
struct Melee2 {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// 攻撃判定の子エンティティ
  entt::entity hitboxEntity = entt::null;
  /// 後隙中の次段への遷移予約（windup/active中の入力で立つ）
  bool comboQueued = false;
};

/// @brief 近距離攻撃3段目（締め技、キャンセル不可）
struct Melee3 {
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
