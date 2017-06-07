#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <fstream>
#include <iterator>
#include <math.h>
#include <omp.h>
#include "sdsl/bits.hpp"

using namespace std;

uint64_t generate_mask(uint64_t kmerSize, uint64_t numberOfBlocks) {
    uint64_t kmerSpace = kmerSize*2, emptySpace = 64 - kmerSpace;
    uint64_t averageKMERBlockSize = kmerSpace/numberOfBlocks;
    if(averageKMERBlockSize%2 != 0) averageKMERBlockSize -= 1;
    uint64_t remainderKMERBlock = kmerSpace - averageKMERBlockSize*numberOfBlocks;
    uint64_t averageEmptyBlockSize = emptySpace/(numberOfBlocks+1);
    if(averageEmptyBlockSize%2 != 0) averageEmptyBlockSize -= 1;
    uint64_t remainderEmptyBlock = emptySpace - averageEmptyBlockSize*(numberOfBlocks + 1);
    /**
    cout << " kmerSpace: " << kmerSpace << "\n averageKMERBlockSize: " << averageKMERBlockSize <<  endl;
    cout << " remainderKMERBlock: " << remainderKMERBlock << "\n averageEmptyBlockSize: " << averageEmptyBlockSize << endl;
    cout << " remainderEmptyBlock: " << remainderEmptyBlock << endl;
    **/
    uint64_t mask = 0ULL;
    for(uint8_t i=0; i<numberOfBlocks+1; i++){
        uint64_t currentEmptyBlockSize = averageEmptyBlockSize, kmerSpaceUsed = i*averageKMERBlockSize;
        if(i==numberOfBlocks){
            currentEmptyBlockSize += remainderEmptyBlock;
            kmerSpaceUsed += remainderKMERBlock;
        }
        uint64_t pos = (i*averageEmptyBlockSize + kmerSpaceUsed), bits = pow(2, currentEmptyBlockSize) - 1;
        mask |= (bits << pos);
    }
    mask = ~mask;
    cout <<"Used Mask: " <<  mask << endl;
    return mask;
}

///////////////////////////////////
//Bits Deposit and Extract
///////////////////////////////////

//Parallel Bits Deposit
//x    HGFEDCBA
//mask 01100100
//res  0CB00A00
//x86_64 BMI2: PDEP
template <typename Integral>
constexpr Integral deposit_bits(Integral x, Integral mask) {
    Integral res = 0;
    for(Integral bb = 1; mask != 0; bb += bb) {
        if(x & bb) {
            res |= mask & (-mask);
        }
        mask &= (mask - 1);
    }
    return res;
}

//Parallel Bits Extract
//x    HGFEDCBA
//mask 01100100
//res  00000GFC
//x86_64 BMI2: PEXT
template <typename Integral>
constexpr Integral extract_bits(Integral x, Integral mask) {
    Integral res = 0;
    for(Integral bb = 1; mask != 0; bb += bb) {
        if(x & mask & -mask) {
            res |= bb;
        }
        mask &= (mask - 1);
    }
    return res;
}

uint64_t expandRight(uint64_t hash, uint64_t mask){
    /*
    uint64_t result=0ULL;
    asm ("pdep %0,%1,%2"
    : "=r" (result)
    : "r" (hash), "r" (mask)
    );
    return _pdep_u64(hash, mask);
    return result;*/
    return deposit_bits(hash, mask);
}

uint64_t compressRight(uint64_t hash, uint64_t mask){
    /*
    uint64_t result=0ULL;
    asm ("pext %0,%1,%2"
    : "=r" (result)
    : "r" (hash), "r" (mask)
    );
    return _pext_u64(hash, mask);
    return result;*/
    return extract_bits(hash, mask);
}

