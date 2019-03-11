#pragma once

#include "stdint.h"
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <chrono>
#include "plf_colony.h"
#include <cassert>
#include <deque>
#include <algorithm>
#include <execution>
using ComponentGUID = uint64_t;

struct Metatype {

	using ConstructorFn = void(void*);
	using DestructorFn = void(void*);

	size_t name_hash{0};
	size_t size{0};
	size_t align{0};
	const char * name{"none"};
	ConstructorFn *constructor;
	DestructorFn *destructor;

	template<typename T>
	constexpr static Metatype BuildMetatype() {

		Metatype meta;

		meta.name_hash = typeid(T).hash_code(); 
		meta.name = typeid(T).name();
		if ( std::is_empty<T>::value )
		{
			meta.size = 0;
		}
		else {
			meta.size = sizeof(T);
		}
	
		meta.align = alignof(T);

		meta.constructor = [](void* p) {
			new(p) T{};
		};
		meta.destructor = [](void* p) {
			((T*)p)->~T();
		};

		return meta;
	}
};



struct BaseComponent {
public:

	using IDCounter = size_t;
protected:

	static IDCounter family_counter_;
};



struct ComponentList {
	std::vector<Metatype> metatypes;

	size_t hash64shift(size_t key)
	{
		key = (~key) + (key << 21); // key = (key << 21) - key - 1;
		key = key ^ (key >> 24);
		key = (key + (key << 3)) + (key << 8); // key * 265
		key = key ^ (key >> 14);
		key = (key + (key << 2)) + (key << 4); // key * 21
		key = key ^ (key >> 28);
		key = key + (key << 31);
		return key;
	};


	inline const uint64_t hash_64_fnv1a(const void* key, const uint64_t len) {

		const char* data = (char*)key;
		uint64_t hash = 0xcbf29ce484222325;
		uint64_t prime = 0x100000001b3;

		for (int i = 0; i < len; ++i) {
			uint8_t value = data[i];
			hash = hash ^ value;
			hash *= prime;
		}

		return hash;

	}
	void sort() {
		std::sort(metatypes.begin(),metatypes.end(),[](auto left, auto right){
		return left.name_hash < right.name_hash;
		});
	}

	void BuildHash() {
		cached_hash = 0;
		and_hash = 0;
		for (auto m : metatypes)
		{
			//size_t hash = hash64shift(m.name_hash);
			cached_hash += m.name_hash;
			size_t keyhash = hash_64_fnv1a(&m.name_hash,sizeof(size_t));
			and_hash |= 0x1L << ((keyhash )% 63);

		}

	}
	std::size_t and_hash{ 0 };
	std::size_t cached_hash{ 0 };
};

struct Archetype {

	Archetype& operator= (const Archetype &rhs) {
		componentlist.metatypes.clear();
		for (auto c : rhs.componentlist.metatypes)
		{
			componentlist.metatypes.push_back(c);
		}
		componentlist.sort();
		componentlist.BuildHash();
		
		return *this;
	}

	//clear constructor
	Archetype() {

	}
	template<typename C>
	void SetAdd(const Archetype &rhs)
	{
		static const Metatype m(Metatype::BuildMetatype<C>());

		//Archetype arc;
		componentlist.metatypes.clear();
		componentlist.metatypes.reserve(rhs.componentlist.metatypes.size() + 1);
		bool added = false;

	

		for (auto c : rhs.componentlist.metatypes)
		{
			componentlist.metatypes.push_back(c);
			if (c.name_hash == m.name_hash) {
				added = true;
			}
		}
		if (!added)
		{
			componentlist.metatypes.push_back(Metatype::BuildMetatype<C>());
		}
		componentlist.sort();
		componentlist.BuildHash();
		//return std::move(arc);
	}
	template<typename C>
	void SetRemove(const Archetype &rhs)
	{
		static const Metatype m(Metatype::BuildMetatype<C>());

		componentlist.metatypes.clear();
		componentlist.metatypes.reserve(rhs.componentlist.metatypes.size());
		
		for (auto c : rhs.componentlist.metatypes)
		{
			if (c.name_hash != m.name_hash) {
				componentlist.metatypes.push_back(c);
			}
		}
		componentlist.sort();
		componentlist.BuildHash();
		//return std::move(arc);
	}
	//registers a new component to the archetype metatypes
	template<typename C>
	void AddComponent()
	{
		Metatype mt = Metatype::BuildMetatype<C>();

		for (auto m : componentlist.metatypes)
		{
			if (m.name_hash == mt.name_hash)
			{
				//already there
				return;
			}
		}

		componentlist.metatypes.push_back(mt);

		componentlist.sort();
		componentlist.BuildHash();
	}
	template<typename C>
	void RemoveComponent()
	{
		Metatype mt = Metatype::BuildMetatype<C>();

		ComponentList newList;
		newList.metatypes.reserve(componentlist.metatypes.size());
		for (auto m : componentlist.metatypes)
		{
			if (m.name_hash != mt.name_hash)
			{
				//already there
				newList.metatypes.push_back(m);
			}
		}

		componentlist = newList;//.metatypes.push_back(mt);
		componentlist.sort();
		componentlist.BuildHash();
	}

