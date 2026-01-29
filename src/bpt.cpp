#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "../include/storage/bpt.hpp"

// Fixed-length string key to ensure POD storage on disk
struct FixedString {
	char data_[65];

	FixedString() { std::memset(data_, 0, sizeof(data_)); }

	explicit FixedString(const char* s) {
		std::memset(data_, 0, sizeof(data_));
		std::strncpy(data_, s, sizeof(data_) - 1);
	}

	explicit FixedString(const std::string& s) : FixedString(s.c_str()) {}
};

inline bool operator==(const FixedString& a, const FixedString& b) {
	return std::strcmp(a.data_, b.data_) == 0;
}

inline bool operator!=(const FixedString& a, const FixedString& b) {
	return !(a == b);
}

inline bool operator<(const FixedString& a, const FixedString& b) {
	return std::strcmp(a.data_, b.data_) < 0;
}

inline bool operator>(const FixedString& a, const FixedString& b) {
	return std::strcmp(a.data_, b.data_) > 0;
}

inline bool operator<=(const FixedString& a, const FixedString& b) {
	return std::strcmp(a.data_, b.data_) <= 0;
}

inline bool operator>=(const FixedString& a, const FixedString& b) {
	return std::strcmp(a.data_, b.data_) >= 0;
}

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	sjtu::BPlusTree<FixedString, int> bpt;
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
			bpt.insert(FixedString(key), val);
		}
		else if (op == "find") {
			std::cin >> key;
			std::vector<int> vec;
			bpt.find_all(FixedString(key), vec);
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
			bpt.erase(FixedString(key), val);
		}
	}
	return 0;
}