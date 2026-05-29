#pragma once

/// @brief std::visit に複数のラムダを渡すためのヘルパー構造体
/// @tparam Ts ラムダまたは関数オブジェクトの型リスト
/// @note 各 Ts の operator() を継承することで、variant のすべての型に対応した
/// visitor を構成する
template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