	template<typename C>
	bool HasComponent() {
		Metatype mt = Metatype::BuildMetatype<C>();

		for (auto m : componentlist.metatypes)
		{
			if (m.name_hash == mt.name_hash)
			{
				return true;
			}
		}
		return false;
	}
	bool MatchAndHash(const ComponentList & Components) {
		const size_t ListHash = Components.and_hash;
		const size_t ThisHash = componentlist.and_hash;
		const size_t andhash = ListHash & ThisHash;
		return andhash  == ListHash;
	}
	int Match(const ComponentList & Components)
	{
		int matches = 0;
		const auto & A_Met = Components.metatypes;
		const auto & B_Met = componentlist.metatypes;
		int iA = 0;
		int iB = 0;
		while (iA < A_Met.size() && iB < B_Met.size()) {
			const size_t hashA = A_Met[iA].name_hash;
			const size_t hashB = B_Met[iB].name_hash;
			if (hashA == hashB)
			{
				matches++;
				iA++;
				iB++;
			}
			else {
				if (hashA < hashB) {
					iA++;
				}
				else {
					iB++;
				}
			}
		}
		
		return matches;
	}

	bool ExactMatch(const ComponentList & Components) {
		if (Components.cached_hash != componentlist.cached_hash) {
			return false;
		}
		if (Components.metatypes.size() != componentlist.metatypes.size())
		{
			return false;
		}
		//else if (Components.metatypes.size() == 0)
		//{
		//	return true;
		//}
		for (int i = 0; i < Components.metatypes.size(); i++)
		{
			if (Components.metatypes[i].name_hash != componentlist.metatypes[i].name_hash)
			{
				return false;
			}
		}
		//for (auto c : Components.metatypes)
		//{
		//	bool bFound = false;
		//	for (auto a : componentlist.metatypes)
		//	{
		//		if (a.name_hash == c.name_hash)
		//		{
		//			bFound = true;
		//		}
		//	}
		//	if (!bFound)
		//	{
		//		return false;
		//	}
		//
		//}
		return true;
	}


	ComponentList componentlist;

	static constexpr size_t ARRAY_SIZE = 4096;
};



struct ArchetypeComponentArray {
	void * data;	
	Metatype metatype;

	ArchetypeComponentArray(Metatype type) {		
		data = malloc(Archetype::ARRAY_SIZE * type.size);
		
		metatype = type;
	}
	ArchetypeComponentArray(Metatype type, void* alloc) {
		data = alloc;
		metatype = type;
	}
	//ArchetypeComponentArray
	//~ArchetypeComponentArray() {
	//	if (data != nullptr)
	//	{
	//		free(data);
	//	}
	//
	//}
	//copy constructor
	ArchetypeComponentArray(ArchetypeComponentArray& arg) // the expression "arg.member" is lvalue
	{
		Metatype type = arg.metatype;
		data = malloc(Archetype::ARRAY_SIZE * type.size);
		metatype = type;

	}
	// Simple move constructor
	ArchetypeComponentArray(ArchetypeComponentArray&& arg) // the expression "arg.member" is lvalue
	{
		data = arg.data;
		arg.data = nullptr;
		metatype = std::move(arg.metatype);
	}
	// Simple move assignment operator
	ArchetypeComponentArray& operator=(ArchetypeComponentArray&& other) {
		data = other.data;
		other.data = nullptr;
		metatype = std::move(other.metatype);
		return *this;
	}

