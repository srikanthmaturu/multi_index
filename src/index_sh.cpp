//
// Created by srikanth on 6/6/17.
//
#include "multi_idx/multi_idx.hpp"
#include "multi_idx/multi_idx_red.hpp"
#include "multi_idx/linear_scan.hpp"
#include "sim_hash/sim_hash_idx.hpp"
#include <sdsl/io.hpp>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <omp.h>

using namespace multi_index;
using namespace sdsl;
using namespace std;

using namespace std::chrono;
using timer = std::chrono::high_resolution_clock;

const string index_name = INDEX_NAME;
string filtered_index_name;

template<class duration_type=std::chrono::seconds>
struct my_timer{
    string phase;
    time_point<timer> start;

    my_timer() = delete;
    my_timer(string _phase) : phase(_phase) {
        std::cout << "Start phase ``" << phase << "''" << std::endl;
        start = timer::now();
    };
    ~my_timer(){
        auto stop = timer::now();
        std::cout << "End phase ``" << phase << "'' ";
        std::cout << " ("<< duration_cast<duration_type>(stop-start).count() << " ";
        std::cout << " ";
        std::cout << " elapsed)" << std::endl;
    }
};

template<uint8_t t_b, uint8_t t_k, typename t_idx>
struct idx_file_trait{
    static std::string value(std::string hash_file){
        return hash_file + "." + to_string(t_b)+"_"+to_string(t_k)+"."+filtered_index_name;
    }
};


template<uint8_t t_b, uint8_t t_k, typename t_strat>
struct idx_file_trait<t_b, t_k, multi_idx_red<t_strat, t_k>>{
static std::string value(std::string hash_file){
    return hash_file + "." + to_string(t_b)+"."+filtered_index_name;
}
};

template<uint8_t t_b, uint8_t t_k>
struct idx_file_trait<t_b, t_k, linear_scan<t_k>>{
static std::string value(std::string hash_file){
    return hash_file;
}
};

void load_sequences(string sequences_file, vector<string>& sequences){
    ifstream input_file(sequences_file, ifstream::in);

    for(string sequence; getline(input_file, sequence);){
        sequences.push_back(sequence);
    }
}

int main(int argc, char* argv[]){
    constexpr uint8_t t_b = BLOCKS;
    constexpr uint8_t t_k = K;
    typedef SH_INDEX_TYPE sh_index_type;
    filtered_index_name = index_name;
    std::string in_fix = "_simd";
    auto pos = index_name.find(in_fix);
    if ( pos != std::string::npos ) {
        filtered_index_name = index_name.substr(0, pos) +
                              index_name.substr(pos+in_fix.size());
    }

    if ( argc < 2 ) {
        cout << "Usage: ./" << argv[0] << " sequences_file [query_file]" << endl;
        return 1;
    }

    string sequences_file = argv[1];
    string queries_file = argv[2];
    string queries_results_file = queries_file + "_search_results.txt";
    string idx_file = idx_file_trait<t_b,t_k,INDEX_TYPE>::value(sequences_file) + "_WS" + to_string(WS);
    string map_file = sequences_file + "_sh_map_WS" + to_string(WS);
    sh_index_type pi;

    {
        ifstream idx_ifs(idx_file);
        ifstream map_ifs(map_file);
        if ( !idx_ifs.good() || !map_ifs.good()){
            auto index_construction_begin_time = timer::now();
            vector<string> sequences;
            load_sequences(sequences_file, sequences);
            {
                //            my_timer<> t("index construction");
//                auto temp = index_type(keys, async);
                cout<< "Index construction begins"<< endl;
                auto temp = sh_index_type(sequences);
                //            std::cout<<"temp.size()="<<temp.size()<<std::endl;
                pi = std::move(temp);
            }
            pi.store_to_files(idx_file, map_file);
            auto index_construction_end_time = timer::now();
            cout<< "Index construction completed." << endl;
            cout << "# total_time_to_construct_index_in_us :- " << duration_cast<chrono::microseconds>(index_construction_end_time-index_construction_begin_time).count() << endl;
        } else {
            cout << "Index already exists. Using the existing index." << endl;
        }

        pi.load_from_files(idx_file, map_file);

        vector<string> queries;
        load_sequences(queries_file, queries);
        ofstream results_file(queries_results_file);
	uint64_t i=0;
	for(string qry: queries){
	    results_file << ">" << qry << endl;
            vector<string> res = pi.match(qry);
            for(string r: res){
                results_file << r << endl;
            }
	    cout << "processed query " << i++ << endl;
        }
    }

}


