#pragma once
#include <algorithm>
#include <assert.h>
#include <execution>
#include <functional>
#include <vector>


template<typename T>
std::vector<T> slice(std::vector<T>& v, int m, int n)
{
	const size_t size = n - m + 1;
	std::vector<T> vec(size);

	const int num_threads = std::min((int)size, (int)std::thread::hardware_concurrency());
	assert(num_threads > 0);
	const int chunkSize = std::max(1, (int)(size / num_threads));

	std::vector<std::pair<int, int>> chunks;
	for (int i = 0; i < num_threads; ++i)
	{
		chunks.emplace_back(i * chunkSize, i * chunkSize + std::min(chunkSize, (int)size));
	}

	std::for_each(std::execution::par, chunks.begin(), chunks.end(), [&](const std::pair<int, int>& pair)
		{
			std::copy(v.begin() + m + pair.first, v.begin() + m + pair.second, vec.begin() + pair.first);
		});

	return vec;
}

template<typename T>
void slice(const std::vector<T>& v, int m, int n, std::vector<T>& vec)
{
	const size_t size = n - m + 1;

	assert(vec.size() >= size);

	const int num_threads = std::min((int)size, (int)std::thread::hardware_concurrency());
	assert(num_threads > 0);
	const int chunkSize = (int)size / (int)num_threads;

	std::vector<std::pair<int, int>> chunks;
	for (int i = 0; i < num_threads; ++i)
	{
		chunks.emplace_back(i * chunkSize, i * chunkSize + std::min(chunkSize, (int)size));
	}

	std::for_each(std::execution::par, chunks.begin(), chunks.end(), [&](const std::pair<int, int>& pair)
		{
			std::copy(v.begin() + m + pair.first, v.begin() + m + pair.second, vec.begin() + pair.first);
		});
}

template<class It>
struct Range
{
	It b, e;
	It begin() const { return b; }
	It end() const { return e; }
	std::size_t size() const { return end() - begin(); }
	bool empty() const { return begin() == end(); }

	Range without_back(std::size_t n = 1) const {
		n = (std::min)(n, size());
		return { begin(), end() - n };
	}
	Range without_front(std::size_t n = 1) const {
		n = (std::min)(n, size());
		return { begin() + n, end() };
	}
	decltype(auto) front() const { return *begin(); }
	decltype(auto) back() const { return *(std::prev(end())); }
};
template<class C>
auto makeRange(C&& c) {
	using std::begin; using std::end;
	return Range{ begin(c), end(c) };
};
template<class Type>
auto makeRange(typename std::vector<Type>::iterator b, typename std::vector<Type>::iterator e)
{
	return Range<typename std::vector<Type>::iterator>{ b, e };
};

template<class Type>
auto makeConstRange(typename std::vector<Type>::const_iterator b, typename std::vector<Type>::const_iterator e)
{
	return Range<typename std::vector<Type>::const_iterator>{ b, e };
};

void serialFor(const std::vector<int>& collection, std::function<void(int)> callback);
void parallelFor(const std::vector<int>& collection, std::function<void(int)> callback);
int splitParallelForGetBatchCount(const std::vector<int>& collection, int chunkSize);
void splitParallelFor(const std::vector<int>& collection, int chunkSize, std::function<void(Range<std::vector<int>::const_iterator>& range, int batchIndex)> callback);