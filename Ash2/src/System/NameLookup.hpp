#pragma once
#include <Siv3D.hpp>

#include <ThirdParty/entt/entt.hpp>

/// @brief 名前からエンティティへの参照を管理するコンテキスト
using NameLookup = s3d::HashTable<s3d::String, entt::entity>;
