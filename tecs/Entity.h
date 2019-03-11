//  Copyright (c) 2019 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#pragma once

#include <stl/swap.h>
#include <tecs/Forward.h>
#include <stdint.h>

namespace mud
{
	template <size_t EcsType, size_t Index>
	struct TypeBuffer
	{};

	template <class T>
	struct TypedBuffer
	{};

	struct refl_ struct_ MUD_ECS_EXPORT Entity
	{
		Entity() {}
		Entity(uint8_t ecs, uint16_t stream, uint32_t handle) : m_ecs(ecs), m_stream(stream), m_handle(handle) {}

		explicit operator bool() const { return m_handle != UINT32_MAX; }
		operator uint32_t() const { return m_handle; }

		bool operator==(const Entity& other) const { return m_handle == other.m_handle; };
		bool operator!=(const Entity& other) const { return m_handle != other.m_handle; };

		void destroy(); // { if(m_handle != UINT32_MAX) s_ecs[m_ecs]->destroy(m_handle); }

		void swap(Entity& other) { using mud::swap; swap(m_handle, other.m_handle); swap(m_ecs, other.m_ecs); }

		uint8_t m_ecs = UINT8_MAX;
		uint16_t m_stream = UINT16_MAX;
		uint32_t m_handle = UINT32_MAX;
	};

	struct Entt
	{
		GridECS* m_ecs = nullptr;
		uint32_t m_handle = UINT32_MAX;

		template <class T>
		T& comp();
	};
}
