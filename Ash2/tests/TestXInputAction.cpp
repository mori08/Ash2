#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Input/XInputAction.hpp"

namespace {
constexpr double kTilt = 0.5;
constexpr double kRelease = 0.24;
}  // namespace

TEST_CASE("XInputAction::StickDirection - below threshold is none") {
  REQUIRE_FALSE(
      XInputAction::StickDirection(Vec2{0.3, 0.2}, kTilt).has_value()
  );
}

TEST_CASE("XInputAction::StickDirection - picks the four directions") {
  // y は上が正
  const auto up = XInputAction::StickDirection(Vec2{0.0, 1.0}, kTilt);
  REQUIRE(up.has_value());
  REQUIRE(*up == NavDirection::Up);
  const auto down = XInputAction::StickDirection(Vec2{0.0, -1.0}, kTilt);
  REQUIRE(down.has_value());
  REQUIRE(*down == NavDirection::Down);
  const auto left = XInputAction::StickDirection(Vec2{-1.0, 0.0}, kTilt);
  REQUIRE(left.has_value());
  REQUIRE(*left == NavDirection::Left);
  const auto right = XInputAction::StickDirection(Vec2{1.0, 0.0}, kTilt);
  REQUIRE(right.has_value());
  REQUIRE(*right == NavDirection::Right);
}

TEST_CASE("XInputAction::StickDirection - diagonal picks the larger axis") {
  const auto right = XInputAction::StickDirection(Vec2{0.9, 0.6}, kTilt);
  REQUIRE(right.has_value());
  REQUIRE(*right == NavDirection::Right);
  const auto down = XInputAction::StickDirection(Vec2{0.6, -0.9}, kTilt);
  REQUIRE(down.has_value());
  REQUIRE(*down == NavDirection::Down);
}

TEST_CASE("XInputAction::StickDirection - equal axes prefer vertical") {
  const auto up = XInputAction::StickDirection(Vec2{0.7, 0.7}, kTilt);
  REQUIRE(up.has_value());
  REQUIRE(*up == NavDirection::Up);
  const auto down = XInputAction::StickDirection(Vec2{-0.7, -0.7}, kTilt);
  REQUIRE(down.has_value());
  REQUIRE(*down == NavDirection::Down);
}

TEST_CASE(
    "XInputAction::HeldStickDirection - hysteresis between tilt and release"
) {
  const Vec2 weak{0.3, 0.0};

  // 倒していない間は tilt に届かないと方向なし
  REQUIRE_FALSE(
      XInputAction::HeldStickDirection(weak, none, kTilt, kRelease).has_value()
  );
  // 倒している間は release を下回るまで保持する
  const auto held = XInputAction::HeldStickDirection(
      weak, NavDirection::Right, kTilt, kRelease
  );
  REQUIRE(held.has_value());
  REQUIRE(*held == NavDirection::Right);
  // release を下回れば離したとみなす
  REQUIRE_FALSE(
      XInputAction::HeldStickDirection(
          Vec2{0.1, 0.0}, NavDirection::Right, kTilt, kRelease
      )
          .has_value()
  );
}

#endif
