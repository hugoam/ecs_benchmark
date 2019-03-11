#ifndef USE_STL
#ifdef MUD_MODULES
module mud.ecs;
#else
#include <stl/vector.hpp>
#include <stl/unordered_map.hpp>
#include <tecs/Api.h>
#include <tecs/ECS.hpp>
#endif

namespace stl
{
	using namespace mud;
	template class MUD_ECS_EXPORT vector<unsigned int>;
	template class MUD_ECS_EXPORT vector<Buffer*>;
	template class MUD_ECS_EXPORT vector<EntityStream*>;
	//template class MUD_ECS_EXPORT vector<EntityData>;
	template class MUD_ECS_EXPORT vector<EntityStream>;
	template class MUD_ECS_EXPORT vector<unique<Buffer>>;
	template class MUD_ECS_EXPORT unordered_map<uint64_t, unsigned short>;
	template class MUD_ECS_EXPORT unordered_map<unsigned int, unsigned int>;
}
#endif
