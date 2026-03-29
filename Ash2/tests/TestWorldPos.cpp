# if USE_TEST
# include <ThirdParty/Catch2/catch.hpp>
# include "WorldPos.hpp"

TEST_CASE("WorldPos::ToScreen - far objects have smaller y")
{
	// 奥にあるものほどy座標が小さい
	WorldPos near{ 0.0, 0.0, 100.0 };
	WorldPos far { 0.0, 0.0, 500.0 };
	REQUIRE(near.ToScreen().y > far.ToScreen().y);
}

TEST_CASE("WorldPos::ToScreen - higher objects have smaller y")
{
	// 高いものほどy座標が小さい
	WorldPos ground{ 0.0,  0.0, 300.0 };
	WorldPos jump  { 0.0, 60.0, 300.0 };
	REQUIRE(jump.ToScreen().y < ground.ToScreen().y);
}

TEST_CASE("WorldPos::ToScreen - x maps to horizontal position")
{
	// x座標は横位置に対応する
	WorldPos left { 100.0, 0.0, 300.0 };
	WorldPos right{ 500.0, 0.0, 300.0 };
	REQUIRE(left.ToScreen().x < right.ToScreen().x);
}

TEST_CASE("DrawOrderLess - far objects come first")
{
	// 奥のオブジェクトが先に来る
	WorldPos near{ 0.0, 0.0, 100.0 };
	WorldPos far { 0.0, 0.0, 500.0 };
	REQUIRE(DrawOrderLess(far, near));
	REQUIRE_FALSE(DrawOrderLess(near, far));
}

# endif
