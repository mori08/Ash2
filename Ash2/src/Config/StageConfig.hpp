#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief ステージ境界の設定値
struct StageConfig {
  /// 横方向（w）の境界の半幅
  double halfW;
  /// 奥行き方向（d）の境界の半幅
  double halfD;

  /// @brief TOML からステージ境界の設定を生成する
  [[nodiscard]] static std::expected<StageConfig, String> FromToml(
      const TOMLValue& toml
  );
};
