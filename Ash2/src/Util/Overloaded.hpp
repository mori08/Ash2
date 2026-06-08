#pragma once

/// @brief std::visit に複数のラムダを渡すためのヘルパー構造体
template <class... Ts>
struct Overloaded : Ts... {
  // 各 Ts の operator() を継承して 1 つの型にまとめることで、
  // variant の全代替型に対応する visitor として渡せるようにする。
  using Ts::operator()...;
};
