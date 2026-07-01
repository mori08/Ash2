#pragma once
#include <Siv3D.hpp>

#include <memory>
#include <variant>

class IPhase;

/// @brief フェーズ生成の型消去インタフェース
struct IPhaseMaker {
  using Ptr = std::shared_ptr<const IPhaseMaker>;

  virtual ~IPhaseMaker() = default;

  /// @brief 保持しているパラメータから IPhase インスタンスを生成する
  [[nodiscard]] virtual std::unique_ptr<IPhase> make() const = 0;
};

/// @brief シナリオのステップ：フェーズをスタックに積む
struct StepPush {
  IPhaseMaker::Ptr maker;
};

/// @brief シナリオのステップ：スタックをリセットして新フェーズを積む
struct StepReset {
  IPhaseMaker::Ptr maker;
};

/// @brief シナリオの 1 ステップを表す variant 型
using ScenarioStep = std::variant<StepPush, StepReset>;

/// @brief シナリオデータ（ロード時に全セクションを変換済み）
struct ScenarioData {
  /// セクション名 → ステップ一覧のテーブル
  HashTable<String, Array<ScenarioStep>> sections;

  /// @brief TOML からシナリオデータを生成する
  [[nodiscard]] static ScenarioData FromToml(const TOMLValue& toml);
};
