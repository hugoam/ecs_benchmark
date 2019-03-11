
#pragma once

#include <stdint.h>
#include <stl/string.h>
#include <stl/memory.h>
#include <stl/map.h>
#include <stl/tuple.h>
#include <tecs/SparsePool.h>
#include <tecs/Forward.h>
#include <tecs/Entity.h>
#include <tecs/Buffer.h>

namespace mud
{
	using Typemap = vector<uint32_t>;

	class BufferArray
	{
	public:
		BufferArray();
		BufferArray(Typemap& typemap, uint32_t size = 0);

		BufferArray(BufferArray&& other) = default;
		BufferArray& operator=(BufferArray&& other) = default;

		BufferArray(const BufferArray& other) = delete;
		BufferArray& operator=(const BufferArray& other) = delete;

		void add_buffer(uint32_t index, unique<Buffer> buffer)
		{
			m_buffers.push_back(move(buffer));
			m_buffer_map[index] = &(*m_buffers.back());
		}

		template <class T>
		uint32_t type_index();

		template <class T>
		void add_buffer();

		template <class... Types>
		void init();

		template <class T>
		TBuffer<T>& buffer();

		uint32_t size() const;

		uint32_t reverse(uint32_t index) const;
		uint32_t handle(uint32_t index) const;

		void ensure(uint32_t capacity);

		void clear();
		uint32_t create();
		uint32_t create(uint32_t count);
		void add();
		void remove(uint32_t handle);

		template <class T>
		void set(uint32_t handle, T component = T());

		template <class T>
		T& get(uint32_t handle);

		Typemap* m_typemap = nullptr;

		SparseHandles m_handles;

		vector<unique<Buffer>> m_buffers;
		vector<Buffer*> m_buffer_map;
	};

	using cstring = const char*;

	class EntityStream : public BufferArray
	{
	public:
		EntityStream();
		EntityStream(cstring name, uint16_t index, Typemap& typemap, uint32_t size = 0);

		template <class... Types>
		void init(uint64_t prototype);

		cstring m_name;
		uint16_t m_index;
		uint64_t m_prototype;
	};

	class GridECS : public BufferArray
	{
	public:
		Typemap m_typemap;
		vector<uint32_t> m_available;

	public:
		GridECS();

		uint32_t create();
		void destroy(uint32_t handle);
	};

	class ECS
	{
	public:
		uint8_t m_index = 0;
		uint32_t m_type_index = 0;
		Typemap m_typemap;

		vector<EntityStream> m_streams;
		map<uint64_t, uint16_t> m_stream_map;

	public:
		ECS(int capacity = 1 << 10);

		ECS(const ECS& other) = delete;
		ECS& operator=(const ECS& other) = delete;

		template <class T>
		uint32_t type_index();

		template <class T>
		uint32_t type_mask();

		template <class... Types>
		uint64_t prototype();

		template <class... Types>
		EntityStream& stream();

		EntityStream& stream(uint32_t handle);

		vector<EntityStream*> match(uint64_t prototype);

		template <class... Types>
		void add_stream(cstring name);

#ifdef MUD_ECS_TYPED
		template <class T>
		void register_type();

		template <class... Types>
		void add_stream();
#endif

		uint32_t create(uint64_t prototype);

		template <class... Types>
		Entity create();

		void destroy(Entity handle);

		template <class T>
		void set(Entity handle, T component = T());

		template <class T>
		bool has(Entity handle);

		template <class T>
		T& get(Entity handle);

		template <class T, class... Types>
		vector<T*> gather();

		template <class... Types, class T_Function>
		void loop(T_Function action);

		template <class... Types, class T_Function>
		void loop_ent(T_Function action);
	};

	template <unsigned EcsType, unsigned NumComponents>
	class tECS : public ECS
	{
	public:
		inline void add_buffer(EntityStream& stream, uint64_t prototype, uint32_t index, unique<Buffer> buffer)
		{
			if(prototype & (1ULL << index) != 0)
				stream.add_buffer(index, move(buffer));
		}

		template <uint32_t... Is>
		inline void init_stream(EntityStream& stream, uint64_t prototype, index_sequence<Is...>)
		{
			swallow{ (this->add_buffer(stream, prototype, Is, TypeBuffer<EcsType, Is>::create()), 0)... };
		}

		inline void add_stream(uint64_t prototype, string name = "")
		{
			uint16_t index = uint16_t(m_streams.size());
			m_streams.push_back({ name.c_str(), index, m_typemap });
			m_stream_map[prototype] = index;
			EntityStream& stream = m_streams.back();
			stream.m_prototype = prototype;
			this->init_stream(stream, prototype, index_tuple<NumComponents>());
		}

		inline EntityStream& stream(uint64_t prototype)
		{
			if(m_stream_map.find(prototype) == m_stream_map.end())
				this->add_stream(prototype);
			uint16_t stream = m_stream_map[prototype];
			return m_streams[stream];
		}

		inline Entity create(uint64_t prototype)
		{
			EntityStream& stream = this->stream(prototype);
			uint32_t handle = stream.create();
			return { m_index, stream.m_index, handle };
		}

		inline Entity create(uint64_t prototype, uint32_t count)
		{
			EntityStream& stream = this->stream(prototype);
			uint32_t handle = stream.create(count);
			return { m_index, stream.m_index, handle };
		}
	};

	export_ extern MUD_ECS_EXPORT ECS* s_ecs[256];

	export_ template <class T>
	inline bool isa(const Entity& entity) { return s_ecs[entity.m_ecs]->has<T>(entity.m_handle); }

	export_ template <class T>
	inline T& asa(const Entity& entity) { return s_ecs[entity.m_ecs]->get<T>(entity.m_handle); }

	export_ template <class T>
	inline T* try_asa(const Entity& entity) { if(entity && isa<T>(entity)) return &asa<T>(entity); else return nullptr; }
	
	export_ template <class T>
	inline T* try_asa(const Entity* entity) { if(entity && isa<T>(*entity)) return &asa<T>(*entity); else return nullptr; }

#ifdef MUD_ECS_TYPED
	struct EntityRef {};

	export_ template <> MUD_ECS_EXPORT Type& type<EntityRef>();

	inline Ref ent_ref(uint32_t entity) { return Ref((void*)uintptr_t(entity), type<EntityRef>()); }
	inline uint32_t as_ent(const Ref& ref) { return ref.m_type->is<EntityRef>() ? uint32_t((uintptr_t)ref.m_value) : UINT32_MAX; }
#endif

	inline cstring entity_prototype(const Entity& entity)
	{
		EntityStream& stream = s_ecs[entity.m_ecs]->stream(entity.m_handle);
		return stream.m_name;
	}

	template <class T>
	struct refl_ struct_ ComponentHandle : public Entity
	{
		ComponentHandle();
		ComponentHandle(uint32_t handle, uint32_t stream);
		ComponentHandle(const Entity& entity);

		operator T&();
		operator const T&() const;

		T* operator->();
		T& operator*();
		const T* operator->() const;
		const T& operator*() const;
	};

	template <class T>
	struct refl_ struct_ nocopy_ EntityHandle : public ComponentHandle<T>
	{
		EntityHandle();
		EntityHandle(uint32_t handle, uint32_t stream);
		~EntityHandle();

		EntityHandle(EntityHandle<T>& other) = delete;
		EntityHandle& operator=(EntityHandle<T>& other) = delete;

		EntityHandle(EntityHandle<T>&& other);
		EntityHandle& operator=(EntityHandle<T>&& other);

		operator Entity() const;
	};
}