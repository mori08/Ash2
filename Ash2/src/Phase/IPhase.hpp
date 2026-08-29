#pragma once

#include <Siv3D.hpp>

#include <concepts>
#include <entt/entt.hpp>
#include <type_traits>
#include <variant>

#include "FrameData.hpp"

class IPhase;

/// @brief フェーズスタックへの操作
class PhaseCommand {
 public:
  /// @brief 何もしない
  struct None {};

  /// @brief スタックから取り出す
  struct Pop {};

  /// @brief フェーズを積む
  struct Push {
    std::unique_ptr<IPhase> nextPhase;
  };

  /// @brief スタックをクリアして積む
  struct Reset {
    std::unique_ptr<IPhase> nextPhase;
  };

  using Value = std::variant<None, Pop, Push, Reset>;

  /// @brief 要素型（None / Pop / Push / Reset）から暗黙に構築する
  template <class T>
    requires(!std::same_as<std::remove_cvref_t<T>, PhaseCommand>) &&
            std::constructible_from<Value, T>
  PhaseCommand(T&& command) : m_command(std::forward<T>(command)) {}

  [[nodiscard]] Value& value() { return m_command; }
  [[nodiscard]] const Value& value() const { return m_command; }

 private:
  Value m_command;
};

/// @brief フェーズの基底クラス
class IPhase {
 public:
  virtual ~IPhase() = default;

  /// @brief スタックに積まれた直後に呼ばれる
  virtual void onAfterPush(entt::registry&) {}

  [[nodiscard]] virtual PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) = 0;

  /// @brief スタックから取り出される直前に呼ばれる
  virtual void onBeforePop(entt::registry&) {}
};
