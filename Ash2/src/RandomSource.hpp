#pragma once
#include <Siv3D.hpp>

/// @brief 乱数エンジンへの参照を包み、乱数の取り出し口を `range` に絞る
/// @note エンジンは所有しない。寿命は呼び出し側が保証する
class RandomSource {
 public:
  explicit RandomSource(DefaultRNG& engine) : m_engine(engine) {}

  /// @brief min 以上 max 以下の一様な実数を引く
  [[nodiscard]] double range(double min, double max) const {
    return Random(min, max, m_engine);
  }

 private:
  DefaultRNG& m_engine;
};
