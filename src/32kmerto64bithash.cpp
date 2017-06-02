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
    uint64_t remainderKMERBlock = kmerSpace%numberOfBlocks;
    uint64_t averageEmptyBlockSize = emptySpace/(numberOfBlocks+1);
    uint64_t remainderEmptyBlock = emptySpace%(numberOfBlocks+1);
    /**
    cout << " kmerSpace: " << kmerSpace << endl;
    cout << " averageKMERBlockSize: " << averageKMERBlockSize <<  endl;
    cout << " remainderKMERBlock: " << remainderKMERBlock << endl;
    cout << " averageEmptyBlockSize: " << averageEmptyBlockSize << endl;
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


uint64_t expandRight(uint64_t hash, uint64_t mask){

    uint64_t result=0ULL;
    asm ("pdep %0,%1,%2"
    : "=r" (result)
    : "r" (hash), "r" (mask)
    );
    //cout << " Re: " << result << " A:" << _pdep_u64(hash, mask) <<  endl;
    //return _pdep_u64(hash, mask);
    cout << " Re: " << result << endl;
    return result;
}

uint64_t compressRight(uint64_t hash, uint64_t mask){

    uint64_t result=0ULL;
    asm ("pext %0,%1,%2"
    : "=r" (result)
    : "r" (hash), "r" (mask)
    );
    cout << " Re: " << result << endl;
    //return _pext_u64(hash, mask);
    return result;
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
			//#pragma omp parallel for
			for(uint64_t i=0; i<lines.size(); i++){
				hashes[i] = computeHash(lines[i].c_str(), lines[i].size(), mask);
				//cout << hashes[i] << endl;
			}
		//cout << " here " << endl;
		for(uint64_t i = 0; i < lines.size(); i++){
			outputfile.write((char*)&hashes[i], 8);
		}
		//ostream_iterator<uint64_t> output_iterator(outputfile, "");
		//copy(hashes.begin(), hashes.begin() + lines.size(), output_iterator);
		lines.clear();			
		}
		//cout << " out " << endl;
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


