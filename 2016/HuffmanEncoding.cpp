#include <cassert>
#include <queue>
#include <map>
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <functional>

constexpr int HuffmanValuePlaceholder = -1;

struct HuffmanNode
{
	size_t weight;
	int value;
	HuffmanNode *parent; // for ease of retriving encoding
	HuffmanNode *left, *right;

	HuffmanNode()
		: weight(0)
		, value(HuffmanValuePlaceholder)
		, parent(nullptr)
		, left(nullptr)
		, right(nullptr) { }
};

struct EncodingEntry
{
	int value;
	std::string code;
};

class HuffmanEncoder
{
public:
	HuffmanEncoder(const std::vector<EncodingEntry> &encoding_data)
	{
		assert(!encoding_data.empty());
		size_t max_code_len = 0;

		for (const auto &entry : encoding_data)
		{
			max_code_len = std::max(max_code_len, entry.code.size());

			encoding_map_[entry.value] = entry.code;
			decoding_map_[entry.code] = entry.value;
		}

		max_code_len_ = max_code_len;
	}

public:
	void Print() const
	{
		std::cout << "===============Encoding Table===============\n";
		for (const auto &encoding_item : encoding_map_)
		{
			const auto &ch = static_cast<char>(encoding_item.first);
			const auto &code = encoding_item.second;
			std::cout << '\'' << ch << '\'';
			std::cout << ":";
			std::cout << '\"' << code << '\"';
			std::cout << std::endl;
		}
		std::cout << "============================================\n";
	}

	std::string Encode(const std::string &raw) const
	{
		std::string result;

		for (char ch : raw)
		{
			auto encoding_iter = encoding_map_.find(ch);
			if (encoding_iter != encoding_map_.end())
			{
				const auto &code = encoding_iter->second;
				std::copy(code.begin(), code.end(), std::back_inserter(result));
			}
			else
			{
				throw std::runtime_error("character not in ahlphabet");
			}
		}

		return result;
	}

	std::string Decode(const std::string &dat) const
	{
		std::string buffer;
		std::string result;
		for (char ch : dat)
		{
			buffer.push_back(ch);
			auto decoding_iter = decoding_map_.find(buffer);
			if (decoding_iter != decoding_map_.end())
			{
				result.push_back(decoding_iter->second);
				buffer.clear();
			}

			if (buffer.size() > max_code_len_)
			{
				throw std::runtime_error("unknown code sequence in data given");
			}
		}

		return result;
	}

private:
	size_t max_code_len_;
	std::map<int, std::string> encoding_map_;
	std::map<std::string, int> decoding_map_;
};

class HuffmanTree
{
public:
	// disable copying and moving
	HuffmanTree(const HuffmanTree&) = delete;
	HuffmanTree &operator=(const HuffmanTree&) = delete;
	HuffmanTree(HuffmanTree&&) = delete;
	HuffmanTree &operator=(HuffmanTree&&) = delete;

	HuffmanTree(const std::string &s)
	{
		size_t weight_table[128] = {};
		for (char ch : s)
		{
			weight_table[ch] += 1;
		}

		auto huffman_node_greater = [](HuffmanNode *lhs, HuffmanNode *rhs)
		{
			assert(lhs != nullptr && rhs != nullptr);
			return lhs->weight > rhs->weight;
		};
		std::vector<HuffmanNode*> valued_nodes;
		std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, decltype(huffman_node_greater)>
			construction_queue(huffman_node_greater);
		for (int ch = 0; ch < 128; ++ch)
		{
			size_t weight = weight_table[ch];
			if (weight != 0)
			{
				HuffmanNode *construct = new HuffmanNode;
				construct->weight = weight;
				construct->value = ch;

				valued_nodes.push_back(construct);
				construction_queue.push(construct);
			}
		}

		while (construction_queue.size() > 1)
		{
			HuffmanNode *left = construction_queue.top(); construction_queue.pop();
			HuffmanNode *right = construction_queue.top(); construction_queue.pop();

			// construct parent node
			HuffmanNode *construct = new HuffmanNode;
			construct->weight = left->weight + right->weight;
			construct->left = left;
			construct->right = right;

			// set it parent of left and right
			left->parent = right->parent = construct;

			// push new construct back to the heap
			construction_queue.push(construct);
		}

		root_ = construction_queue.top(); construction_queue.pop();
		valued_nodes_ = std::move(valued_nodes);
	}
	~HuffmanTree()
	{
		DestroyInternal(root_);
	}

public:

