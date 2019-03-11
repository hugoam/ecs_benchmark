#include <tecs/ECS.hpp>

#include <ctime>

#include <stl/vector.hpp>
#include <stl/string.hpp>

enum EcsIndex
{
	Main
};

namespace mud
{
	template <> struct TypeBuffer<Main, 0>  { static unique<Buffer> create() { return construct<TBuffer<Position>>(); } };
	template <> struct TypeBuffer<Main, 1>  { static unique<Buffer> create() { return construct<TBuffer<Rotation>>(); } };
	template <> struct TypeBuffer<Main, 2>  { static unique<Buffer> create() { return construct<TBuffer<Velocity>>(); } };
	template <> struct TypeBuffer<Main, 3>  { static unique<Buffer> create() { return construct<TBuffer<Mass>>(); } };
	template <> struct TypeBuffer<Main, 4>  { static unique<Buffer> create() { return construct<TBuffer<Color>>(); } };
	template <> struct TypeBuffer<Main, 5>  { static unique<Buffer> create() { return construct<TBuffer<Battery>>(); } };
	template <> struct TypeBuffer<Main, 6>  { static unique<Buffer> create() { return construct<TBuffer<Damage>>(); } };
	template <> struct TypeBuffer<Main, 7>  { static unique<Buffer> create() { return construct<TBuffer<Health>>(); } };
	template <> struct TypeBuffer<Main, 8>  { static unique<Buffer> create() { return construct<TBuffer<Attack>>(); } };
	template <> struct TypeBuffer<Main, 9>  { static unique<Buffer> create() { return construct<TBuffer<Defense>>(); } };
	template <> struct TypeBuffer<Main, 10> { static unique<Buffer> create() { return construct<TBuffer<Stamina>>(); } };
	template <> struct TypeBuffer<Main, 11> { static unique<Buffer> create() { return construct<TBuffer<Strength>>(); } };
	template <> struct TypeBuffer<Main, 12> { static unique<Buffer> create() { return construct<TBuffer<Agility>>(); } };
	template <> struct TypeBuffer<Main, 13> { static unique<Buffer> create() { return construct<TBuffer<Intelligence>>(); } };
	
	template <> struct TypedBuffer<Position>     { static size_t index() { return 0; } };
	template <> struct TypedBuffer<Rotation>     { static size_t index() { return 1; } };
	template <> struct TypedBuffer<Velocity>     { static size_t index() { return 2; } };
	template <> struct TypedBuffer<Mass>         { static size_t index() { return 3; } };
	template <> struct TypedBuffer<Color>        { static size_t index() { return 4; } };
	template <> struct TypedBuffer<Battery>      { static size_t index() { return 5; } };
	template <> struct TypedBuffer<Damage>       { static size_t index() { return 6; } };
	template <> struct TypedBuffer<Health>       { static size_t index() { return 7; } };
	template <> struct TypedBuffer<Attack>       { static size_t index() { return 8; } };
	template <> struct TypedBuffer<Defense>      { static size_t index() { return 9; } };
	template <> struct TypedBuffer<Stamina>      { static size_t index() { return 10; } };
	template <> struct TypedBuffer<Strength>     { static size_t index() { return 11; } };
	template <> struct TypedBuffer<Agility>      { static size_t index() { return 12; } };
	template <> struct TypedBuffer<Intelligence> { static size_t index() { return 13; } };
}

using ECS = mud::tECS<Main, 13>;

double bench_create_empty_tecs(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<>();
	ecs.stream(prototype).ensure(n);
	for(int i = 0; i < n; i++) {
		ecs.create(prototype);
	}
	double result = timespec_measure(&start);
	return result;
}

double bench_create_empty_tecs_batch(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<>();
	ecs.stream(prototype).ensure(n);
	ecs.create(prototype, n);
	double result = timespec_measure(&start);
	return result;
}

double bench_create_1component_tecs(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<Position>();
	for(int i = 0; i < n; i++) {
		ecs.create(prototype);
	}
	double result = timespec_measure(&start);

	return result;
}

double bench_create_1component_tecs_batch(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<Position>();
	ecs.create(prototype, n);
	double result = timespec_measure(&start);

	return result;
}

double bench_create_2component_tecs(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<Position, Velocity>();
	for(std::uint64_t i = 0; i < n; i++) {
		ecs.create(prototype);
	}
	double result = timespec_measure(&start);

	return result;
}

double bench_create_2component_tecs_batch(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<Position, Velocity>();
	ecs.create(prototype, n);
	double result = timespec_measure(&start);

	return result;
}

double bench_create_3component_tecs(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<Position, Velocity, Mass>();
	for(std::uint64_t i = 0; i < n; i++) {
		ecs.create(prototype);
	}
	double result = timespec_measure(&start);

	return result;
}

double bench_create_3component_tecs_batch(int n) {
	ECS ecs;

	struct timespec start; timespec_gettime(&start);
	uint64_t prototype = ecs.prototype<Position, Velocity, Mass>();
	ecs.create(prototype, n);
	double result = timespec_measure(&start);

	return result;
}

