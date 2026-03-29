# include <Siv3D.hpp>
# include "WorldPos.hpp"

# if USE_TEST
# define CATCH_CONFIG_RUNNER
# include <ThirdParty/Catch2/catch.hpp>

static void RunTests()
{
	Console.open();
	if (Catch::Session().run() != 0)
	{
		static_cast<void>(std::getchar());
	}
}
# endif

void Main()
{
# if USE_TEST
	RunTests();
# endif

	while (System::Update())
	{
	}
}
