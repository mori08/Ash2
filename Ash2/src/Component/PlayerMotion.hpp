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

/// @brief ダッシュ中（構え・ダッシュ・後隙A・後隙Bの4区間を持つ）
struct Dash {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// ダッシュ攻撃への遷移予約（後隙B開始時に発生）
  bool dashAttackQueued = false;
  /// 再ダッシュへの遷移予約（後隙B開始時に発生）
  bool dashQueued = false;
  /// ダッシュ移動中に記録した最終方向ベクトル（正規化済み）
  Vec2 lastDashDir = {1.0, 0.0};
};

/// @brief ダッシュ攻撃中（構え・攻撃・後隙の3区間を持つ）
struct DashAttack {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// 攻撃判定の子エンティティ
  entt::entity hitboxEntity = entt::null;
  /// ダッシュ時の移動方向（突進フェーズに使用、正規化済み）
  Vec2 dashDir = {1.0, 0.0};
};

/// @brief 空中攻撃中（構え・攻撃・後隙の3区間を持ち、接地で Landing
/// へ遷移する）
struct AirAttack {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// 攻撃判定の子エンティティ
  entt::entity hitboxEntity = entt::null;
};

/// @brief 着地硬直中（空中アクションの接地検出から遷移する）
struct Landing {
  /// 残り硬直時間（秒）
  double timer = 0.0;
};

}  // namespace PlayerMotion
