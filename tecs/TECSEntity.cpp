//  Copyright (c) 2019 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#ifdef MUD_MODULES
module mud.ecs;
#else
#include <tecs/Entity.h>
#include <tecs/ECS.h>
#include <tecs/ECS.hpp>
#endif

namespace mud
{
	ECS* s_ecs[256] = {};

	void Entity::destroy()
	{
		if(m_handle != UINT32_MAX)
			s_ecs[m_ecs]->destroy(*this);
	}
}
