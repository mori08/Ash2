#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>
#include <functional>

#include "IPhase.hpp"

/// 「はじめる」で積む開始演出のシナリオセクション名
inline constexpr StringView kTitleStartSectionName = U"TitleStart";

/// @brief タイトル画面フェーズ（背景・水面線・植物・昇る泡・ロゴと「はじめる」
/// 「おわる」のメニュー）
class TitlePhase : public IPhase {
 public:
  /// @brief TitlePhase の生成パラメータ（引数なし）
  struct Param {};

  TitlePhase() = default;

  explicit TitlePhase(const Param& /*param*/) : TitlePhase() {}

  /// @brief 背景・水面線・植物・ロゴとメニュー（2ボタン）を生成し、
  /// 押されたボタンと処理の対応表を作る
  void onAfterPush(entt::registry& registry) override;

  /// @brief 植物・泡・メニューを更新し、押されたボタンの処理の結果を返す
  /// @note 泡は最初の呼び出しで1つ出し、以降は設定の間隔（ばらつきあり）で出す
  /// @return 押されたボタンの処理が返す `PhaseCommand`。押されていなければ
  /// `None`
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

  /// @brief 背景・水面線・植物・泡・ロゴとメニューのエンティティを破棄し、
  /// 対応表を空にする
  void onBeforePop(entt::registry& registry) override;

 private:
  /// @brief 間隔が来たら泡を出し、出ている泡を昇らせる
  void updateBubbles(entt::registry& registry, const FrameData& frameData);

  /// 背景（画面全体の矩形）のエンティティ
  entt::entity m_background = entt::null;
  /// 水面線（横長の矩形）のエンティティ
  entt::entity m_surfaceLine = entt::null;
  /// ロゴ画像のエンティティ
  entt::entity m_logo = entt::null;
  /// 植物の親エンティティ（子は棒）
  Array<entt::entity> m_plants;
  /// メニューの親エンティティ（子はボタン）
  entt::entity m_menu = entt::null;
  /// 次の泡を出すまでの残り秒数。0 以下になったら出す
  double m_bubbleWait = 0.0;
  /// ボタンのエンティティ → 押されたときの処理
  HashTable<entt::entity, std::function<PhaseCommand()>> m_actions;
};
