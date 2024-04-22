
#####################################################################################################
## Run one benchmark
#####################################################################################################

mkdir -p ../RESULTS/BASELINE.4C.8MB; ./sim ../TRACES/bwaves_06.mat.gz ../TRACES/bwaves_06.mat.gz ../TRACES/bwaves_06.mat.gz ../TRACES/bwaves_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/src_rrs_exp_1k_bwaves_06.res ; 
mkdir -p ../RESULTS/BASELINE.4C.8MB; ./sim ../TRACES/bzip2_06.mat.gz ../TRACES/bzip2_06.mat.gz ../TRACES/bzip2_06.mat.gz ../TRACES/bzip2_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/src_rrs_exp_1k_bzip2_06.res ; 
mkdir -p ../RESULTS/BASELINE.4C.8MB; ./sim ../TRACES/cactusADM_06.mat.gz ../TRACES/cactusADM_06.mat.gz ../TRACES/cactusADM_06.mat.gz ../TRACES/cactusADM_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/src_rrs_exp_1k_cactusADM_06.res ; 
mkdir -p ../RESULTS/BASELINE.4C.8MB; ./sim ../TRACES/GemsFDTD_06.mat.gz ../TRACES/GemsFDTD_06.mat.gz ../TRACES/GemsFDTD_06.mat.gz ../TRACES/GemsFDTD_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/src_rrs_exp_1k_GemsFDTD_06.res ; 
mkdir -p ../RESULTS/BASELINE.4C.8MB; ./sim ../TRACES/mcf_06.mat.gz ../TRACES/mcf_06.mat.gz ../TRACES/mcf_06.mat.gz ../TRACES/mcf_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/src_rrs_exp_1k_mcf_06.res ; 
mkdir -p ../RESULTS/BASELINE.4C.8MB; ./sim ../TRACES/zeusmp_06.mat.gz ../TRACES/zeusmp_06.mat.gz ../TRACES/zeusmp_06.mat.gz ../TRACES/zeusmp_06.mat.gz > ../RESULTS/BASELINE.4C.8MB/src_rrs_exp_1k_zeusmp_06.res ; 

#####################################################################################################
## Run all benchmarks
#####################################################################################################


suite="lab1"
fw=7
#fw    - number of simulations to run in parallel
#sutie - set of benchmarks to simulate (defined in bench_common.pl
#mkdir "../RESULTS/BASELINE.4C.8MB/src_half_double"
#perl ../SCRIPTS/runall.pl --w $suite --f $fw  --d "../RESULTS/BASELINE.4C.8MB/src_half_double" ;

