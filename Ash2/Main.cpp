# include <Siv3D.hpp>
# include "WorldPos.hpp"

void Main()
{
	Scene::SetBackground(ColorF{ 0.15, 0.15, 0.2 });

	const Font font{ 14 };

	struct TestObj
	{
		WorldPos pos;
		ColorF color;
		String label;
	};

	Array<TestObj> objects = {
		{ { 200.0,   0.0, 500.0 }, Palette::Skyblue, U"d=500（奥）" },
		{ { 500.0,   0.0, 350.0 }, Palette::Lime,    U"d=350" },
		{ { 350.0,   0.0, 200.0 }, Palette::Orange,  U"d=200（手前）" },
		{ { 450.0, -60.0, 350.0 }, Palette::Yellow,  U"d=350 ジャンプ" },
	};

	while (System::Update())
	{
		// 奥行きガイドライン
		for (int32 i = 1; i <= 6; ++i)
		{
			const double y = i * 100.0;
			Line{ 0.0, y, Scene::Width(), y }.draw(1.0, ColorF{ 1.0, 0.2 });
			font(U"d={}"_fmt(y)).draw(4.0, y - 16.0, ColorF{ 1.0, 0.4 });
		}

		// 奥から手前の順にソートして描画
		objects.sort_by([](const TestObj& a, const TestObj& b)
		{
			return DrawOrderLess(a.pos, b.pos);
		});

		for (const auto& obj : objects)
		{
			const Vec2 screen = obj.pos.ToScreen();
			Circle{ screen, 20.0 }.draw(obj.color);
			font(obj.label).draw(screen + Vec2{ 26.0, -8.0 }, obj.color);
		}
	}
}
