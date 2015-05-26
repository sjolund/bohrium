#!/bin/bash
dst="/home/chris/.bohrium/config.ini"
src_filterOn="/home/chris/.bohrium/config-filterOn.ini"
src_filterOff="/home/chris/.bohrium/config-filterOff.ini"

res_filterOn="results/filterOn.p"
res_filterOff="results/filterOff.p"


echo "collect time measures with preprocessor filter"
cp $src_filterOn $dst
python preprocessor_benchmark.py $res_filterOn
#valgrind dython preprocessor_benchmark.py $res_filterOn


echo "collect time measures without preprocessor filter"
cp $src_filterOff $dst
python preprocessor_benchmark.py $res_filterOff
#valgrind dython preprocessor_benchmark.py $res_filterOff


#echo "Assert correct results using filter"
# to be implemented

echo "combining results in plots"
python preprocessor_benchmark_make_plots.py $res_filterOn $res_filterOff