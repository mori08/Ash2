#pragma once

#include "IPhase.hpp"

/// @brief プレイヤー操作デモシーン
class DemoPhase : public IPhase {
 public:
  /// @brief DemoPhase の生成パラメータ（引数なし）
  struct Param {};

  /// @brief デフォルトコンストラクタ
  DemoPhase() = default;

  /// @brief コンストラクタ（Param 受け取り版）
  /// @param param 生成パラメータ（未使用）
  explicit DemoPhase(const Param& /*param*/) : DemoPhase() {}

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
};
