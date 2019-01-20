#include "pch.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <algorithm>

#include "KVStore.hpp"

typedef KVStore <char, int> my_kv_t;
int main(int argc, const char * * argv, const char * * envp) {
	if (1 >= argc) {
		return 0;
	}
	my_kv_t kv;
	int arg_i;
	int count;
	size_t argv_i_length;
	for (arg_i = 1; arg_i < argc; ++arg_i) {
		count = 1;
		argv_i_length = strlen(argv[arg_i]);
		if (0 == kv.retrieve(argv[arg_i], argv_i_length, count)) {
			++count;
			kv.store_or_update(argv[arg_i], argv_i_length, count);
		} else {
			kv.store_or_update(argv[arg_i], argv_i_length, count);
		}
	}
	kv.dump();
}
