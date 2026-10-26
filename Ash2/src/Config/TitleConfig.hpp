#pragma once
#include <Siv3D.hpp>

/// @brief タイトル画面の配置の設定値
struct TitleConfig {
  /// ロゴの y 座標（論理座標、px）
  double logoY;

  /// @brief TOML からタイトル画面の設定を生成する
  /// @throw Error 必須キーの欠落・型違いがあるとき
  [[nodiscard]] static TitleConfig FromToml(const TOMLValue& toml);
};
