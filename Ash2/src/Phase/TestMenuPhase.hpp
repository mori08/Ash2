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

  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

 private:
  /// メニュー項目（表示名 + 生成ラムダ）
  struct MenuItem {
    /// 表示名
    s3d::String label;
    /// フェーズ生成ラムダ
    std::function<std::unique_ptr<IPhase>(entt::registry&)> create;
  };

  static constexpr int KFontSize = 24;

  s3d::Array<MenuItem> m_items;
  int m_selectedIndex = 0;
  s3d::Font m_font{KFontSize};
};
