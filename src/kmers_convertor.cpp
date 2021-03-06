/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "32kmerto64bithash.cpp"
#include <cstdint>
int main(int argc, char** argv){
	if(argc < 4){
		cout << " Input both input and output files: " << endl;
		cout << "Usage:" << argv[0] << "conversion_type file1 file2" << endl;
		cout << "conversion_type = 0: 	file1 is input_sequence_file, file2 is  output_hash_file " <<endl;
		cout << "converstion_type = 1: file1 is input_hash_file, file2 is output_sequence_file " << endl; 
                cout << "kmer_size (only for conversion type = 1)" << endl;
        cout << "Number of kmer blocks: n" << endl;
		exit(1);
	}
    uint64_t kmer_size = stoull(argv[4]);
    uint64_t numberOfBlocks = stoull(argv[5]);

    cout << "Conversion started...." << endl;
	if(stoi(argv[1]) ==  0){	
		sequenceConvertor(argv[2], argv[3], kmer_size, numberOfBlocks); }
	else {

		hashConvertor(argv[2], argv[3], kmer_size, numberOfBlocks);
	}
}