template <typename... Components, typename Func>
double iterate_view(ECS& ecs, int n_iter, Func func) {
	double result = 0;

	for(int i = 0; i < n_iter; i++) {
		struct timespec start; timespec_gettime(&start);
		ecs.loop<Components...>(func);
		double t = timespec_measure(&start);
		if(!result || (t < result)) {
			result = t;
		}
	}

	return result;
}

void create_pathological(ECS& ecs, std::vector< std::vector <int>> entity_list) {

    for (std::vector<int>& component_list: entity_list) {
		uint64_t prototype = 0U;

		prototype |= ecs.type_mask<Position>();
		prototype |= ecs.type_mask<Velocity>();
		prototype |= ecs.type_mask<Mass>();
		prototype |= ecs.type_mask<Damage>();

        for (int c: component_list) {
            switch(c) {
            case 0:
				prototype |= ecs.type_mask<Stamina>();
                break;
            case 1:
				prototype |= ecs.type_mask<Strength>();
                break;
            case 2:
				prototype |= ecs.type_mask<Agility>();
                break;
            case 3:
				prototype |= ecs.type_mask<Intelligence>();
                break;
            case 4:
				prototype |= ecs.type_mask<Color>();
                break;
            case 5:
                prototype |= ecs.type_mask<Battery>();
                break;
            case 6:
				prototype |= ecs.type_mask<Rotation>();
                break;
            case 7:
				prototype |= ecs.type_mask<Health>();
                break;
            case 8:
				prototype |= ecs.type_mask<Attack>();
                break;
            case 9:
				prototype |= ecs.type_mask<Defense>();
                break;
            }
        }

		uint32_t e = ecs.create(prototype);
    }
}

double bench_iter_one_pathological_tecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    ECS ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position>(ecs, n_iter, [](Position& p) {
        p.x ++;
        p.y ++;
    });
}

