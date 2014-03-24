#!/bin/bash
#SBATCH -J "fgamma"
seed=$(expr `date +%s` + $RANDOM)
time ./fgamma --seed=${seed} $@
