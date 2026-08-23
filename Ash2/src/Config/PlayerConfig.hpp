#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief 攻撃・ダッシュ系アクション共通の4区間タイムライン
///
/// 構え（windup）・攻撃/移動（active）・後隙A（recoveryA、キャンセル不可）・
/// 後隙B（recoveryB、キャンセル可）の順に経過する。区間の意味はアクションごとに
/// 異なる（例: Dash の active はダッシュ移動時間）。
// 経過時刻からの区間判定はアクションを問わず共通のため、この構造体に集約する。
struct MotionTimeline {
  /// 構え時間（秒）
  double windupSec = 0.0;
  /// 攻撃/移動が有効な時間（秒）
  double activeSec = 0.0;
  /// 後隙A（キャンセル不可）の時間（秒）
  double recoveryASec = 0.0;
  /// 後隙B（キャンセル可）の時間（秒）
  double recoveryBSec = 0.0;

  /// @brief active 区間の開始時刻（秒）
  [[nodiscard]] double activeStart() const { return windupSec; }
  /// @brief active 区間の終了時刻（秒）
  [[nodiscard]] double activeEnd() const { return windupSec + activeSec; }
  /// @brief 後隙Aの終了時刻（秒）
  [[nodiscard]] double recoveryAEnd() const {
    return activeEnd() + recoveryASec;
  }
  /// @brief 後隙Bの終了時刻（秒、モーション全体の終了時刻でもある）
  [[nodiscard]] double recoveryBEnd() const {
    return recoveryAEnd() + recoveryBSec;
  }

  /// @brief active 区間中か
  [[nodiscard]] bool isActive(double elapsed) const {
    return elapsed >= activeStart() && elapsed < activeEnd();
  }
  /// @brief 後隙B（キャンセル可能区間）に入っているか
  [[nodiscard]] bool isCancelable(double elapsed) const {
    return elapsed >= recoveryAEnd();
  }
  /// @brief モーション全体が終了したか
  [[nodiscard]] bool isFinished(double elapsed) const {
    return elapsed >= recoveryBEnd();
  }
  /// @brief active 区間内の進行度
  /// @return 0.0〜1.0（区間外を渡した場合は範囲外の値になりうる）
  [[nodiscard]] double activeProgress(double elapsed) const {
    return (elapsed - activeStart()) / activeSec;
  }
};

/// @brief 近接攻撃の軌道パターン
enum class MeleeTrajectory : uint8 {
  /// 前方への直線的な突き出し
  Thrust,
  /// 斜め下から斜め上への斬り上げ
  Slash,
};

/// @brief 近接1振り分の共通設定
struct MeleeSwingConfig {
  /// この振りのタイムライン
  MotionTimeline timeline;
  /// 攻撃カプセルの半径
  double radius = 0.0;
  /// 軌道パターン
  MeleeTrajectory trajectory = MeleeTrajectory::Thrust;
  /// Slash 軌道時の斬り上げの振り幅（capMidH を中心とした上下の幅）。
  /// Thrust では未使用
  double slashRiseHeight = 0.0;
  /// Slash 軌道の曲がり具合。0.0 で始点と終点を結ぶ直線、大きいほど
  /// 前に出てから跳ね上がる弧を描く。Thrust では未使用
  double slashCurve = 0.0;
  /// ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間（秒）
  double hitstopSec = 0.0;
};

/// @brief 近接コンボの締め段の設定値
struct MeleeFinisherConfig {
  /// 共通の振り設定
  MeleeSwingConfig swing;
  /// 見た目を担う光の数（2以上、parse 時に検証する）
  int32 lightCount = 2;
  /// 光どうしの間隔（h 方向）。攻撃開始時が最大で終了時に 0 へ閉じる
  double lightGap = 0.0;
};

