#pragma once

#include <ecs/Forward.h>
#include <ecs/ECS.h>
#include <ecs/SparseBuffer.h>

#include <stl/vector.h>

namespace mud
{
	class ComponentBufferBase
	{
	public:
		virtual ~ComponentBufferBase() {}

		EntFlags m_flag;

		virtual void Clear() = 0;
		virtual void Add() = 0;
		virtual void Remove(uint32_t index) = 0;
		//virtual Ref Get(uint32_t index) = 0;

		virtual void SortComponents() = 0;
	};

	template <class T>
	class ComponentBuffer : public ComponentBufferBase
	{
	public:
		ComponentBuffer() {}
		ComponentBuffer(int bufferIndex, int capacity = 1 << 10)
		{
			m_data.reserve(capacity);
			m_flag = uint64_t(1ULL << bufferIndex);
		}

		virtual void SortComponents() override
		{
			//quicksort(to_array(this->m_keys), to_array(this->m_data), this->m_indices);
		}

		virtual void Clear() override
		{
			m_data.clear();
		}

		virtual void Add() override
		{
			m_data.emplace_back();
		}

		virtual void Remove(uint32_t index) override
		{
			swap(m_data[index], m_data.back());
			m_data.pop_back();
		}

		vector<T> m_data;
	};
}