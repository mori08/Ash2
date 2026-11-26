#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "RandomSource.hpp"

// 誤用（nullptr・代入・一時オブジェクト・引数の書き忘れ）はコンパイルで防がれる
static_assert(!std::is_constructible_v<RandomSource, std::nullptr_t>);
static_assert(!std::is_copy_assignable_v<RandomSource>);
static_assert(!std::is_constructible_v<RandomSource, DefaultRNG>);
static_assert(!std::is_default_constructible_v<RandomSource>);

TEST_CASE("RandomSource::range - values stay within the range") {
  // 引いた値が [min, max] に収まる
  DefaultRNG engine{12345};
  const RandomSource random{engine};
  for (int32 i = 0; i < 1000; ++i) {
    const double value = random.range(-2.0, 3.0);
    REQUIRE(value >= -2.0);
    REQUIRE(value <= 3.0);
  }
}

TEST_CASE("RandomSource::range - same seed gives same sequence") {
  // 同じシードのエンジンからは同じ値の列が得られる
  DefaultRNG engineA{777};
  DefaultRNG engineB{777};
  const RandomSource a{engineA};
  const RandomSource b{engineB};
  for (int32 i = 0; i < 10; ++i) {
    REQUIRE(a.range(0.0, 100.0) == b.range(0.0, 100.0));
  }
}

TEST_CASE("RandomSource - copies share the engine") {
  DefaultRNG engine{42};
  DefaultRNG expectedEngine{42};
  const RandomSource original{engine};
  const RandomSource copy = original;
  const RandomSource expected{expectedEngine};

  // コピーと交互に引いても、1つのエンジンから続けて引いた列と一致する
  const double first = original.range(0.0, 1.0);
  const double second = copy.range(0.0, 1.0);
  REQUIRE(first == expected.range(0.0, 1.0));
  REQUIRE(second == expected.range(0.0, 1.0));
}

#endif
