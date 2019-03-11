#pragma once

#define export_

#define refl_
#define struct_

#define MUD_POOL_EXPORT
#define MUD_ECS_EXPORT

#define UNUSED(x) (void)x

namespace mud
{
	template <class T> struct ComponentHandle;
	template <class T> struct EntityHandle;

    class Prototype;
	struct Entity;
	class ECS;
	class GridECS;
	class Complex;
}

#ifdef MUD_META_GENERATOR
#include <stl/vector.h>
namespace stl
{
}
#endif
