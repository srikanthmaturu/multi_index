SET(base_url "http://xor.di.unipi.it/~rossano")

# A samll default data set for testing
#SET(local_test_cases merged_illumina_31_onlykmers.txt)
## Uncomment the following line to use the datasets from the SIGIR 2016 paper
#SET(local_test_cases Clueweb09-Full.SimHash Clueweb09-Full.OddSketch lsh_sift_64.hash mlh_sift_64.hash)
#SET(test_cases ${local_test_cases} PARENT_SCOPE)

#FOREACH(test_case ${local_test_cases})
#    SET(abs_test_case ${CMAKE_HOME_DIRECTORY}/data/${test_case})
#   IF(NOT EXISTS ${abs_test_case})
#        SET(tc_url ${base_url}/${test_case})
#        MESSAGE(${tc_url})
#        FILE(DOWNLOAD ${tc_url} ${abs_test_case} SHOW_PROGRESS)
#    endif(NOT EXISTS ${abs_test_case})
#ENDFOREACH(test_case)

ADD_EXECUTABLE(kmers_convertor src/32kmerto64bithash.cpp)

SET(test_cases PARENT_SCOPE)
SET(local_kmer_files merged_illumina_31_onlykmers.txt)

FOREACH (kmer_file ${local_kmer_files})
    SET(abs_kmer_file ${CMAKE_HOME_DIRECTORY}/data/${kmer_file})
    IF(NOT EXISTS ${abs_kmer_file})
        ADD_CUSTOM_COMMAND(OUTPUT ${abs_kmer_file}.hash
                            COMMAND $<TARGET_FILE:kmers_convertor> 0 ${abs_kmer_file} ${abs_kmer_file}.hash
                            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                            COMMENT "Converting ${abs_test_case} to hash file."
                            VERBATIM)
    endif(NOT EXISTS ${abs_kmer_file})
    LIST(APPEND test_cases ${abs_kmer_file})
ENDFOREACH
