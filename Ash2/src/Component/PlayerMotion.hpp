#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <variant>

/// @brief Player専用のモーション（行動状態）
namespace PlayerMotion {

/// @brief 通常状態（待機・移動・ジャンプ可能）
struct Neutral {};

/// @brief 近接コンボの継続段（次段を持つ段）
struct MeleeChain {
  /// コンボ段のインデックス（0始まり、cfg.melee.chain を参照する）
  size_t stage = 0;
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
  /// 後隙中の次段への遷移予約（windup/active中の入力で立つ）
  bool comboQueued = false;
};

/// @brief 近接コンボの締め段（コンボ継続・キャンセルを受け付けない）
struct MeleeFinisher {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
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
  /// 空中発動か（true: 空中ダッシュ相当。接地強制遷移の有無等が変わる）
  bool air = false;
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
  /// 空中発動か（true: 空中ダッシュ攻撃相当。接地強制遷移の有無が変わる）
  bool air = false;
  /// ダッシュ時の移動方向（突進フェーズに使用、正規化済み）
  Vec2 dashDir = {1.0, 0.0};
};

/// @brief 空中攻撃中（構え・攻撃・後隙の3区間を持ち、接地で Landing
/// へ遷移する）
struct AirAttack {
  /// モーション開始からの経過時間（秒）
  double elapsed = 0.0;
};

/// @brief 着地硬直中（空中アクションの接地検出から遷移する）
struct Landing {
  /// 残り硬直時間（秒）
  double timer = 0.0;
};

/// @brief 仰け反り中（軽い被弾リアクション。外部要因による強制遷移。
/// ARCHITECTURE.md の「例外：外部要因による強制遷移」を参照）
struct Stagger {
  /// 残り時間（秒）
  double timer = 0.0;
};

/// @brief 吹き飛ばし中（重い被弾リアクション。外部要因による強制遷移。
/// 放物線は Velocity+Gravity による物理に委ね、接地判定で Downed へ遷移する）
struct Knockback {};

/// @brief ダウン中（地面に倒れている）
struct Downed {
  /// 残り時間（秒）
  double timer = 0.0;
};

/// @brief 起き上がり中
struct GetUp {
  /// 残り時間（秒）
  double timer = 0.0;
};

/// @brief プレイヤーの排他的な行動状態
using Variant = std::variant<
    Neutral, MeleeChain, MeleeFinisher, Ranged, Dash, DashAttack, AirAttack,
    Landing, Stagger, Knockback, Downed, GetUp>;

}  // namespace PlayerMotion
