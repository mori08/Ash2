#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "IPhase.hpp"

/// @brief タイトル画面フェーズ（現状は仮のタイトル文字列を表示するだけ）
class TitlePhase : public IPhase {
 public:
  /// @brief TitlePhase の生成パラメータ（引数なし）
  struct Param {};

  TitlePhase() = default;

  explicit TitlePhase(const Param& /*param*/) : TitlePhase() {}

  /// @brief タイトル文字のエンティティを生成する（y 座標は
  /// TitleConfig::logoY、x 座標は画面中央）
  void onAfterPush(entt::registry& registry) override;

  /// @brief 何もしない（遷移は持たない）
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief タイトル文字のエンティティを破棄する
  void onBeforePop(entt::registry& registry) override;

 private:
  /// タイトル文字のエンティティ
  entt::entity m_logo = entt::null;
};
