// a demostration implementation of encoding and decoding for convolutional code
// PERFORMANCE IS NOT CONSIDERED WHEN CODE IS WRITTEN

#include <vector>
#include <cassert>
#include <array>
#include <random>

using namespace std;

template<int N>
int CalculateCodeDistance(const array<int, N>& lhs, const array<int, N>& rhs)
{
	int acc = 0;
	for (int i = 0; i < N; ++i)
	{
		acc += (lhs[i] == rhs[i] ? 0 : 1);
	}

	return acc;
}

int CalculateCodeDistance(const vector<int>& lhs, const vector<int>& rhs)
{
	int acc = 0;
	for (int i = 0; i < min(lhs.size(), rhs.size()); ++i)
	{
		acc += (lhs[i] == rhs[i] ? 0 : 1);
	}

	return acc;
}

// N: number of parallel impulse to go
// L: length of impulse response in bit
template<int N, int L>
class ConvImpulseResponse
{
	static_assert(N > 0);
	static_assert(L > 0 && L < 20);

public:
	// number of internal state
	static constexpr int S = 1 << (L - 1);

	ConvImpulseResponse(const array<array<int, L>, N>& response)
		: response_(response)
	{
	}
	ConvImpulseResponse(initializer_list<array<int, L>> init)
	{
		int i = 0;
		for (const auto& impulse : init)
		{
			response_[i++] = impulse;
		}
	}

	array<int, N> CalculateOutput(int state, int input)
	{
		VerifyData(state, input);

		array<int, N> result;
		int path_index = 0;

		for (const auto& impulse : response_)
		{
			int s = state;
			int tmp = input & impulse[0];
			for (int i = 1; i < impulse.size(); ++i)
			{
				tmp ^= s & impulse[i];
				s >>= 1;
			}

			result[path_index++] = tmp;
		}

		return result;
	}

	int CalculateNextState(int state, int input)
	{
		VerifyData(state, input);

		return ((state << 1) & (S - 1)) + input;
	}

private:
	void VerifyData(int state, int input)
	{
		assert(state >= 0 && state < S);
		assert(input == 0 || input == 1);
	}

	array<array<int, L>, N> response_;
};

template<int N, int L>
class Decoder
{
public:
	using ImpulseResponse = ConvImpulseResponse<N, L>;

	Decoder(const ImpulseResponse& response)
		: response_(response)
	{
		Initialize();
	}

	void Feed(const array<int, N>& group)
	{
		auto new_dist = dist_trace_;
		auto new_state = state_trace_;

		for (auto& d : new_dist)
		{
			d = numeric_limits<int>::max();
		}
		for (auto& vec : new_state)
		{
			vec.push_back(-1);
		}

		for (int i = 0; i < ImpulseResponse::S; ++i)
		{
			for (int raw_bit = 0; raw_bit <= 1; ++raw_bit)
			{
				auto output = response_.CalculateOutput(i, raw_bit);
				auto target = response_.CalculateNextState(i, raw_bit);

				auto path_dist = dist_trace_[i] + CalculateCodeDistance(group, output);
				if (path_dist < new_dist[target])
				{
					new_dist[target] = path_dist;

					new_state[target] = state_trace_[i];
					new_state[target].push_back(raw_bit);
				}
			}
		}

		dist_trace_ = new_dist;
		state_trace_ = new_state;
	}

	void FeedMany(const vector<int>& data)
	{
		assert(data.size() % N == 0);

		array<int, N> buf;
		for (int i = 0; i < data.size() / N; ++i)
		{
			for (int j = 0; j < N; ++j)
				buf[j] = data[i*N + j];

			Feed(buf);
		}
	}

	vector<int> Finalize()
	{
		auto min_iter = min_element(dist_trace_.begin(), dist_trace_.end());
		auto index = distance(dist_trace_.begin(), min_iter);
		auto result = state_trace_[index];

		Initialize();
		return result;
	}

private:
	void Initialize()
	{
		fill(state_trace_.begin(), state_trace_.end(), vector<int>{});

		// WORKAROUND, potential bugs may appear
		fill(dist_trace_.begin(), dist_trace_.end(), 9999);
		dist_trace_[0] = 0;
	}

