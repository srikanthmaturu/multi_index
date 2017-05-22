#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <fstream>
#include <iterator>
#include <omp.h>
#include "sdsl/bits.hpp"

using namespace std;

uint64_t computeHash(const char* sequence, int length){
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
		otherwise:
			temp = 0ULL;
		}
		hash = hash | temp;
	}
	return hash;
}

char* reverseHash(uint64_t hash, int kmerSize){
	char* sequence = new char[kmerSize+2];
	uint64_t temp;

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
			otherwise:
				sequence[kmerSize-i-1] = ' ';
		}
	}	
	sequence[kmerSize] = '\n';
        sequence[kmerSize+1] = '\0';
	return sequence;

}


void sequenceConvertor(char* sequencefile, char* hashfile){
	ifstream inputfile(sequencefile, ifstream::in);
	ofstream outputfile (hashfile, ofstream::out);
	
	vector<string> lines;
	vector<uint64_t> hashes;
	uint64_t batch_size = 1000000;

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
			for(uint8_t i=0; i<lines.size(); i++){
				hashes[i] = computeHash(lines[i].c_str(), lines[i].size());
				//cout << hashes[i] << endl;
			}
		for(uint8_t i = 0; i < lines.size(); i++){
			outputfile.write((char*)&hashes[i], 8);
		}
		//ostream_iterator<uint64_t> output_iterator(outputfile, "");
		//copy(hashes.begin(), hashes.begin() + lines.size(), output_iterator);
		lines.clear();			
		}
	}	
}

uint64_t computeHammingDistance(uint64_t a, uint64_t b, uint64_t kmer_size){
	uint64_t bs = (0xFFFFFFFFFFFFFFFF) >> (64 - 2*kmer_size);
        uint64_t t = (~(a ^ b)) & (bs);
	t &= (t << 1ULL);
	t &= 0xAAAAAAAAAAAAAAAAULL;
	return kmer_size - sdsl::bits::cnt(t);
}

void hashConvertor(char* hashfile, char* sequencefile, int kmerSize){
	ifstream inputfile(hashfile,ifstream::in | ifstream::binary);
	ofstream outputfile(sequencefile, ofstream::out);

	vector<uint64_t> hashes;
	vector<string> lines;
	int batch_size = 1000000;
	lines.reserve(batch_size);
	
	while(!inputfile.eof()){
		uint64_t hash;
		inputfile.read((char*)&hash, 8);
		if(!inputfile.fail()){
			hashes.push_back(hash);
		}
		if(hashes.size() == batch_size || inputfile.eof()){
			#pragma omp parallel for
			for(int i=0; i<hashes.size(); i++){
				lines[i] = reverseHash(hashes[i], kmerSize);
				//cout << lines[i]; 
			}
		for(int i = 0; i < hashes.size(); i++){
			outputfile.write(lines[i].c_str(), lines[i].size());
		}
		hashes.clear();		
		}	
	}
}




