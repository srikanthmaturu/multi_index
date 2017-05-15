#!/bin/sh
#SBATCH --time=24:00:00
#SBATCH --mem-per-cpu=32000
#SBATCH --job-name=kmers-full-data-multi-index-test
#SBATCH --error=/work/deogun/smaturu/job.%J.err
#SBATCH --error=/work/deogun/smaturu/job.%J.out


alias echo="echo -e"

fasta_query_file="../data/test2.fa"
main_kmers_file="../data/KMERS_31_ECOLI.txt"
query_kmers_file=${fasta_query_file}_query_kmers_file


echo "Fasta query file $fasta_query_file ."
echo "Main kmers file $main_kmers_file ."
echo "\n"

if [ -f "${query_kmers_file}" ]
then
	echo "$query_kmers_file exists.\n"	
else 
	echo "$query_kmers_file not exists. creating it..."
	./fastaparser 31 $fasta_query_file $query_kmers_file
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

./mi_bs_index_10 ${main_kmers_file}.hash  ${query_kmers_file}.hash 1
