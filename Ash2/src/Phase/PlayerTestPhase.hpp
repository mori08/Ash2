#pragma once

#include "IPhase.hpp"

/// @brief プレイヤー操作テストフェーズ
class PlayerTestPhase : public IPhase {
 public:
  /// @brief PlayerTestPhase の生成パラメータ（引数なし）
  struct Param {};

  /// @brief デフォルトコンストラクタ
  PlayerTestPhase() = default;

  /// @brief コンストラクタ（Param 受け取り版）
  /// @param param 生成パラメータ（未使用）
  explicit PlayerTestPhase(const Param& /*param*/) : PlayerTestPhase() {}

  /// @brief プレイヤーエンティティ（ルート）を生成する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @param frameData フレームごとの更新データ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

  /// @brief プレイヤーエンティティ（ルート＋子孫）を破棄する
  /// @param registry ECS レジストリ
  void onBeforePop(entt::registry& registry) override;

 private:
  /// @brief プレイヤーを破棄して最新の設定で再生成する
  /// @param registry ECS レジストリ
  void reloadPlayer(entt::registry& registry);

  /// プレイヤーのルートエンティティ
  entt::entity m_playerRoot = entt::null;
  /// 現在の攻撃クリップ名（空文字=攻撃中でない）
  s3d::String m_attackClip;
  /// 攻撃モーション残り時間（秒）
  double m_attackTimer = 0.0;
  /// ダミーターゲットエンティティ
  entt::entity m_dummyTarget = entt::null;
  /// 攻撃判定の子エンティティ（攻撃中のみ生成、終了時に destroy）
  entt::entity m_attackEntity = entt::null;
};
