#pragma once

#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief テストフェーズ一覧メニュー
class TestMenuPhase : public IPhase {
 public:
  /// @brief TestMenuPhase の生成パラメータ（引数なし）
  struct Param {};

  TestMenuPhase() = default;

  explicit TestMenuPhase(const Param& /*param*/) : TestMenuPhase() {}

  /// @brief メニュー項目を初期化する
  void onAfterPush(entt::registry& registry) override;

  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

 private:
  /// メニュー項目（表示名 + 生成ラムダ）
  struct MenuItem {
    /// 表示名
    String label;
    /// フェーズ生成ラムダ
    std::function<std::unique_ptr<IPhase>(entt::registry&)> create;
  };

  static constexpr int32 kFontSize = 24;

  Array<MenuItem> m_items;
  size_t m_selectedIndex = 0;
  Font m_font{kFontSize};
};
