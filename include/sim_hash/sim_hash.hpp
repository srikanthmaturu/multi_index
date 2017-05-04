#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "sim_hash/MurmurHash3.h"

/* Window_size is the number of tokens that hashed togheter */
std::vector<uint64_t> get_hashed_tokens(std::string &string, size_t window_size) {
  
  uint32_t seed1 = 0x2c759c01;
  uint32_t seed2 = 0xfef136d7;
  
  std::istringstream iss(string);
  std::vector<std::string> tokens;
  std::copy(std::istream_iterator<std::string>(iss),
       std::istream_iterator<std::string>(),
       std::back_inserter(tokens));
  
  std::vector<uint64_t> hashes;
  hashes.reserve(tokens.size());
  for(size_t i = 0; i < tokens.size(); ++i) {
    
    std::string supertoken(tokens[i]);
    for(size_t j = i+1; j < std::min(i+window_size, tokens.size()); ++j) 
      supertoken.append(" " + tokens[j]);
  
    uint32_t h1, h2;
    MurmurHash3_x86_32(supertoken.c_str(), 
                       supertoken.length(), 
                       seed1,
                       &h1);
    MurmurHash3_x86_32(supertoken.c_str(), 
                       supertoken.length(), 
                       seed2,
                       &h2);
    uint64_t h = ((uint64_t)h1 << 32) + h2;
    //std::cout << supertoken << " h:" << h << std::endl;
    hashes.push_back(h);
  }  
  return hashes;
}

/* Calculates the similarity hash with super-tokens of length window_size. */
uint64_t simhash64(std::string &string, size_t window_size) {
  
  std::vector<int> histogram(64, 0);

	std::vector<uint64_t> hashes = get_hashed_tokens(string, window_size);

  for(auto &hash : hashes) {
		int weight = 1; // token weight is 1 by default; it may change depending on the hashes frequencies

		for (size_t bit = 0; bit < 64; ++bit)
			histogram[bit] += (hash & (1ULL << bit)) == 0 ? -weight : weight;
	}

	/* Calculate a bit vector from the histogram */
	uint64_t hash = 0;
	for (size_t bit = 0; bit < 64; ++bit)
		hash |= ((uint64_t)(histogram[bit] >= 0)) << bit;
	
	return hash;
}

/* Calculates the oddsketch with super-tokens of length window_size. */
uint64_t oddsketch64(std::string &string, size_t window_size) {

	std::vector<uint64_t> hashes = get_hashed_tokens(string, window_size);
  uint64_t oddsketch = 0;
  for(auto hash : hashes) {
    size_t bit = hash%64;
    oddsketch = oddsketch ^ (1ULL << bit);
	}
  
	return oddsketch;
}