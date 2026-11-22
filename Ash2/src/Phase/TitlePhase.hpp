#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>

#include "IPhase.hpp"

/// 「はじめる」で積む開始演出のシナリオセクション名
inline constexpr StringView kTitleStartSectionName = U"TitleStart";

/// @brief タイトル画面フェーズ（背景・水面線・ロゴと「はじめる」「おわる」の
/// メニュー）
class TitlePhase : public IPhase {
 public:
  /// @brief TitlePhase の生成パラメータ（引数なし）
  struct Param {};

  TitlePhase() = default;

  explicit TitlePhase(const Param& /*param*/) : TitlePhase() {}

  /// @brief 背景・水面線・ロゴとメニュー（2ボタン）のエンティティを生成し、
  /// 押されたボタンと処理の対応表を作る
  void onAfterPush(entt::registry& registry) override;

  /// @brief メニューを更新し、押されたボタンに対応する処理の結果を返す
  /// @return 押されたボタンの処理が返す `PhaseCommand`。押されていなければ
  /// `None`
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief 背景・水面線・ロゴとメニューのエンティティを破棄し、対応表を
  /// 空にする
  void onBeforePop(entt::registry& registry) override;

 private:
  /// 背景（画面全体の矩形）のエンティティ
  entt::entity m_background = entt::null;
  /// 水面線（横長の矩形）のエンティティ
  entt::entity m_surfaceLine = entt::null;
  /// ロゴ画像のエンティティ
  entt::entity m_logo = entt::null;
  /// メニューの親エンティティ（子はボタン）
  entt::entity m_menu = entt::null;
  /// ボタンのエンティティ → 押されたときの処理
  HashTable<entt::entity, std::function<PhaseCommand()>> m_actions;
};
