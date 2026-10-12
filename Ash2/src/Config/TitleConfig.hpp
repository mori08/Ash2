#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief タイトル画面の配置の設定値
struct TitleConfig {
  /// ロゴの y 座標（論理座標、px）
  double logoY;

  /// @brief TOML からタイトル画面の設定を生成する
  [[nodiscard]] static std::expected<TitleConfig, String> FromToml(
      const TOMLValue& toml
  );
};
