#!/bin/bash
#SBATCH -J "fgamma"
echo "@ FGAMMA START" `date`

echo "@ SLURM_JOB_ID" $SLURM_JOB_ID
echo "@ SLURM_JOB_NAME" $SLURM_JOB_NAME
echo "@ SLURM_JOB_NUM_NODES" $SLURM_JOB_NUM_NODES
echo "@ SLURM_JOB_NODELIST" $SLURM_JOB_NODELIST

seed=$(expr `date +%s` + $RANDOM)

set -x
time ./fgamma -v0 --seed=${seed} $@
{ set +x; } 2>/dev/null

echo "@ FGAMMA END" `date`
