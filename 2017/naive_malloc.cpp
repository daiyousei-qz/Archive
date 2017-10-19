#include <memory>
#include <cstdint>
#include <random>

struct FreeNode
{
	int dummy;
	int size; // from *this to end of free space
	FreeNode* prev;
	FreeNode* next;

	/* bytes */
};

struct AllocatedNode
{
	int size; // from *this to end of free space

	/* bytes */
};


constexpr auto kHeapSize = 4 * 1024 * 1024;

void* heap_begin;
void* heap_end;

FreeNode free_list_head;

void* AdvancePointer(void* ptr, int bytes)
{
	return reinterpret_cast<uint8_t*>(ptr) + bytes;
}

FreeNode* MakeFreeNode(void* ptr, int size, FreeNode* prev, FreeNode* next)
{
	auto result = reinterpret_cast<FreeNode*>(ptr);
	result->dummy = 0;
	result->size = size;
	result->prev = prev;
	result->next = next;

	return result;
}

AllocatedNode* MakeAllocatedNode(void* ptr, int size)
{
	auto result = reinterpret_cast<AllocatedNode*>(ptr);
	result->size = size;

	return result;
}

void Initialize()
{
	heap_begin = malloc(kHeapSize);
	heap_end = AdvancePointer(heap_begin, kHeapSize);

	free_list_head.dummy = 0;
	free_list_head.size = 0;
	free_list_head.prev = nullptr;
	free_list_head.next = MakeFreeNode(heap_begin, kHeapSize, &free_list_head, nullptr);
}

AllocatedNode* Alloc(int sz)
{
	// filter invalid size
	if (sz <= 0 || sz > kHeapSize / 2) 
		return nullptr;

	// append 4 for node size meta info
	sz += 4;

	// at least 16 is required for one allocation
	if (sz < 16) sz = 16;

	// align size to 4
	while (sz & 0b11) sz += 1;

	// try to find a chunk of memory to fit
	auto chunk = free_list_head.next;
	while (chunk)
	{
		auto size_delta = chunk->size - sz;
		if (size_delta < 0)
		{
			// not enough space, check next chunk
			chunk = chunk->next;
		}
		else if (size_delta < 16)
		{
			// additional space won't make another chunk
			// so give away the entire chunk
			
			sz = chunk->size;
			if (chunk->prev) chunk->prev->next = chunk->next;
			if (chunk->next) chunk->next->prev = chunk->prev;

			return MakeAllocatedNode(chunk, sz);
		}
		else
		{
			// shrink chunk for allocation
			auto new_chunk = MakeFreeNode(AdvancePointer(chunk, sz), chunk->size - sz, chunk->prev, chunk->next);
			if (chunk->prev) chunk->prev->next = new_chunk;
			if (chunk->next) chunk->next->prev = new_chunk;

			return MakeAllocatedNode(chunk, sz);
		}
	}

	// allocation failed
	return nullptr;
}

bool TryMergeChunk(FreeNode* chunk)
{
	auto chunk_view = reinterpret_cast<FreeNode*>(AdvancePointer(chunk, chunk->size));
	if (chunk_view == heap_end || chunk_view->dummy != 0)
	{
		// - given chunk is the last in the heap
		// - next chunk is allocated
		// no merging in both case
		return false;
	}
	else
	{
		// next node is free, remove it from free list and merge it into this chunk
		if (chunk_view->prev) chunk_view->prev->next = chunk_view->next;
		if (chunk_view->next) chunk_view->next->prev = chunk_view->prev;

		chunk->size += chunk_view->size;
		return true;
	}
}

void Free(AllocatedNode* node)
{
	auto freed_chunk = MakeFreeNode(node, node->size, nullptr, nullptr);
	while (TryMergeChunk(freed_chunk)) { }

	if (free_list_head.next)
	{
		free_list_head.next->prev = freed_chunk;
		freed_chunk->next = free_list_head.next;
	}
	
	freed_chunk->prev = &free_list_head;
	free_list_head.next = freed_chunk;
}

int main()
{
	using namespace std;

	Initialize();

	random_device rd;
	mt19937 mt{ rd() };
	bernoulli_distribution choice_dis;
	uniform_int_distribution<int> alloc_dis{ 4, 1000 };

	vector<AllocatedNode*> nodes;
	for (int i = 0; i < 1000; ++i)
	{
		if (nodes.size() < static_cast<int>(sqrt(i)) || choice_dis(mt))
		{
			auto sz = alloc_dis(mt);
			auto p = Alloc(sz);

			if (!p)
			{
				printf("failed to allocate a block of size %d bytes\n", sz);
				break;
			}
			else
			{
				printf("requested %d bytes(%d bytes allocated)\n", sz, p->size);
			}

			nodes.push_back(p);
		}
		else
		{
			shuffle(nodes.begin(), nodes.end(), mt);

			if (nodes.empty()) continue;

			printf("freed %d bytes\n", nodes.back()->size);
			Free(nodes.back());
			nodes.pop_back();
		}
	}

	for (auto node : nodes)
	{
		Free(node);
	}


	printf("\n\n");
	for (auto p = free_list_head.next; p; p = p->next)
	{
		printf("free node: %d bytes\n", p->size);
	}
	system("pause");
}