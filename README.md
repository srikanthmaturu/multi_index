Multi-Index
=========

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

## Run the build.sh script. This will compile all executables required for the
## experiments.

cd build
cmake ..
cmake ..
sh build.sh

--------------------
Run Experiments: |
--------------------
## Edit the sample.slurm file. Modify the fasta_query_file and main_kmers_file
## shell variables to specify your input fasta file and reference_kmers_file
## Modify --mem-per-cpu=32000 variable in the slurm file to match require memory
## for the experiments.
## This will submit a slurm job. Once successful it will create an results file 
## in the same directory as query_fasta_file exists.

sbatch sample.slurm



Reference
---------

    @inproceedings{SIGIR16b,
        author = {Simon Gog and Rossano Venturini},
        title = {Fast and compact {H}amming distance index},
        booktitle = {{SIGIR} 2016: {P}roceedings of the 38th {A}nnual {I}nternational {ACM SIGIR} {C}onference on {R}esearch and {D}evelopment in {I}nformation {R}etrieval},
        year = {2016},
        pages = {--}
    }
