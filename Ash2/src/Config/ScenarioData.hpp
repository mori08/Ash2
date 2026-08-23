#pragma once
#include <Siv3D.hpp>

#include <expected>
#include <functional>
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

/// @brief PhaseLoader の生成結果
struct LoadedPhase {
  IPhaseMaker::Ptr maker;
  /// 別のシナリオセクションを参照する場合のみ設定される
  Optional<String> referencedSection;
};

/// @brief TOML 値から LoadedPhase を生成する関数の型
using PhaseLoader =
    std::function<std::expected<LoadedPhase, String>(const TOMLValue&)>;

/// @brief フェーズ名 → PhaseLoader のマップ
using PhaseLoaderTable = HashTable<String, PhaseLoader>;

/// @brief 起動時に最初に実行されるシナリオセクション名
inline constexpr StringView kInitSectionName = U"init";

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
  /// @param loaders フェーズ名 → PhaseLoader のマップ（呼び出し元が用意する）
  [[nodiscard]] static std::expected<ScenarioData, String> FromToml(
      const TOMLValue& toml, const PhaseLoaderTable& loaders
  );
};