	void Copy(uint64_t src, uint64_t dst)
	{
		char *psrc = (char*)data + src * metatype.size;
		char *pdst = (char*)data + dst * metatype.size;

		memcpy(pdst, psrc, metatype.size);
	}
	void Create(uint64_t idx) {
		void* ptr = (char*)data + idx * metatype.size;
		metatype.constructor(ptr);
	}
	void Destroy(uint64_t idx) {
		void* ptr = (char*)data + idx * metatype.size;
		metatype.destructor(ptr);
	}
	//copy from a different array
	void CopyFromOuter(uint64_t src, uint64_t dst, ArchetypeComponentArray * other)
	{
		char *psrc = (char*)other->data + src * metatype.size;
		char *pdst = (char*)data + dst * metatype.size;

		memcpy(pdst, psrc, metatype.size);
	}


	template<typename T>
	T &Get(size_t index) {
		assert(Metatype::BuildMetatype<T>().name_hash == metatype.name_hash);

		T * ptr = (T*)data;

		return ptr[index];
	}
};
template<typename T>
struct TypedArchetypeComponentArray {
	T * data{ nullptr };
	TypedArchetypeComponentArray() = default;
	TypedArchetypeComponentArray(const ArchetypeComponentArray &tarray) {
		data = (T*)tarray.data;
	}

	T&Get(size_t index) {
		return data[index];
	}
};
struct EntityHandle {
	size_t id;
	size_t generation;
	EntityHandle() {
		id = -1;
		generation = -1;
	}
	bool operator ==(const EntityHandle &b) const {
		return id == b.id && generation == b.generation;
	}
};
struct ArchetypeBlockStorage;
struct ArchetypeBlock;
struct ECSWorld;
//void ArchetypeBlockStorage::DeleteBlock(ArchetypeBlockStorage * store, ArchetypeBlock * blk);
//16 kilobytes
static const size_t BLOCK_MEMORY = 16384;
struct ArchetypeBlock {
	using byte = unsigned char ;

	size_t canary;
	Archetype myArch;
	uint16_t fullsize;
	void* memory;
	std::vector<ArchetypeComponentArray> componentArrays;

	//handle array
	EntityHandle * entities;
	
	//max index that has an entity
	int16_t last{ 0 };
	//new entities that shouldnt be iterated yet
	int16_t newets{ 0 };
	uint64_t lastIteration{ 0 };	

	ArchetypeBlockStorage * storage;
	ArchetypeBlock(const Archetype &arch) {
		myArch = arch;
		myArch.componentlist.BuildHash();
		
	}
	void InitializeBlock()
	{
		//size in bytes per entity
		uint16_t entity_size = sizeof(EntityHandle);
		for (auto &m : myArch.componentlist.metatypes)
		{
			//ArchetypeComponentArray newArray = ;
			if (m.size != 0)
			{
				entity_size += m.size;
				//componentArrays.push_back(std::move(ArchetypeComponentArray(m)));

			}
		}
		memory = malloc(BLOCK_MEMORY);
		fullsize = (BLOCK_MEMORY / entity_size) - 2;
		byte* alloc = (byte*)memory;
		entities = new(alloc) EntityHandle[fullsize];
		//advance stack allocate
		alloc += sizeof(EntityHandle) * (fullsize + 1);

		for (auto &m : myArch.componentlist.metatypes)
		{
			//ArchetypeComponentArray newArray = ;
			if (m.size != 0)
			{
				//entity_size += m.size;
				componentArrays.push_back(std::move(ArchetypeComponentArray(m, alloc)));
				alloc += m.size * (fullsize + 1);
			}
		}


		canary = 0xDEADBEEF;
		newets = 0;
		//myArch = arch;
		//myArch.componentlist.BuildHash();
		last = 0;
	}

	~ArchetypeBlock()
	{
		last = 0;
		canary = 0;
		if (memory != nullptr)
		{
			free(memory);
		}
		
	}

