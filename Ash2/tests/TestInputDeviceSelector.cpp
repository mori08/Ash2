#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Input/InputDeviceSelector.hpp"

namespace {

using Device = InputDeviceSelector::Device;
using Choice = InputDeviceSelector::DeviceChoice;
using Signals = InputDeviceSelector::DeviceSignals;

constexpr double kSwitch = 8.0;

/// @brief パッド接続中で、入力のないフレームの観測値
Signals IdleSignals(double cursorTravel = 0.0) {
  return Signals{
      .padConnected = true,
      .padInput = false,
      .keyboardMouseButton = false,
      .cursorTravel = cursorTravel,
  };
}

}  // namespace

TEST_CASE("ChooseDevice - small cursor move on gamepad accumulates") {
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Gamepad, .padCursorTravel = 0.0},
      IdleSignals(3.0), kSwitch
  );
  REQUIRE(result.device == Device::Gamepad);
  REQUIRE(result.padCursorTravel == Approx(3.0));
}

TEST_CASE(
    "ChooseDevice - accumulated move over the distance switches to keyboard"
) {
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Gamepad, .padCursorTravel = 6.0},
      IdleSignals(3.0), kSwitch
  );
  REQUIRE(result.device == Device::Keyboard);
  REQUIRE(result.padCursorTravel == 0.0);
}

TEST_CASE("ChooseDevice - travel equal to the distance does not switch") {
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Gamepad, .padCursorTravel = 5.0},
      IdleSignals(3.0), kSwitch
  );
  REQUIRE(result.device == Device::Gamepad);
}

TEST_CASE("ChooseDevice - gamepad input frame stays on gamepad and resets") {
  Signals signals = IdleSignals(20.0);
  signals.padInput = true;
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Gamepad, .padCursorTravel = 5.0}, signals,
      kSwitch
  );
  REQUIRE(result.device == Device::Gamepad);
  REQUIRE(result.padCursorTravel == 0.0);
}

TEST_CASE("ChooseDevice - cursor move on keyboard does not accumulate") {
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Keyboard, .padCursorTravel = 0.0},
      IdleSignals(50.0), kSwitch
  );
  REQUIRE(result.device == Device::Keyboard);
  REQUIRE(result.padCursorTravel == 0.0);
}

TEST_CASE("ChooseDevice - keyboard or mouse button switches to keyboard") {
  Signals signals = IdleSignals();
  signals.keyboardMouseButton = true;
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Gamepad, .padCursorTravel = 5.0}, signals,
      kSwitch
  );
  REQUIRE(result.device == Device::Keyboard);
  REQUIRE(result.padCursorTravel == 0.0);
}

TEST_CASE("ChooseDevice - disconnect on gamepad falls back to keyboard") {
  Signals signals = IdleSignals();
  signals.padConnected = false;
  const auto result = InputDeviceSelector::ChooseDevice(
      Choice{.device = Device::Gamepad, .padCursorTravel = 5.0}, signals,
      kSwitch
  );
  REQUIRE(result.device == Device::Keyboard);
  REQUIRE(result.padCursorTravel == 0.0);
}

#endif