	ImpulseResponse response_;

	array<int, ImpulseResponse::S> dist_trace_;
	array<vector<int>, ImpulseResponse::S> state_trace_;
};

template<int N, int L>
class Encoder
{
public:
	using ImpulseResponse = ConvImpulseResponse<N, L>;

	Encoder(const ImpulseResponse& response)
		: response_(response) { }

	void Feed(int bit)
	{
		// calculate response for each circuit
		auto output = response_.CalculateOutput(state_, bit);
		buffer_.insert(buffer_.end(), output.begin(), output.end());

		// update state register
		state_ = response_.CalculateNextState(state_, bit);
	}

	void FeedMany(const vector<int>& data)
	{
		for (auto bit : data)
			Feed(bit);
	}

	vector<int> Finalize()
	{
		for (int i = 1; i < L; ++i)
			Feed(0);

		return move(buffer_);
	}

private:
	int state_ = 0;
	ImpulseResponse response_;

	vector<int> buffer_ = {};
};

void PrintBitVector(const vector<int>& data, const char* promt = "")
{
	printf(promt);

	for (int bit : data)
	{
		printf(bit ? "1" : "0");
	}
	printf("\n");
}

static auto GlobalRandomSource = default_random_engine{ random_device{}() };

vector<int> GenerateBitStream(int len)
{
	auto distri = bernoulli_distribution{};
	auto result = vector<int>{};

	for (int i = 0; i < len; ++i)
	{
		result.push_back(distri(GlobalRandomSource) ? 1 : 0);
	}

	return result;
}

vector<int> SimulateBSC(const vector<int>& stream, double errorRate)
{
	// select bits to flip
	vector<int> places;
	for (int i = 0; i < stream.size(); ++i)
	{
		places.push_back(i);
	}

	shuffle(places.begin(), places.end(), GlobalRandomSource);

	// flip bits and return
	auto result = stream;
	for (int i = 0; i < stream.size() && i * 1.0 / stream.size() < errorRate; ++i)
	{
		auto j = places[i];
		result[j] = result[j] ? 0 : 1;
	}

	return result;
}

void Test(int len, double channelErrorRate)
{
	printf("===================================================\n");

	// generate response
	auto a0 = array<int, 4>{ 1, 0, 1, 1 };
	auto a1 = array<int, 4>{ 1, 1, 1, 1 };
	auto response = ConvImpulseResponse<2, 4>{ a0, a1 };

	printf("g0 = 1011\n");
	printf("g1 = 1111\n");

	// generate raw data
	auto rawStream = GenerateBitStream(len);
	PrintBitVector(rawStream, "raw:         ");

	// encode it
	auto encoder = Encoder(response);

	encoder.FeedMany(rawStream);
	auto encodedStream = encoder.Finalize();
	PrintBitVector(encodedStream, "encoded:     ");

	// transmit it through a BSC
	auto transmittedStream = SimulateBSC(encodedStream, channelErrorRate);
	PrintBitVector(transmittedStream, "transmitted: ");

	// decode it
	auto decoder = Decoder(response);

	decoder.FeedMany(transmittedStream);
	auto decodedStream = decoder.Finalize();
	PrintBitVector(decodedStream, "decoded:     ");

	// other information
	auto transmitDist = CalculateCodeDistance(encodedStream, transmittedStream);
	auto codeDist = CalculateCodeDistance(rawStream, decodedStream);

	printf("\n");
	printf("designed channel error rate: %f\n", channelErrorRate);
	printf("actual channel error rate: %f\n", transmitDist * 1.0 / encodedStream.size());
	printf("code error rate: %f\n", codeDist * 1.0 / rawStream.size());
}

int main() 
{
	Test(20, 0.05);
	Test(20, 0.1);
	Test(20, 0.15);
	Test(20, 0.2);

	system("pause");
}