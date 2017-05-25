#!/bin/sh
#SBATCH --time=24:00:00
#SBATCH --ntasks-per-node=16
#SBATCH --nodes=1
#SBATCH --mem-per-cpu=32000
#SBATCH --sockets-per-node=1
#SBATCH --cores-per-socket=1
#SBATCH --threads-per-core=1
#SBATCH --job-name=kmers-full-data-multi-index-test
#SBATCH --error=/work/deogun/smaturu/thesis/multi_index_v2/multi_index/build/job.%J.err
#SBATCH --error=/work/deogun/smaturu/thesis/multi_index_v2/multi_index/build/job.%J.out


alias echo="echo -e"

fasta_query_file="../data/test2.fa"
main_kmers_file="../data/KMER_19"
query_kmers_file=${fasta_query_file}_query_kmers_file


echo "Fasta query file $fasta_query_file ."
echo "Main kmers file $main_kmers_file ."
echo "\n"

if [ -f "${query_kmers_file}" ]
then
	echo "$query_kmers_file exists.\n"	
else 
	echo "$query_kmers_file not exists. creating it..."
	./fastaparser 19 $fasta_query_file $query_kmers_file
	echo "$query_kmers_file created.\n"
fi

if [ -f "${main_kmers_file}.hash" ]
then
	echo "${main_kmers_file}.hash exists. \n"
else
	echo "${main_kmers_file}.hash not exists. creating it..."
	./kmers_convertor 0 $main_kmers_file ${main_kmers_file}.hash
	echo "${main_kmers_file}.hash created.\n"
fi

if [ -f "${query_kmers_file}.hash" ]
then
	echo "${query_kmers_file}.hash exists.\n"
else
	echo "${query_kmers_file}.hash not exists. creating it..."
	./kmers_convertor 0 $query_kmers_file ${query_kmers_file}.hash
	echo "${query_kmers_file}.hash created.\n"
fi

./mi_tc_bv_st_red_index_10 ${main_kmers_file}.hash  ${query_kmers_file}.hash 1 0 0 0 19