/// @brief 近距離攻撃の設定値
struct MeleeConfig {
  /// 攻撃カプセルの高さ中点（WorldPos.h 方向オフセット）
  double capMidH;
  /// 攻撃リーチ（w 軸方向の距離）
  double reach;
  /// 与えるダメージ量（全段共通）
  int32 damage;
  /// 継続段の設定（1つ以上、先頭が1段目）
  Array<MeleeSwingConfig> chain;
  /// 締め段の設定
  MeleeFinisherConfig finisher;
};

/// @brief ダッシュの設定値
struct DashConfig {
  /// ダッシュ移動速度（ピクセル/秒）
  double speed;
  /// 構え・ダッシュ・後隙A・後隙Bのタイムライン（activeSec がダッシュ移動時間）
  MotionTimeline timeline;
  /// 1回の発生に必要なスタミナ消費量
  int32 staminaCost;
};

/// @brief 遠距離攻撃の設定値
struct RangedConfig {
  /// 攻撃リーチ（w 軸方向の距離）
  double reach;
  /// 攻撃カプセルの半径（弾コライダーの半径・CircleDrawable の表示半径と兼用）
  double radius;
  /// 与えるダメージ量
  int32 damage;
  /// 弾の移動速度（横方向、ピクセル/秒）
  double bulletSpeed;
  /// 弾の発射高さ（プレイヤーの WorldPos.h からのオフセット）
  double spawnHeight;
  /// 1回の発生に必要なスタミナ消費量
  int32 staminaCost;
};

/// @brief ダッシュ攻撃の設定値
struct DashAttackConfig {
  /// 構え・攻撃・後隙A・後隙Bのタイムライン（recoveryBSec は 0 相当）
  MotionTimeline timeline;
  /// 突進速度（ピクセル/秒）
  double speed;
  /// ヒットボックスの軌道半径（w-d 平面上の円）
  double orbitRadius;
  /// 攻撃カプセルの半径
  double radius;
  /// 与えるダメージ量
  int32 damage;
  /// ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間（秒）
  double hitstopSec = 0.0;
};

/// @brief 空中攻撃の設定値
struct AirAttackConfig {
  /// 構え・攻撃・後隙A・後隙Bのタイムライン（recoveryBSec は 0 相当）
  MotionTimeline timeline;
  /// 地上ニュートラル速度（PlayerConfig::speed）に対するドリフト移動速度の倍率
  double driftRatio;
  /// ヒットボックスの軌道半径（w-h 平面上の円）
  double orbitRadius;
  /// 攻撃カプセルの半径
  double radius;
  /// 与えるダメージ量
  int32 damage;
  /// ヒット成立時に攻撃側・被弾側へ付与するヒットストップ時間（秒）
  double hitstopSec = 0.0;
};

/// @brief スタミナ回復の設定値
struct StaminaConfig {
  /// 行動後に回復が始まるまでの待機秒数
  double recoveryDelay;
  /// 毎秒、スタミナ不足分の何割を回復するか（0.5 = 不足分の半分/秒）
  double recoveryRate;
};

/// @brief 着地硬直の設定値
struct LandingConfig {
  /// 着地硬直時間（秒）
  double recoverySec;
};

/// @brief 攻撃演出共通の設定値
struct AttackEffectConfig {
  /// ヒットボックス解放後のフェードアウト時間（秒）
  double fadeSec = 0.0;
};

/// @brief プレイヤーの設定値
struct PlayerConfig {
  /// 横移動速度（ピクセル/秒）
  double speed;
  /// ジャンプ初速（ピクセル/秒）
  double jumpSpeed;
  /// 重力加速度（ピクセル/秒^2）
  double gravity;
  MeleeConfig melee;
  RangedConfig ranged;
  DashConfig dash;
  DashAttackConfig dashAttack;
  AirAttackConfig airAttack;
  StaminaConfig stamina;
  LandingConfig landing;
  AttackEffectConfig attackEffect;

  /// @brief TOML からプレイヤー設定を生成する
  [[nodiscard]] static std::expected<PlayerConfig, String> FromToml(
      const TOMLValue& toml
  );
};