uint64_t computeHash(const char* sequence, int length, uint64_t mask){
	uint64_t hash = 0ULL, temp = 0ULL;
	for(int i=0; i<length; i++){
		int pos = length - i - 1;
		switch(sequence[i]){
		case 'a':
		case  'A':
			temp = 0ULL << (pos)*2;
			break;
		case 't':
		case 'T':
			temp = 1ULL << (pos)*2;
			break;
		case 'g':
		case 'G':
			temp = 2ULL << (pos)*2;
			break;
		case 'c':
		case 'C':
			temp = 3ULL << (pos)*2;
			break;
		default:
			temp = 0ULL;
		}
		hash = hash | temp;
	}
	return expandRight(hash, mask);
}

char* reverseHash(uint64_t hash, int kmerSize, uint64_t mask){
	char* sequence = new char[kmerSize+2];
	uint64_t temp;
    hash = compressRight(hash, mask);
	for(int i = 0; i < kmerSize; i++){
		temp = ((3ULL << i * 2) & hash) >> i * 2 ;
		switch(temp){
			case 0:
				sequence[kmerSize-i-1] = 'A';
				break;
			case 1:
				sequence[kmerSize-i-1] = 'T';
				break;
			case 2:
				sequence[kmerSize-i-1] = 'G';
				break;
			case 3: 
				sequence[kmerSize-i-1] = 'C';
				break;
			default:
				sequence[kmerSize-i-1] = ' ';
		}
	}	
	sequence[kmerSize] = '\n';
        sequence[kmerSize+1] = '\0';
	return sequence;

}


void sequenceConvertor(char* sequencefile, char* hashfile, uint64_t kmerSize, uint64_t numberOfBlocks){
	ifstream inputfile(sequencefile, ifstream::in);
	ofstream outputfile (hashfile, ofstream::out);
	
	vector<string> lines;
	vector<uint64_t> hashes;
	uint64_t batch_size = 1000000;
    uint64_t mask = generate_mask(kmerSize, numberOfBlocks);
	hashes.reserve(batch_size);
	char * input_char_sequence = new char[1024];
	while(!inputfile.eof()){
		inputfile.getline(input_char_sequence, 1024, '\n');
		lines.push_back(input_char_sequence);
		if(lines.size() == batch_size || inputfile.eof()){
			if(inputfile.eof()){
				lines.pop_back();
			}
			#pragma omp parallel for
			for(uint64_t i=0; i<lines.size(); i++){
				hashes[i] = computeHash(lines[i].c_str(), lines[i].size(), mask);
				//cout << hashes[i] << endl;
			}
		for(uint64_t i = 0; i < lines.size(); i++){
			outputfile.write((char*)&hashes[i], 8);
		}
		lines.clear();			
		}
	}	
}

uint64_t hd_spec(uint64_t a, uint64_t b){
        uint64_t t = (a ^ b);
        t |= t<<1;
        t &= 0xAAAAAAAAAAAAAAAAULL;
        return sdsl::bits::cnt(t);
}

void hashConvertor(char* hashfile, char* sequencefile, int kmerSize, int numberOfBlocks){
	ifstream inputfile(hashfile,ifstream::in | ifstream::binary);
	ofstream outputfile(sequencefile, ofstream::out);

	vector<uint64_t> hashes;
	vector<string> lines;
	uint64_t batch_size = 1000000;
	lines.reserve(batch_size);
	uint64_t mask = generate_mask(kmerSize, numberOfBlocks);
	while(!inputfile.eof()){
		uint64_t hash;
		inputfile.read((char*)&hash, 8);
		if(!inputfile.fail()){
			hashes.push_back(hash);
		}
		if(hashes.size() == batch_size || inputfile.eof()){
			#pragma omp parallel for
			for(uint64_t i=0; i<hashes.size(); i++){
				lines[i] = reverseHash(hashes[i], kmerSize, mask);
				//cout << lines[i]; 
			}
		for(uint64_t i = 0; i < hashes.size(); i++){
			outputfile.write(lines[i].c_str(), lines[i].size());
		}
		hashes.clear();		
		}	
	}
}


