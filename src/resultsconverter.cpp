#include <iostream>
#include <cstdlib>
#include <fstream>
#include "32kmerto64bithash.cpp"
#include <omp.h>

void translateResultsFile(string input_results_file, string output_results_file, uint64_t kmer_size, uint64_t t_k){
    ifstream inputfile(input_results_file, ifstream::in);
    ofstream outputfile(output_results_file, ofstream::out | ofstream::binary));
    
    vecotr<uint64_t> * hashes;
    uint64_t query, result;
    vector< vector< pair<string,uint64_t> > > query_results_vector(qry.size());
    vector< vector<uint64_t>* > m_vector(); 
    vector< uint64_t > queries(qry.size());
    while(!inputfile.eof()){
        uint64_t t, marker = 0xFFFFFFFFFFFFFFFFULL;
        inputfile.read((char*)&t, 8);
        if(t & marker == 0){
            if(!hashes->isEmpty()) {
                m_vector.push_back(hashes);
            }
            hashes = new vector<uint64_t>();
            inputfile.read((char*)&query, 8);
            queries.push_back(query);
        }
        else {
            inputfile.read((char*)&result, 8);
            hashes.push_back(result);
        }
    }
    
    #pragma omp parallel for
    for (size_t i=0; i<m_vector.size(); ++i){
        auto result = m_vector[i];
        cout << " processing query " << i << endl;
	for (size_t j=0; j<result.size(); ++j){
            uint64_t hamming_distance = computeHammingDistance(queries[i], result[j], kmer_size);
            if(hamming_distance > t_k/2) continue;
            string original_query_result = reverseHash(result[j], kmer_size);
            original_query_result.pop_back();
	    query_results_vector[i].push_back(make_pair(original_query_result, hamming_distance));
        }
    }
    
    cout << "saving results in the results file. " << endl;
    for (size_t i=0; i<qry.size(); ++i){
        query_results << "\n\n"<< i<< ": \t" << queries[i].c_str() << endl;
        query_results << "\nApproximate Sequences:\n";
        for (size_t j=0; j<query_results_vector[i].size(); ++j){
            query_results << "\t\t" << query_results_vector[i][j].first.c_str() << "  " << query_results_vector[i][j].second << endl;
       }
    }
}                     
                        
int main(int argc, char** argv){
	if(argc < 5){
		cout << "Usage:" << argv[0] << "64_results_file output_file kmer_size t_K" << endl;
                cout << "64_results_file: results file" << endl;
                cout << "output_file: output results file" << endl;
                cout << "kmer_size: kmer size" << endl;
                cout << "t_k: k " << endl;
		exit(1);
	}
	cout << "Translating input results file...." << endl;
	translateResultsFile(argv[1], argv[2], stoull(argv[3]), stoull(argv[4]));
}