	// Simple move constructor
	ArchetypeBlock(ArchetypeBlock&& arg) 
	{
		canary = arg.canary;
		myArch = arg.myArch;
		fullsize = arg.fullsize;
		memory = arg.memory;
		arg.memory = nullptr;
		componentArrays = std::move(arg.componentArrays);

		//handle array
		entities = arg.entities;

		//max index that has an entity
		last = arg.last;
		//new entities that shouldnt be iterated yet
		newets= arg.newets;
		lastIteration= arg.lastIteration;
	}
	// Simple copy constructor (move it)
	ArchetypeBlock(const ArchetypeBlock& arg)
	{
		canary = arg.canary;
		myArch = arg.myArch;
		//fullsize = arg.fullsize;
		//memory = arg.memory;
		////arg.memory = nullptr;
		//componentArrays = (arg.componentArrays);
		//
		////handle array
		//entities = arg.entities;
		//
		////max index that has an entity
		//last = arg.last;
		////new entities that shouldnt be iterated yet
		//newets = arg.newets;
		//lastIteration = arg.lastIteration;
	}
	// Simple move assignment operator
	ArchetypeBlock& operator=(ArchetypeBlock&& arg) {
		canary = arg.canary;
		myArch = arg.myArch;
		fullsize = arg.fullsize;
		memory = arg.memory;
		arg.memory = nullptr;
		componentArrays = std::move(arg.componentArrays);

		//handle array
		entities = arg.entities;

		//max index that has an entity
		last = arg.last;
		//new entities that shouldnt be iterated yet
		newets = arg.newets;
		lastIteration = arg.lastIteration;
		return *this;
	}
	bool checkSanity() {
		if (canary != 0xDEADBEEF) return false;
		for (int i = 0; i < fullsize; i++)
		{
			if (entities[i].generation != 1 && i < last)
			{
				//std::cout << "what the fuck";
				return false;
			}
		}
		return true;
	}

	template<typename C>
	TypedArchetypeComponentArray<C> GetComponentArray() {
		Metatype mc = Metatype::BuildMetatype<C>();
		//myArch
		int i = 0;
		for (Metatype & cl : myArch.componentlist.metatypes)
		{
			if (cl.size != 0)
			{
				if (cl.name_hash == mc.name_hash)
				{
					TypedArchetypeComponentArray<C> tarray = TypedArchetypeComponentArray<C>(componentArrays[i]);

					return tarray;
				}
				else
				{
					i++;
				}
			}
		}

		

		return TypedArchetypeComponentArray<C>();
	}

	uint16_t AddEntity(EntityHandle handle)
	{
		uint16_t pos = last + newets;
		assert(pos < fullsize);

		entities[pos] = handle;		

		//last++;
		newets++;
		//entities[last].generation = 7;
		return pos;
	}

	void InitializeEntity(uint64_t entity) {
		for (auto &clist : componentArrays)
		{
			clist.Create(entity);
		}
	}
	void DestroyEntity(uint64_t entity) {
		for (auto &clist : componentArrays)
		{
			clist.Destroy(entity);
		}
	}

	void DestroyAll() {
		for (uint64_t i = 0; i < last + newets; i++)
		{
			for (auto &clist : componentArrays)
			{
				clist.Destroy(i);
			}
		}
		
	}
	//copy a entity from a different block into this one
	void CopyEntityFromBlock(uint64_t destEntity, uint64_t srcEntity, ArchetypeBlock * otherblock)
	{
		//copy components to the index
		for (auto &csrc : otherblock->componentArrays)
		{
			for (auto &cthis : componentArrays)
			{
				//find a component metatype match, and copy it to this block
				if (csrc.metatype.name_hash == cthis.metatype.name_hash)
				{
					cthis.CopyFromOuter(srcEntity, destEntity, &csrc);
					//break first for
					break;
				}
			}
		}

		//copy entity handle	
		entities[destEntity] = otherblock->entities[srcEntity];
		//if (destEntity >= last)
		//{
		//	assert(false);
		//	last++;
		//}
	}

	EntityHandle GetLastEntity() {
		return entities[last - 1];
	}
	int GetSpace() {
		return fullsize - last - newets;
	}
	void refresh(uint64_t iterationIDX) {
		//assert(canary ==)
		assert(canary == 0xDEADBEEF);
		lastIteration = iterationIDX;
		last += newets;
		newets = 0;
	}
	
};


struct ArchetypeBlockStorage {


	Archetype myArch;
	int nblocks;
	ArchetypeBlockStorage(const Archetype & arch) {
		myArch = arch;
		//last = nullptr;
		//first = nullptr;
		nblocks = 0;
		//CreateNewBlock();
		//last = first;
		myArch.componentlist.BuildHash();

	}

	~ArchetypeBlockStorage() {
		//ArchetypeBlock * ptr = first;
		////iterate linked list
		//while (ptr != nullptr)
		//{
		//	auto d = ptr;
		//	ptr = ptr->next;
		//	delete d;			
		//}
	}
	ArchetypeBlock * CreateNewBlock() {
		nblocks++;
		//ArchetypeBlock newblock = ArchetypeBlock(myArch);
		
		auto b = block_colony.insert( myArch );

		ArchetypeBlock * blk = &(*b);
		blk->InitializeBlock();
		//blk->next = nullptr;
		blk->storage = this;

		assert(nblocks == block_colony.size());
		return blk;
	}


