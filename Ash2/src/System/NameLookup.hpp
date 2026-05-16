#pragma once
#include <Siv3D.hpp>

#include <ThirdParty/entt/entt.hpp>

#include "Component/Name.hpp"

/// @brief 名前からエンティティへの参照を管理するコンテキスト
using NameLookup = s3d::HashTable<s3d::String, entt::entity>;

namespace NameLookupSystem {

/// @brief Name コンポーネント削除時に NameLookup
/// のエントリを削除するシグナルハンドラ
inline void OnNameDestroyed(entt::registry& registry, entt::entity entity) {
  registry.ctx().get<NameLookup>().erase(registry.get<Name>(entity).value);
}

/// @brief registry に Name 削除シグナルを登録する
/// @param registry 対象レジストリ
inline void Connect(entt::registry& registry) {
  registry.on_destroy<Name>().connect<&OnNameDestroyed>();
}

}  // namespace NameLookupSystem
