/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <iterator>
#include <omp.h>

using namespace std;

vector<string>& getKMERS(string sequence, int kmerSize){
        int length = sequence.size(), steps = length - kmerSize;
        string subString;
        vector<string> * kmers = new vector<string>(steps);
        #pragma omp parallel for
        for(int count=0; count< steps; count++){
                (*kmers)[count]= sequence.substr(count, kmerSize);
                //cout << (*kmers)[count] << endl;
        }
        return *kmers;
}

void generateKMERS(string inputFileName, string outputFileName,  int kmerSize){
        ifstream dataFile(inputFileName.c_str(), ifstream::in);
        ofstream outputFile(outputFileName.c_str(), ofstream::out);

        int batch_size = 1000000;
        vector<string> lines, kmers;

        cout << "Generating KMERS..." << endl;

        regex e ("^>");
        smatch m;

        lines.reserve(batch_size);

        while(!dataFile.eof()){
                string line;
                getline(dataFile, line);

                if(!regex_search(line, e)){
                        if(line.size() >= kmerSize){
                                lines.push_back(line);
                        }
                }

                if(lines.size() == batch_size || dataFile.fail() || dataFile.eof                                                                                                             ()){
                        for(int i=0; i<lines.size(); i++){
                                vector<string> temp = getKMERS(lines[i], kmerSiz                                                                                                             e);
                                kmers.insert(kmers.end(), temp.begin(), temp.end                                                                                                             ());
                        }
                        lines.clear();
                }
        }
        //cout << "Sorting started..." << endl;
        //sort(kmers.begin(), kmers.end());
        //cout << "Removing duplicates..." << endl;
        //vector<string>::iterator it = unique(kmers.begin(), kmers.end());
        //kmers.resize(distance(kmers.begin(), it));

        cout << "Writing KMERS "<< kmers.size() <<" to output file..." << endl;
        ostream_iterator<string> outIt (outputFile, "\n");
        copy(kmers.begin(), kmers.end(), outIt);
        dataFile.close();
        outputFile.close();
}


int main(int argc, char** argv){
        if(argc < 4){
        cout << "Arguments missing: " << endl;
        cout << "Usage ./fasta-parser [kmerSize] [inputfile] [outputfile] " << endl;                                                                                                           ndl;
        exit(1);
        }
        cout << "Input File: " << argv[2]  << " Output File:" << argv[3]  << " k                                                                                                             merSize: " << stoi(argv[1]) << endl;
        generateKMERS(argv[2], argv[3], stoi(argv[1]));

}