	void DeleteBlock(ArchetypeBlock * blk) {

		freeblock = nullptr;


		nblocks--;

		bool bDeleted = false;
		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			//for (auto &it : block_colony)
			//{
			if (&(*it) == blk)
			{
				it->DestroyAll();
				it->last = 0;
				block_colony.erase(it);
				bDeleted = true;
				break;
			}
		}
		assert(bDeleted);
		assert(nblocks == block_colony.size());
	}
	template<typename F>
	void Iterate(F&& f) {

		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			it->refresh(0);
			if (it->last != 0)
			{
				f((*it));
			}

		}
	}
	void AddBlocksToList(std::vector<ArchetypeBlock*>& blocks) {

		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			assert(it->canary == 0xDEADBEEF);
			it->refresh(0);
			if (it->last != 0)
			{

				blocks.push_back(&(*it));
			}

		}
	}

	ArchetypeBlock * FindFreeBlock() {
		//cached freeblock
		if (freeblock != nullptr && freeblock->GetSpace() > 0)
		{
			return freeblock;
		}

		ArchetypeBlock * ptr = nullptr;
		//iterate linked list

		for (auto & b : block_colony)
		{
			ptr = &b;

			if (ptr->GetSpace() > 0)
			{
				freeblock = ptr;
				return ptr;
			}

			//ptr = ptr->next;

		}

		freeblock = nullptr;

		return nullptr;
	}

	ArchetypeBlock * freeblock{ nullptr };
	//ArchetypeBlock * first{ nullptr };
	//ArchetypeBlock * last{ nullptr };

	plf::colony<ArchetypeBlock> block_colony;

	static void DeleteBlock(ArchetypeBlockStorage * store, ArchetypeBlock * blk)
	{
		store->DeleteBlock(blk);
	}
};





struct EntityStorage {
	size_t generation{ 0 };
	//size_t block;
	ArchetypeBlock * block{ nullptr };
	uint16_t blockindex{ 0 };
};


struct ECSWorld {

	bool MatchAndHash(size_t ListHash, size_t MatchHash) {

		const size_t andhash = ListHash & MatchHash;
		return andhash == MatchHash;
	}
	std::vector<ArchetypeBlock*> IterationBlocks;
	
