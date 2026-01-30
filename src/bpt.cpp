#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "../include/storage/bpt.hpp"
#include "../include/utils/fixed_string.hpp"

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	sjtu::BPlusTree<sjtu::FixedString<64>, int> bpt;
	int q = 0;
	if (!(std::cin >> q)) {
		return 0;
	}
	while (q--) {
		std::string op;
		std::string key;
		int val = 0;
		std::cin >> op;
		if (op == "insert") {
			std::cin >> key >> val;
			bpt.insert(sjtu::FixedString<64>(key), val);
		}
		else if (op == "find") {
			std::cin >> key;
			std::vector<int> vec;
			bpt.find_all(sjtu::FixedString<64>(key), vec);
			if (vec.empty()) {
				std::cout << "null\n";
			}
			else {
				for (int v : vec) {
					std::cout << v << ' ';
				}
				std::cout << '\n';
			}
		}
		else if (op == "delete") {
			std::cin >> key >> val;
			bpt.erase(sjtu::FixedString<64>(key), val);
		}
		else if (op == "clear") {
			bpt.clear();
		}
		else {
			std::cout << "unknown operation" << std::endl;
		}
	}
	return 0;
}