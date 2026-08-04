#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief TOML テーブルの必須キーを読み、欠落キーをまとめて記録する
///
/// 1インスタンスが1テーブル・1プレフィックスを担当する。get() は欠落時に
/// 既定値 T{} を返して読み進め、check()/wrap() でまとめて失敗に畳む。
class TomlFields {
 public:
  /// @param context メッセージ先頭に付ける関数名
  /// @param prefix メッセージ内でキー名に前置するテーブル名
  TomlFields(TOMLValue table, String context, String prefix = {})
      : m_table{std::move(table)},
        m_context{std::move(context)},
        m_prefix{std::move(prefix)} {}

  /// @brief 必須キーを読む
  /// @return 欠落時は T{}（キー名を記録して読み進める）
  template <class T>
  [[nodiscard]] T get(StringView key) {
    if (auto value = m_table[String{key}].getOpt<T>()) {
      return *std::move(value);
    }
    m_missing.push_back(
        m_prefix.isEmpty() ? String{key} : (m_prefix + U"." + String{key})
    );
    return T{};
  }

  /// @brief 記録した欠落を確認する
  /// @return 欠落があれば全欠落キーを並べた unexpected
  [[nodiscard]] std::expected<void, String> check() const {
    if (m_missing.isEmpty()) {
      return {};
    }
    return std::unexpected{
        m_context + U": キーがありません: " + m_missing.join(U", ", U"", U"")
    };
  }

  /// @brief 記録した欠落を expected に畳む
  /// @return 欠落がなければ value、あれば check() と同じ unexpected
  template <class T>
  [[nodiscard]] std::expected<T, String> wrap(T value) const {
    if (auto result = check(); !result) {
      return std::unexpected{std::move(result).error()};
    }
    return value;
  }

 private:
  TOMLValue m_table;
  String m_context;
  String m_prefix;
  Array<String> m_missing;
};
