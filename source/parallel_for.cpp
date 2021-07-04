#include "parallel_for.h"


void serialFor(const std::vector<int>& collection, std::function<void(int)> callback)
{
	std::for_each(std::execution::seq, collection.begin(), collection.end(), callback);
};


void parallelFor(const std::vector<int>& collection, std::function<void(int)> callback)
{
	std::for_each(std::execution::par_unseq, collection.begin(), collection.end(), callback);
};

int splitParallelForGetBatchCount(const std::vector<int>& collection, int chunkSize)
{
	int chunkCount = std::max(1, (int)(collection.size() / chunkSize));
	chunkCount += (chunkSize * chunkCount < collection.size()) ? 1 : 0;
	return chunkCount;
}

void splitParallelFor(const std::vector<int>& collection, int chunkSize, std::function<void(Range<std::vector<int>::const_iterator>& range, int batchIndex)> callback)
{
	assert(chunkSize > 0);
	assert(collection.size() > 0);

	int chunkCount = splitParallelForGetBatchCount(collection, chunkSize);

	std::vector<int> chunks; chunks.resize(chunkCount);
	std::iota(chunks.begin(), chunks.end(), 0);

	std::for_each(std::execution::par_unseq, chunks.begin(), chunks.end(), [&](int i)
		{
			callback(
				makeConstRange<int>(
					collection.cbegin() + i * chunkSize,
					collection.cbegin() + (std::min((int)(collection.cend() - collection.cbegin()), (int)(i * chunkSize + chunkSize)))
					),
				i
			);
		});
};