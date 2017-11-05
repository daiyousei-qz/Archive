#include <vector>
#include <cassert>
#include <optional>
#include <random>
#include <algorithm>
#include <fstream>
#include <memory>

struct Coord
{
	int x;
	int y;
};

class ResidentType
{
public:
	ResidentType(float weight, const std::string& symbol, const std::string& name)
		: weight_(weight), symbol_(symbol), name_(name) { }

	float		Weight() const { return weight_; }
	const auto& Symbol() const { return symbol_; }
	const auto& Name()	 const { return name_; }

private:
	float		weight_;
	std::string symbol_;
	std::string name_;
};

class Board
{
public:
	int Width() { return width_; }
	int Height() { return height_; }

	int Population() { return population_; }
	float Threshold() { return threshold_; }

	float DisatisficationIndex() { return disatisfication_index_; }
	float AverageSimilarityIndex() { return averge_similarity_index_; }

	void Initialize(int width, int height, int population, float threshold, const std::vector<ResidentType>& residents)
	{
		assert(!residents.empty());
		assert(width > 0 && height > 0 && population > 0);
		assert(population < width * height);
		assert(threshold > 0 && threshold < 1);

		// initialize parameters
		//

		residents_ = residents;

		width_ = width;
		height_ = height;
		population_ = population;
		threshold_ = threshold;

		// initialize board
		//

		RefreshBoard();

		// calculate indices and next board
		//

		Iterate();
	}

	// discards current board and calculate the next
	void Iterate()
	{
		// duplicate the board
		//

		cur_board_ = next_board_;

		// find empty blocks and candidates to move
		//

		std::vector<Coord> unstable_blocks;

		auto moving_counter = 0.f;
		auto similarity_ind_sum = 0.f;

		for (int j = 0; j < Height(); ++j)
		{
			for (int i = 0; i < Width(); ++i)
			{
				auto pos = Coord{ i, j };
				auto similarity = CalcSimilarityIndex(pos);

				if (similarity)
				{
					// residented
					similarity_ind_sum += *similarity;

					if (*similarity < threshold_)
					{
						// willing to move
						moving_counter += 1;
						unstable_blocks.push_back(pos);
					}
				}
				else
				{
					// not residented
					unstable_blocks.push_back(pos);
				}
			}
		}

		disatisfication_index_ = moving_counter / population_;
		averge_similarity_index_ = similarity_ind_sum / population_;

		// perform moving
		//

		// generate some randome permutation
		std::vector<int> perm;
		std::generate_n(
			std::back_inserter(perm), unstable_blocks.size(),
			[i = 0]() mutable { return i++; }
		);

		std::shuffle(perm.begin(), perm.end(), rng_);

		// ordinals in perm denotes index of moving blocks
		for (int i = 0; i < perm.size(); ++i)
		{
			auto dest_pos = unstable_blocks[i];
			auto src_pos = unstable_blocks[perm[i]];

			next_board_[CoordToOffset(dest_pos)] = cur_board_[CoordToOffset(src_pos)];
		}
	}

	void PrintBoard(const char* placeholder = "  ")
	{
		for (int j = 0; j < Height(); ++j)
		{
			for (int i = 0; i < Width(); ++i)
			{
				auto res = cur_board_[CoordToOffset({ i, j })];
				printf(res ? res->Symbol().c_str() : placeholder);
			}

			putchar('\n');
		}
	}
private:
	void RefreshBoard()
	{
		// resize board
		//

		next_board_.resize(ActualWidth() * ActualHeight());
		std::fill(next_board_.begin(), next_board_.end(), nullptr);

		cur_board_.resize(ActualWidth() * ActualHeight());
		std::fill(cur_board_.begin(), cur_board_.end(), nullptr);

		// initialize next board in random
		//

		std::vector<bool> occupied(width_*height_, false);
		std::fill_n(occupied.begin(), population_, true);
		std::shuffle(occupied.begin(), occupied.end(), rng_);

		auto res_weight = std::vector<float>{};
		for (const auto& res : residents_)
		{
			res_weight.push_back(res.Weight());
		}

		std::discrete_distribution<int> dis{ res_weight.begin(), res_weight.end() };
		for (int j = 0; j < Height(); ++j)
		{
			for (int i = 0; i < Width(); ++i)
			{
				if (occupied[j*width_ + i])
				{
					next_board_[CoordToOffset({ i, j })] = &residents_[dis(rng_)];
				}
			}
		}
	}

	std::optional<float> CalcSimilarityIndex(Coord pos)
	{
		static constexpr Coord kNeighborCoordDelta[] = {
			{ -1, -1 },{ 0, -1 },{ 1, -1 },{ 1, 0 },
			{ 1, 1 },{ 0, 1 },{ -1, 1 },{ -1, 0 }
		};

		auto res = next_board_[CoordToOffset(pos)];

		// no resident living at the position
		if (!res) return std::nullopt;

		int same = 0, diff = 0;
		for (auto delta : kNeighborCoordDelta)
		{
			auto neighbor_pos = Coord{ pos.x + delta.x, pos.y + delta.y };
			auto neighbor_res = next_board_[CoordToOffset(neighbor_pos)];

			if (neighbor_res)
			{
				if (neighbor_res == res)
				{
					same += 1;
				}
				else
				{
					diff += 1;
				}
			}
		}

		// no neighbor
		if (same + diff == 0)
		{
			return 1.f;
		}
		else
		{
			return static_cast<float>(same) / (same + diff);
		}
	}

	int ActualWidth() { return width_ + 2; }
	int ActualHeight() { return height_ + 2; }

	int CoordToOffset(Coord pos)
	{
		auto actual_pos = Coord{ pos.x + 1, pos.y + 1 };

		return actual_pos.y * ActualWidth() + actual_pos.x;
	}

	// TODO: fix insufficient entropy
	std::mt19937 rng_{ std::random_device{}() };

	std::vector<ResidentType> residents_;

	int width_ = 0;
	int height_ = 0;
	int population_ = 0;
	float threshold_ = 0.f;

	float disatisfication_index_;
	float averge_similarity_index_;

	std::vector<ResidentType*> next_board_;
	std::vector<ResidentType*> cur_board_;
};

int main()
{
	using namespace std;
	
	std::vector<ResidentType> res_vec = {
		{ .1f, "¨€", "rich" },
		{ .1f, "||", "midclass" },
		{ .1f, "--", "poor" },
	};

	Board board;
	board.Initialize(60, 60, 2800, 0.6, res_vec);

	for (const auto& res : res_vec)
		printf("%s DENOTES %s with weight of %f\n", res.Symbol().c_str(), res.Name().c_str(), res.Weight());

	for (int i = 0; i < 100; ++i)
		board.Iterate();

	board.PrintBoard();
	system("pause");
}