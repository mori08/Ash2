#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>

#include "IPhase.hpp"

/// 「はじめる」で積む開始演出のシナリオセクション名
inline constexpr StringView kTitleStartSectionName = U"TitleStart";

/// @brief タイトル画面フェーズ（ロゴと「はじめる」「おわる」のメニュー）
class TitlePhase : public IPhase {
 public:
  /// @brief TitlePhase の生成パラメータ（引数なし）
  struct Param {};

  TitlePhase() = default;

  explicit TitlePhase(const Param& /*param*/) : TitlePhase() {}

  /// @brief ロゴとメニュー（2ボタン）のエンティティを生成し、押されたボタンと
  /// 処理の対応表を作る
  void onAfterPush(entt::registry& registry) override;

  /// @brief メニューを更新し、押されたボタンに対応する処理の結果を返す
  /// @return 押されたボタンの処理が返す `PhaseCommand`。押されていなければ
  /// `None`
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief ロゴとメニューのエンティティを破棄し、対応表を空にする
  void onBeforePop(entt::registry& registry) override;

 private:
  /// ロゴ画像のエンティティ
  entt::entity m_logo = entt::null;
  /// メニューの親エンティティ（子はボタン）
  entt::entity m_menu = entt::null;
  /// ボタンのエンティティ → 押されたときの処理
  HashTable<entt::entity, std::function<PhaseCommand()>> m_actions;
};
