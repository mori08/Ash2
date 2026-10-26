#pragma once
#include <Siv3D.hpp>

/// @brief バトル用アリーナの境界の設定値（全ステージ共通）
struct ArenaConfig {
  /// 横方向（w）の境界の半幅
  double halfW;
  /// 奥行き方向（d）の境界の半幅
  double halfD;

  /// @brief TOML からアリーナ境界の設定を生成する
  /// @throw Error 必須キーの欠落・型違い・不正な値があるとき
  [[nodiscard]] static ArenaConfig FromToml(const TOMLValue& toml);
};
