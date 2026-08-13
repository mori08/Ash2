#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/PlayerConfig.hpp"

// 全アクション共通の4区間判定。境界が1つずれるとヒットボックスの生成・解放
// フレームが全モーションでずれるため、境界そのものを検証する。

namespace {

/// 構え 0.10 / 有効 0.20 / 後隙A 0.30 / 後隙B 0.40
constexpr MotionTimeline kTimeline{
    .windupSec = 0.10,
    .activeSec = 0.20,
    .recoveryASec = 0.30,
    .recoveryBSec = 0.40
};

}  // namespace

TEST_CASE("MotionTimeline - section boundaries accumulate in order") {
  REQUIRE(kTimeline.activeStart() == Approx(0.10));
  REQUIRE(kTimeline.activeEnd() == Approx(0.30));
  REQUIRE(kTimeline.recoveryAEnd() == Approx(0.60));
  REQUIRE(kTimeline.recoveryBEnd() == Approx(1.00));
}

// 境界は区間秒数の加算結果であり、10進リテラルとは一致しない
// （0.10 + 0.20 は 0.30 にならない）。境界そのものはアクセサから取る。
constexpr double kEpsilon = 0.01;

TEST_CASE("MotionTimeline - isActive includes its start but not its end") {
  // [activeStart, activeEnd) の半開区間
  REQUIRE_FALSE(kTimeline.isActive(kTimeline.activeStart() - kEpsilon));
  REQUIRE(kTimeline.isActive(kTimeline.activeStart()));
  REQUIRE(kTimeline.isActive(kTimeline.activeEnd() - kEpsilon));
  REQUIRE_FALSE(kTimeline.isActive(kTimeline.activeEnd()));
}

TEST_CASE("MotionTimeline - isCancelable starts when recovery A ends") {
  // 後隙Aはキャンセル不可、後隙Bはキャンセル可
  REQUIRE_FALSE(kTimeline.isCancelable(kTimeline.activeEnd()));
  REQUIRE_FALSE(kTimeline.isCancelable(kTimeline.recoveryAEnd() - kEpsilon));
  REQUIRE(kTimeline.isCancelable(kTimeline.recoveryAEnd()));
  REQUIRE(kTimeline.isCancelable(kTimeline.recoveryBEnd() - kEpsilon));
}

TEST_CASE("MotionTimeline - isFinished starts when recovery B ends") {
  REQUIRE_FALSE(kTimeline.isFinished(kTimeline.recoveryBEnd() - kEpsilon));
  REQUIRE(kTimeline.isFinished(kTimeline.recoveryBEnd()));
}

TEST_CASE("MotionTimeline - activeProgress spans 0 to 1 across the section") {
  REQUIRE(kTimeline.activeProgress(kTimeline.activeStart()) == Approx(0.0));
  REQUIRE(kTimeline.activeProgress(0.20) == Approx(0.5));
  REQUIRE(kTimeline.activeProgress(kTimeline.activeEnd()) == Approx(1.0));
}

TEST_CASE("MotionTimeline - a zero-length active section is never active") {
  // isActive が常に false になることで、activeProgress の 0 除算を
  // 呼び出し側が踏まないようになっている
  constexpr MotionTimeline noActive{
      .windupSec = 0.10, .activeSec = 0.0, .recoveryBSec = 0.20
  };

  REQUIRE_FALSE(noActive.isActive(0.09));
  REQUIRE_FALSE(noActive.isActive(0.10));
  REQUIRE_FALSE(noActive.isActive(0.11));
}

#endif