	uint64_t iterationIdx{ 0 };
	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList, const ComponentList& NoneOfList, F && f, bool bParallel = false) {
		iterationIdx++;
		//bParallel = false;
		IterationBlocks.resize(0);
		IterationBlocks.reserve(1000);
		

		bIsIterating = true;
		for (size_t i = 0; i < BlockStorage.size(); i++)
		{
		//block needs to have the same amount of components, and match all of them
			if (MatchAndHash(BlockANDHashes[i], AllOfList.and_hash))
			{
				ArchetypeBlockStorage & b = BlockStorage[i];
					if (b.myArch.MatchAndHash(AllOfList) && b.myArch.Match(AllOfList) == AllOfList.metatypes.size())
					{
						if (b.myArch.Match(NoneOfList) == 0)
						{

							b.AddBlocksToList(IterationBlocks);
								//b.Iterate(f);
								//f(b);
						}
							//b.Iterate(f);
					}
			}
		}

		

		//std::for_each(IterationBlocks.begin(), IterationBlocks.end(), [&](auto bk) {
		//	
		//	assert(bk->canary == 0xDEADBEEF);
		//	//f(*bk);
		//});
		if (bParallel)
		{
			std::for_each(std::execution::par, IterationBlocks.begin(), IterationBlocks.end(), [&](auto bk) {

				assert(bk->last <= bk->fullsize);
				assert(bk->canary == 0xDEADBEEF);
				f(*bk);
			});
		}
		else
		{
			std::for_each(IterationBlocks.begin(), IterationBlocks.end(), [&](auto bk) {
				assert(bk->last <= bk->fullsize);
				assert(bk->canary == 0xDEADBEEF);
				f(*bk);
			});
		}



		bIsIterating = false;
	}
	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList, F && f, bool bParallel = false) {
		iterationIdx++;
		//bParallel = false;
		IterationBlocks.resize(0);
		IterationBlocks.reserve(1000);
		bIsIterating = true;
		size_t hashes = 0;
		size_t matches= 0;
		size_t truematches= 0;
		for (size_t i = 0; i < BlockStorage.size(); i++)
		{
			hashes++;
			//block needs to have the same amount of components, and match all of them
			if (MatchAndHash(BlockANDHashes[i], AllOfList.and_hash))
			{			
				matches++;
				ArchetypeBlockStorage & b = BlockStorage[i];
				if (b.myArch.MatchAndHash(AllOfList) && b.myArch.Match(AllOfList) == AllOfList.metatypes.size())
				{
					truematches++;
					b.AddBlocksToList(IterationBlocks);
					//	b.Iterate(f);
				}
			}
		}
		//std::cout << "Iterations: " << hashes << "Matches: " << matches <<" True matches:" <<truematches  << std::endl;
		//return;

		if (bParallel)
		{
			std::for_each(std::execution::par, IterationBlocks.begin(), IterationBlocks.end(), [&](auto bk) {
				assert(bk->last <= bk->fullsize);
				assert(bk->canary == 0xDEADBEEF);
				f(*bk);
			});
		}
		else
		{
			std::for_each(IterationBlocks.begin(), IterationBlocks.end(), [&](auto bk) {
				assert(bk->last <= bk->fullsize);
				assert(bk->canary == 0xDEADBEEF);
				f(*bk);
			});
		}




		bIsIterating = false;
	}
	bool ValidateAll() {
		bool valid = true;
		for (ArchetypeBlockStorage & b : BlockStorage)
		{
			b.Iterate([&](auto & blk) {
				bool b = blk.checkSanity();
				if (b == false)
				{
					valid = false;
				}
			});
		}
		return valid;

	}

	ArchetypeBlock * FindOrCreateBlockForArchetype(const Archetype & arc)
	{
		ArchetypeBlock * entityBlock = nullptr;
		//find the free block	
		const size_t numComponents = arc.componentlist.metatypes.size();
		
		const size_t blocksize = BlockStorage.size();
		//for (ArchetypeBlockStorage & b : BlockStorage)
		for (size_t i = 0; i < blocksize; i++)
		{			
			//block needs to have the same amount of components, and match all of them
			if(BlockHashes[i] == arc.componentlist.cached_hash)
			{
				ArchetypeBlockStorage * b = &BlockStorage[i];
				if (b->myArch.ExactMatch(arc.componentlist))
				{
					entityBlock = b->FindFreeBlock();
					if (entityBlock == nullptr)
					{
						entityBlock = b->CreateNewBlock();
						break;

					}
				}
			}
		}
		//block not found, create a new one
		if (entityBlock == nullptr)
		{
			
			//auto str = ArchetypeBlockStorage(arc);
			BlockStorage.push_back(ArchetypeBlockStorage(arc));
			BlockHashes.push_back(arc.componentlist.cached_hash);
			BlockANDHashes.push_back(arc.componentlist.and_hash);
			entityBlock = BlockStorage[BlockStorage.size() - 1].CreateNewBlock();
			//entityBlock = CreateBlock(arc);
		}

		return entityBlock;
	}

	std::vector<EntityHandle> CreateEntityBatched(Archetype & arc, size_t amount)
	{
		arc.componentlist.BuildHash();
		//Entities.reserve(Entities.size() + amount);
		std::vector<EntityHandle> Handles;
		Handles.reserve(amount);
		size_t amount_left = amount;
		bool bCanReuseEntities = CanReuseEntity();
		int reuses = deletedEntities.size();
		int newcreations = amount - reuses;
		if (newcreations > 0)
		{
			//Entities.reserve(Entities.size() + newcreations);
		}

		while (amount_left > 0)
		{
			ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);
			int freespace = entityBlock->GetSpace();
			for (int i = 0; i < freespace; i++) {

				if (amount_left <= 0)
				{
					return Handles;
				}
				EntityHandle newEntity;
				bool bReuse = CanReuseEntity();
				size_t index = 0;
				if (bCanReuseEntities && bReuse) {

					index = deletedEntities.front();
					deletedEntities.pop_front();
					assert(Entities[index].block == nullptr);
					newEntity.id = index;
					newEntity.generation = Entities[index].generation + 1;
				}
				else
				{
					bCanReuseEntities = false;
					newEntity.id = Entities.size();
					index = newEntity.id;
					newEntity.generation = 1;
				}

				uint16_t pos = entityBlock->AddEntity(newEntity);
				entityBlock->InitializeEntity(pos);

				EntityStorage et;
				et.block = entityBlock;
				et.blockindex = pos;
				et.generation = newEntity.generation;

				if (bReuse) {
					Entities[index] = et;
				}
				else {
					Entities.push_back(et);
				}

				//Entities.push_back(et);
				Handles.push_back(newEntity);
				amount_left--;
			}
		}
		return Handles;
	}
	EntityHandle CreateEntity(Archetype & arc) {
		arc.componentlist.BuildHash();
		EntityHandle newEntity;

		bool bReuse = CanReuseEntity();
		size_t index = 0;
		if (bReuse) {
			
			index = deletedEntities.front();
			assert(Entities[index].block == nullptr);
			deletedEntities.pop_front();
			newEntity.id = index;
			newEntity.generation = Entities[index].generation + 1;
		}
		else
		{
			newEntity.id = Entities.size();
			index = Entities.size();
			newEntity.generation = 1;
		}


		ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);

		//insert entity handle into the block
		uint16_t pos = entityBlock->AddEntity(newEntity);
		entityBlock->InitializeEntity(pos);

		EntityStorage et;
		et.block = entityBlock;
		et.blockindex = pos;
		et.generation = newEntity.generation;

		if (bReuse) {
			Entities[index] = et;
		}
		else {

			Entities.push_back(et);
			index = Entities.size() - 1;
		}

		
		ArchetypeBlock* block = Entities[newEntity.id].block;
		auto blockidx = Entities[newEntity.id].blockindex;
		assert(block->entities[blockidx] == newEntity);
		return newEntity;
	}

	template<typename C>
	void AddComponent(EntityHandle entity)		//, C&comp)
	{
		if (Valid(entity)) {
			const EntityStorage & et = Entities[entity.id];
			//valid entity

			static Archetype cached;

			cached.SetAdd<C>(et.block->myArch);

			//static Archetype newarch = et.block->myArch;
			//newarch.AddComponent<C>();
			//Archetype newarch = Archetype::CreateAdd<C>(et.block->myArch);
			SetEntityArchetype(entity, cached);// newarch);
			new(&GetComponent<C>(entity)) C{};
		}
	}
	template<typename C>
	void HasComponent(EntityHandle entity)
	{
		if (Valid(entity)) {
			const EntityStorage & et = Entities[entity.id];
			return et.block->myArch->HasComponent<C>();
		}
		else {
			return false;
		}
	}


	template<typename C>
	void RemoveComponent(EntityHandle entity)		//, C&comp)
	{
		const EntityStorage & et = Entities[entity.id];
		//valid entity
		if (Valid(entity)) {

			static Archetype cached;
			cached.SetRemove<C>(et.block->myArch);

			//Archetype newarch = Archetype::CreateRemove<C>(et.block->myArch);// et.block->myArch;
			//newarch.RemoveComponent<C>();

			GetComponent<C>(entity).~C();
			SetEntityArchetype(entity, cached);
			//ValidateAll();

		}
	}

	void DeleteEntity(EntityHandle entity) {
		assert(Valid(entity));
		EntityStorage & et = Entities[entity.id];

		//delete the entity from its block
		ArchetypeBlock * oldBlock = et.block;
		oldBlock->refresh(0);
		auto oldpos = et.blockindex;
		EntityHandle Swapped;

		oldBlock->DestroyEntity(oldpos);
		if (RemoveAndSwapFromBlock(oldpos, *oldBlock, Swapped))
		{
			oldBlock->storage->DeleteBlock(oldBlock);
		}
		if (Valid(Swapped, false))
		{
			Entities[Swapped.id].blockindex = oldpos;
		}
		
		deletedEntities.push_back(entity.id);

		et.block = nullptr;
		et.blockindex = 0;
		et.generation++;
	}

	//returns true if the block got deleted
	bool RemoveAndSwapFromBlock(uint64_t idx, ArchetypeBlock & Block, EntityHandle & SwappedEntity) {
		assert(Block.entities[idx].generation != -1);
		assert(idx < Block.last + Block.newets);
		//shrink
		//if (newets != 0)
		{
			//	newets--;
		}
		bool bIsInitialized = false;
		if (idx < Block.last)
		{
			bIsInitialized = true;
		}
		uint16_t swap_entity = (Block.last + Block.newets) - 1;

		

		if (bIsInitialized)
		{
			Block.last--;
		}
		else {
			Block.newets--;
		}


		assert(Block.canary == 0xDEADBEEF);

		if (Block.last <= 0 && Block.newets <= 0)
		{
			//block emptied

			return true;
		}
		assert(idx <= swap_entity);
		if (idx == swap_entity)
		{
			//assert(false);
			Block.entities[swap_entity].generation = -1;
			Block.entities[swap_entity].id = -1;
			return false;
		}

		//copy components to the index
		for (auto &ca : Block.componentArrays)
		{
			ca.Copy(swap_entity, idx);
		}

		//copy entity handle
		Block.entities[idx] = Block.entities[swap_entity];

		Entities[Block.entities[idx].id].blockindex = idx;

		SwappedEntity = Block.entities[swap_entity];
		Block.entities[swap_entity].generation = -1;
		Block.entities[swap_entity].id = -1;
		return false;
	}

	bool Valid(EntityHandle entity, bool perform_asserts = true)
	{
		const bool bEnableAsserts = false;
		perform_asserts = perform_asserts && bEnableAsserts;

		if (perform_asserts) assert(entity.id < Entities.size());
		if (entity.id >= Entities.size()) return false;
		const EntityStorage & et = Entities[entity.id];
		if (et.block != nullptr)
		{
			if (perform_asserts) assert(et.block->canary == 0xDEADBEEF);
			if (et.block->canary != 0xDEADBEEF) return false;
		}
		if (perform_asserts)assert(et.blockindex <= et.block->fullsize);
		if (perform_asserts)assert(et.blockindex >= 0);

		//valid entity
		return (et.generation == entity.generation  && et.block != nullptr && et.block->last + et.block->newets <= Archetype::ARRAY_SIZE);

	}

	void SetEntityArchetype(EntityHandle entity, Archetype &arc) {

		EntityStorage & et = Entities[entity.id];

		assert(Valid(entity));

		ArchetypeBlock * oldBlock = et.block;
		if (oldBlock->myArch.ExactMatch(arc.componentlist))
		{
			return;
		}

		assert(oldBlock->canary == 0xDEADBEEF);

		ArchetypeBlock * newBlock = FindOrCreateBlockForArchetype(arc);

		//create entity in the new block
		auto pos = newBlock->AddEntity(entity);
		auto oldpos = et.blockindex;
		//copy old block entity into new block entity
		newBlock->CopyEntityFromBlock(pos, oldpos, oldBlock);

		//clear old entity slot
		EntityHandle SwapEntity;// = oldBlock->GetLastEntity();





		if (RemoveAndSwapFromBlock(oldpos, *oldBlock, SwapEntity))
		{
			//if (bIsIterating)
			//{
			//	bWantsDeletes = true;
			//}
			//else
			//{
			auto storage = oldBlock->storage;
			storage->DeleteBlock(oldBlock);
			//}
			//oldBlock->storage->DeleteBlock(oldBlock);	
		}
		if (Valid(SwapEntity, false))
		{
			Entities[SwapEntity.id].blockindex = oldpos;
		}
		//update the low level data
		et.block = newBlock;
		et.blockindex = pos;

	}

	bool CanReuseEntity() {
		return !deletedEntities.empty();
	}

	template<typename Component>
	Component & GetComponent(EntityHandle entity) {

		const auto et = Entities[entity.id];
		return et.block->GetComponentArray<Component>().Get(et.blockindex);

	}

	std::vector<EntityStorage> Entities;
	std::deque<size_t> deletedEntities;


	std::vector<ArchetypeBlockStorage> BlockStorage;
	std::vector<size_t> BlockHashes;
	std::vector<size_t> BlockANDHashes;
	bool bIsIterating{ false };
	bool bWantsDeletes{ false };

	//EnTT style  wrapper for porting
	EntityHandle create() {
		Archetype empty;
		return CreateEntity(empty);
	}
	void destroy(EntityHandle entity)
	{
		DeleteEntity(entity);
	}

	template<typename Component>
	Component & assign(EntityHandle entity, Component & args) {
		AddComponent<Component>(entity);
		Component & comp = GetComponent<Component>(entity);
		comp = args;
		return GetComponent<Component>(entity);
	}
	template<typename Component>
	Component & assign(EntityHandle entity) {
		AddComponent<Component>(entity);
		Component & comp = GetComponent<Component>(entity);
		return GetComponent<Component>(entity);
	}

	template<typename Component>
	bool has(EntityHandle entity) {
		return HasComponent<Component>(entity);
	}

	template<typename Component>
	void remove(EntityHandle entity)
	{
		RemoveComponent<Component>(entity);
	}
	template<typename Component>
	Component & get(EntityHandle entity) {
		return GetComponent<Component>(entity);
	}
	template<typename Component>
	Component & accomodate(EntityHandle entity, Component & args) {
		return assign(entity, args);
	}

};