	// print huffman tree in formated s-expression
	void Print()
	{
		std::cout << "================Huffman Tree================\n";
		PrintInternal(root_, 0);
		std::cout << "============================================\n";
	}

	HuffmanEncoder CreateEncoder() const
	{
		assert(root_ != nullptr);

		std::vector<EncodingEntry> encodings;
		for (HuffmanNode *p : valued_nodes_)
		{
			assert(p->value != HuffmanValuePlaceholder);

			HuffmanNode *cur = p;
			std::string reversed_code;
			while (cur != root_)
			{
				HuffmanNode *parent = cur->parent;
				reversed_code.push_back(cur == parent->left ? '0' : '1');

				cur = parent;
			}

			EncodingEntry new_entry;
			new_entry.value = p->value;
			std::copy(reversed_code.rbegin(), reversed_code.rend(), std::back_inserter(new_entry.code));

			encodings.emplace_back(std::move(new_entry));
		}

		return HuffmanEncoder(encodings);
	}
private:
	void PrintIdentation(size_t depth)
	{
		for (int i = 0; i <depth; ++i)
		{
			std::cout << "  ";
		}
	}

	void PrintInternal(HuffmanNode *p, size_t depth)
	{
		assert(p != nullptr);
		bool nodeAtBottom = p->value != HuffmanValuePlaceholder;

		PrintIdentation(depth);
		std::cout << "(NODE ";
		std::cout << p->weight;

		if (nodeAtBottom)
		{
			std::cout << ' ';

			// deal with some non-printable characters
			switch (p->value)
			{
				case ' ':
					std::cout << "<whitespace>";
					break;
				default:
					std::cout << static_cast<char>(p->value);
					break;
			}
			std::cout << ')' << std::endl;
		}
		else
		{
			std::cout << std::endl;
			PrintInternal(p->left, depth + 1);
			PrintInternal(p->right, depth + 1);

			PrintIdentation(depth);
			std::cout << ')' << std::endl;
		}
	}

	void DestroyInternal(HuffmanNode *subtree)
	{
		if (subtree == nullptr) return;

		DestroyInternal(subtree->left);
		DestroyInternal(subtree->right);
		delete subtree;
	}

private:
	HuffmanNode *root_;
	std::vector<HuffmanNode*> valued_nodes_;
};

void RunEncodingTest(const HuffmanEncoder &encoder, const std::string &raw)
{
	std::string encoded = encoder.Encode(raw);
	std::string decoded = encoder.Decode(encoded);
	size_t encodedSizeInBytes = (encoded.size() + 7) / 8;
	size_t decodedSizeInBytes = decoded.size();

	std::cout << "================Huffman Tree================\n";
	std::cout << "Encoded Bitstream: " << encoded << std::endl;
	std::cout << "Encoded Size: " << encodedSizeInBytes << " bytes" << std::endl;
	std::cout << "Decoded Text: " << decoded << std::endl;
	std::cout << "Decoded Size: " << decodedSizeInBytes << " bytes" << std::endl;
	std::cout << "Compression Ratio: " << decodedSizeInBytes * 1.0 / encodedSizeInBytes << std::endl;
	std::cout << "============================================\n";
}

int main()
{
	std::string s;
	std::getline(std::cin, s);
	std::cout << "TEST TEXT:" << '\"' << s << '\"' << std::endl;

	// construct huffman tree from given text
	HuffmanTree tree(s);
	tree.Print();
	std::cout << std::endl;

	// generate encoder
	HuffmanEncoder encoder = tree.CreateEncoder();
	encoder.Print();
	std::cout << std::endl;

	// run encoding test
	RunEncodingTest(encoder, s);

	system("pause");
	return 0;
}