//
// Created by srikanth on 6/5/17.
//
#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>

#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "sim_hash/sim_hash.hpp"
#include "sdsl/io.hpp"
using namespace multi_index;
using namespace sdsl;

template<typename m_idx_type, uint8_t ws=5>
class sim_hash_idx {
public:
    sim_hash_idx() = default;
    sim_hash_idx(const sim_hash_idx &) = default;
    sim_hash_idx(sim_hash_idx &&) = default;
    sim_hash_idx &operator=(const sim_hash_idx &) = default;
    sim_hash_idx &operator=(sim_hash_idx &&) = default;

    sim_hash_idx(vector<string>& data){
        vector<uint64_t> keys;
        generate_sim_hashes(data, keys);
        construct_index(keys);
        constuct_map(data, keys, hash_sequence_map);
    }

    m_idx_type mi;
    unordered_map<uint64_t, vector<string>> hash_sequence_map;

    vector<string> match(string& qry){
        vector<string> res;
        for(auto rh:unique_vec(get<0>(mi.match(simhash64(qry, ws))))){
            auto r = hash_sequence_map[rh];
            res.insert(res.end(), r.begin(), r.end());
        }
        return res;
    }

    void generate_sim_hashes(vector<string>& sequences, vector<uint64_t>& hashes){
        for(string sequence:sequences){
            hashes.push_back(simhash64(sequence, ws));
        }
    }

    vector<uint64_t> unique_vec(vector<uint64_t> v){
        if(v.size()>0){
            std::sort(v.begin(), v.end());
            auto end = std::unique(v.begin(), v.end());
            v.resize(end-v.begin());
        }
        return v;
    }

    void constuct_map(vector<string>& sequences, vector<uint64_t>& hashes, unordered_map<uint64_t, vector<string>>& hash_sequence_map){
        for(uint64_t i=0; i<hashes.size(); i++){
            uint64_t hash = hashes[i];
            if(hash_sequence_map.count(hash) ==0){
                vector<string> seqs;
                seqs.push_back(sequences[i]);
                hash_sequence_map[hash] = seqs;
            }
            else {
                hash_sequence_map[hash].push_back(sequences[i]);
                auto it = std::unique(hash_sequence_map[hash].begin(), hash_sequence_map[hash].end());
                hash_sequence_map[hash].resize(std::distance(hash_sequence_map[hash].begin(), it));
            }
        }
        /*for(auto it= hash_sequence_map.begin(); it!=hash_sequence_map.end(); ++it){
            if(hash_sequence_map.count((*it).first) > 0){
                auto res = hash_sequence_map[(*it).first];
                cout << "hash: "<< (*it).first << " ";
                for(auto i: res){
                    cout << "  seq: " << i << " ";
                }
                cout << endl;
            }
        }*/
    }

    void construct_index(vector<uint64_t>& keys){
         mi = std::move(m_idx_type(keys));
    }

    void load_idx(std::string idx_file) {
        using namespace sdsl;
        load_from_file(mi, idx_file);
    }

    void serialize_idx(std::string idx_file){
        using namespace sdsl;
        store_to_file(mi, idx_file);
    }

    void serialize_map(std::string map_file){
        ofstream map_file_ofs(map_file);
        boost::archive::binary_oarchive oa(map_file_ofs);
        oa << hash_sequence_map;
    }

    void load_map(std::string map_file){
        ifstream map_file_ifs(map_file);
        boost::archive::binary_iarchive ia(map_file_ifs);
        ia >> hash_sequence_map;
        /*for(auto it= hash_sequence_map.begin(); it!=hash_sequence_map.end(); ++it){
            if(hash_sequence_map.count((*it).first) > 0){
                auto res = hash_sequence_map[(*it).first];
                cout << "hash: "<< (*it).first << " ";
                for(auto i: res){
                    cout << "  seq: " << i << " ";
                }
                cout << endl;
            }
        }*/
    }

    void load_from_files(std::string idx_file, std::string map_file){
        load_idx(idx_file);
        cout << "IDX loaded from file." << endl;
        load_map(map_file);
        cout << "Map file loaded from file." << endl;
    }

    void store_to_files(std::string idx_file, std::string map_file){
        serialize_idx(idx_file);
        serialize_map(map_file);
    }
};