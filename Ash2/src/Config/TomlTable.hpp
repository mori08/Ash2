#pragma once
#include <Siv3D.hpp>

/// @brief TOML テーブルから単一キーの値を引く
///
/// ドット区切りのパスは扱わない（単一のキー名のみ）。
/// operator[] は欠落時にライブラリ内部で例外を使うため、
/// hasMember で存在を確かめてから引く。
/// @return テーブルでない、またはキーが欠落しているとき空の TOMLValue
[[nodiscard]] inline TOMLValue TomlMember(
    const TOMLValue& table, StringView key
) {
  const String name{key};
  if (!table.isTable() || !table.hasMember(name)) {
    return TOMLValue{};
  }
  return table[name];
}

/// @brief TOML テーブルのキーを読む
///
/// 必須キーの欠落や型違いはその場で throw する。
class TomlTable {
 public:
  explicit TomlTable(TOMLValue table) : m_table{std::move(table)} {}

  /// @brief 必須キーを読む
  /// @throw Error キーがないか、型が違うとき
  template <class T>
  [[nodiscard]] T get(StringView key) const {
    if (auto value = TomlMember(m_table, key).getOpt<T>()) {
      return *std::move(value);
    }
    throw Error{U"TOML の必須キーがないか、型が違う"};
  }

  /// @brief 省略可能キーを読む
  /// @return キーが欠落しているとき defaultValue
  /// @throw Error キーがあり、型が違うとき
  template <class T>
  [[nodiscard]] T getOr(StringView key, T defaultValue) const {
    const auto member = TomlMember(m_table, key);
    if (member.isEmpty()) {
      return defaultValue;
    }
    if (auto value = member.getOpt<T>()) {
      return *std::move(value);
    }
    throw Error{U"TOML の省略可能キーの型が違う"};
  }

 private:
  TOMLValue m_table;
};
