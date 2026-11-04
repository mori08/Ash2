#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief タイトル画面フェーズ（ロゴ画像を表示するだけ）
class TitlePhase : public IPhase {
 public:
  /// @brief TitlePhase の生成パラメータ（引数なし）
  struct Param {};

  TitlePhase() = default;

  explicit TitlePhase(const Param& /*param*/) : TitlePhase() {}

  /// @brief ロゴ画像のエンティティを生成する（中心の y 座標は
  /// TitleConfig::logoY、x 座標は画面中央）
  void onAfterPush(entt::registry& registry) override;

  /// @brief 何もしない（遷移は持たない）
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief ロゴ画像のエンティティを破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// ロゴ画像のエンティティ
  entt::entity m_logo = entt::null;
};
