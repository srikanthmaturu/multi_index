Multi-Index
=========

Installation
------------


--------------------
Installation Steps: |
--------------------

## Load essential components.

module load compiler/gcc/5.4
module load python/2.7
module load R/3.3
module load cmake/3.5
module load git/2.7


## Clone the git repository. This will download all the source code from the github 
## and stores in the multi_index folder.

git clone https://github.com/srikanthmaturu/multi_index.git

cd multi_index

## This will install the sub-module sdsl library.
./install.sh

cd build

## Copy the merged_illumina_31_onlykmers.txt file in to the multi_index/data/ directory

cp SRC_DIR/merged_illumina_31_onlykmers.txt multi_index/data/

cmake ..



--------------------
Creating Index:      |
--------------------

## Now create a slurm file with the following contents and run it using the sbatch
## command while in the build directory

--------------------------------------------------------------------------------------

#!/bin/sh
#SBATCH --time=24:00:00
#SBATCH --mem-per-cpu=128000
#SBATCH --job-name=kmers-full-data-multi-index-test
#SBATCH --error=/work/deogun/smaturu/job.%J.err
#SBATCH --error=/work/deogun/smaturu/job.%J.out

make exp0

--------------------------------------------------------------------------------------

## submit the job using sbatch command.
sbatch test.slurm

## Once the slurm is successfull. Index will be created in the multi_index/data/ folder. 
## Along with the index, a query results file also can be found which contains
## the results of the sample query that was derived from the entire hash.
## This also takes care of the conversion of merged_illumina_31_onlykmers.txt into hash file.

--------------------
Running queries on  |
the Index:          |
--------------------

## Index is located at multi_index/data/merged_illumina_31_onlykmers.txt.hash.data.4_3.mi_bs
## To query an index run the following executable stored in the build directory.

cd build
./mi_bs_index_3 ../data/merged_illumina_31_onlykmers.txt.hash.data [query_file_name] 1

## This will create a query_results with a file name [query_file_name]_search_results.txt file 
## The results can be opened in the notepad.


--------------------
Generating a query  |
hash file:          |
--------------------
## Use following steps to generate a query file.

cd build
./kmers_convertor 0 [input_query_file] [output_hash_file]

## This will generate the output_hash_file file with 64 bit keys. This file can be direct input
## to the mi_bs_index_3







Reference
---------

    @inproceedings{SIGIR16b,
        author = {Simon Gog and Rossano Venturini},
        title = {Fast and compact {H}amming distance index},
        booktitle = {{SIGIR} 2016: {P}roceedings of the 38th {A}nnual {I}nternational {ACM SIGIR} {C}onference on {R}esearch and {D}evelopment in {I}nformation {R}etrieval},
        year = {2016},
        pages = {--}
    }