double bench_iter_two_pathological_tecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    ECS ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position, Velocity>(ecs, n_iter, [](Position& p, Velocity& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_three_pathological_tecs(int n_iter, std::vector< std::vector<int>> entity_list) {
	ECS ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position, Velocity, Mass>(ecs, n_iter, [](Position& p, Velocity& v, Mass& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_four_pathological_tecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    ECS ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position, Velocity, Mass, Damage>(ecs, n_iter, [](Position& p, Velocity& v, Mass& m, Damage& d) {
        p.x += v.x / m.value / d.value;
        p.y += v.y / m.value / d.value;
    });
}

#if 0
double bench_delete_1component_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
    }

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.destroy(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_add_one_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		ecs.create();
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.assign<Position>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_add_two_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		ecs.create();
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_add_three_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		ecs.create();
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_add_four_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		ecs.create();
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
        ecs.assign<Color>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_remove_one_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		const auto entity = ecs.create();
        ecs.assign<Position>(entity);
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.remove<Position>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_remove_two_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.remove<Position>(entity);
        ecs.remove<Velocity>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_remove_three_tecs(int n) {
    tecs::registry<std::uint64_t> ecs;

	for (int i = 0; i < n; i++) {
		const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
	}

    struct timespec start; timespec_gettime(&start);
    ecs.each([&ecs](auto entity) {
        ecs.remove<Position>(entity);
        ecs.remove<Velocity>(entity);
        ecs.remove<Mass>(entity);
    });
    double result = timespec_measure(&start);

    return result;
}

double bench_iter_one_tecs_view(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
    }

    return iterate_view<Position>(ecs, n_iter, [](auto& p) {
        p.x ++;
        p.y ++;
    });
}

double bench_iter_two_tecs_view(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
    }

    return iterate_view<Position, Velocity>(ecs, n_iter, [](auto& p, auto& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_three_two_types_tecs_view(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
        if (i < n / 2) {
            ecs.assign<Rotation>(entity);
        }
    }

    return iterate_view<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_two_tecs_group(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity>();

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
    }

    return iterate_group<Position, Velocity>(ecs, n_iter, [](auto& p, auto& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_three_two_types_tecs_group(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity, Mass>();

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
        if (i < n / 2) {
            ecs.assign<Rotation>(entity);
        }
    }

    return iterate_group<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_three_tecs_view(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
    }

    return iterate_view<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_three_tecs_group(int n, int n_iter) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity, Mass>();

    for(std::uint64_t i = 0; i < n; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Velocity>(entity);
        ecs.assign<Mass>(entity);
    }

    return iterate_group<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

void create_eight_types(tecs::registry<std::uint64_t>& ecs, std::uint64_t count, bool match_all) {
    for(std::uint64_t i = 0; i < count; i++) {
        const auto entity = ecs.create();
        ecs.assign<Position>(entity);
        ecs.assign<Mass>(entity);
        ecs.assign<Damage>(entity);
        if ((i % 8) == 1) {
            ecs.assign<Rotation>(entity);
        } else 
        if ((i % 8) == 2) {
            ecs.assign<Color>(entity);
        } else 
        if ((i % 8) == 3) {
            ecs.assign<Battery>(entity);
        } else 
        if ((i % 8) == 4) {
            ecs.assign<Rotation>(entity);
            ecs.assign<Color>(entity);
        } else 
        if ((i % 8) == 5) {
            ecs.assign<Rotation>(entity);
            ecs.assign<Battery>(entity);
        } else 
        if ((i % 8) == 6) {
            ecs.assign<Battery>(entity);
            ecs.assign<Color>(entity);
        } else 
        if ((i % 8) == 7) {
            ecs.assign<Rotation>(entity);
            ecs.assign<Battery>(entity);
            ecs.assign<Color>(entity);
        }

        if (match_all || i % 2) {
            ecs.assign<Velocity>(entity);
        }
    }
}

double bench_iter_two_eight_types_tecs_view(int n, int n_iter, bool match_all) {
    tecs::registry<std::uint64_t> ecs;

    int count = n * (2 - match_all);

    create_eight_types(ecs, count, match_all);

    return iterate_view<Position, Velocity>(ecs, n_iter, [](auto& p, auto& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_three_eight_types_tecs_view(int n, int n_iter, bool match_all) {
    tecs::registry<std::uint64_t> ecs;

    int count = n * (2 - match_all);

    create_eight_types(ecs, count, match_all);

    return iterate_view<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_two_eight_types_tecs_group(int n, int n_iter, bool match_all) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity>();

    int count = n * (2 - match_all);

    create_eight_types(ecs, count, match_all);

    return iterate_group<Position, Velocity>(ecs, n_iter, [](auto& p, auto& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_three_eight_types_tecs_group(int n, int n_iter, bool match_all) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity, Mass>();

    int count = n * (2 - match_all);

    create_eight_types(ecs, count, match_all);

    return iterate_group<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_four_eight_types_tecs_view(int n, int n_iter, bool match_all) {
    tecs::registry<std::uint64_t> ecs;

    int count = n * (2 - match_all);

    create_eight_types(ecs, count, match_all);

    return iterate_view<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_four_eight_types_tecs_group(int n, int n_iter, bool match_all) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity, Mass, Damage>();

    int count = n * (2 - match_all);

    create_eight_types(ecs, count, match_all);

    return iterate_group<Position, Velocity, Mass, Damage>(ecs, n_iter, [](auto& p, auto& v, auto& m, auto& d) {
        p.x += v.x / m.value / d.value;
        p.y += v.y / m.value / d.value;
    });
}

void create_pathological(tecs::registry<std::uint64_t>& ecs, std::vector< std::vector <int>> entity_list) {

    for (std::vector<int>& component_list: entity_list) {
        const auto e = ecs.create();
        ecs.assign<Position>(e);
        ecs.assign<Velocity>(e);
        ecs.assign<Mass>(e);
        ecs.assign<Damage>(e);

        for (int c: component_list) {
            switch(c) {
            case 0:
                ecs.assign<Stamina>(e);
                break;
            case 1:
                ecs.assign<Strength>(e);
                break;
            case 2:
                ecs.assign<Agility>(e);
                break;
            case 3:
                ecs.assign<Intelligence>(e);
                break;
            case 4:
                ecs.assign<Color>(e);
                break;
            case 5:
                ecs.assign<Battery>(e);
                break;
            case 6:
                ecs.assign<Rotation>(e);
                break;
            case 7:
                ecs.assign<Health>(e);
                break;
            case 8:
                ecs.assign<Attack>(e);
                break;
            case 9:
                ecs.assign<Defense>(e);
                break;
            }
        }
    }
}


double bench_iter_one_pathological_tecs(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position>(ecs, n_iter, [](auto& p) {
        p.x ++;
        p.y ++;
    });
}

double bench_iter_two_pathological_tecs_view(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position, Velocity>(ecs, n_iter, [](auto& p, auto& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_two_pathological_tecs_group(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity>();

    create_pathological(ecs, entity_list);

    return iterate_group<Position, Velocity>(ecs, n_iter, [](auto& p, auto& v) {
        p.x += v.x;
        p.y += v.y;
    });
}

double bench_iter_three_pathological_tecs_view(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_three_pathological_tecs_group(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity, Mass>();

    create_pathological(ecs, entity_list);

    return iterate_group<Position, Velocity, Mass>(ecs, n_iter, [](auto& p, auto& v, auto& m) {
        p.x += v.x / m.value;
        p.y += v.y / m.value;
    });
}

double bench_iter_four_pathological_tecs_view(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;

    create_pathological(ecs, entity_list);

    return iterate_view<Position, Velocity, Mass, Damage>(ecs, n_iter, [](auto& p, auto& v, auto& m, auto& d) {
        p.x += v.x / m.value / d.value;
        p.y += v.y / m.value / d.value;
    });
}

double bench_iter_four_pathological_tecs_group(int n_iter, std::vector< std::vector<int>> entity_list) {
    tecs::registry<std::uint64_t> ecs;
    ecs.group<Position, Velocity, Mass, Damage>();

    create_pathological(ecs, entity_list);

    return iterate_group<Position, Velocity, Mass, Damage>(ecs, n_iter, [](auto& p, auto& v, auto& m, auto& d) {
        p.x += v.x / m.value / d.value;
        p.y += v.y / m.value / d.value;
    });
}
#endif
