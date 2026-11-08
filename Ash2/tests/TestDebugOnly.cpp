#ifdef _DEBUG

#include <Siv3D.hpp>

#include <ThirdParty/Catch2/catch.hpp>

#include "DebugOnly.hpp"

namespace {

using DebugOnly::AutoCaptureAction;
using DebugOnly::AutoCaptureProgress;

constexpr double kFrame = 1.0 / 60.0;

}  // namespace

TEST_CASE("ParseCaptureAfter - empty text means no capture") {
  REQUIRE_FALSE(DebugOnly::ParseCaptureAfter(U"").has_value());
}

TEST_CASE("ParseCaptureAfter - valid numbers are parsed") {
  REQUIRE(DebugOnly::ParseCaptureAfter(U"2.5").value() == Approx(2.5));
  REQUIRE(DebugOnly::ParseCaptureAfter(U"0").value() == Approx(0.0));
}

TEST_CASE("ParseCaptureAfter - invalid text throws") {
  REQUIRE_THROWS_AS(DebugOnly::ParseCaptureAfter(U"abc"), Error);
  REQUIRE_THROWS_AS(DebugOnly::ParseCaptureAfter(U"-1"), Error);
  REQUIRE_THROWS_AS(DebugOnly::ParseCaptureAfter(U"inf"), Error);
  REQUIRE_THROWS_AS(DebugOnly::ParseCaptureAfter(U"nan"), Error);
}

TEST_CASE("AutoCaptureProgress - captures once when time elapses") {
  AutoCaptureProgress progress{0.5};

  REQUIRE(progress.advance(0.25) == AutoCaptureAction::None);
  REQUIRE(progress.advance(0.2) == AutoCaptureAction::None);
  REQUIRE(progress.advance(0.05) == AutoCaptureAction::Capture);
}

TEST_CASE("AutoCaptureProgress - exits on the frame after capture") {
  AutoCaptureProgress progress{0.0};

  REQUIRE(progress.advance(kFrame) == AutoCaptureAction::Capture);
  REQUIRE(progress.advance(kFrame) == AutoCaptureAction::Exit);
  REQUIRE(progress.advance(kFrame) == AutoCaptureAction::None);
}

TEST_CASE("AutoCaptureProgress - zero seconds captures on the first frame") {
  AutoCaptureProgress progress{0.0};

  REQUIRE(progress.advance(0.0) == AutoCaptureAction::Capture);
}

#endif
