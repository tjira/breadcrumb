#!/bin/bash

I=$(shuf -i 0-1000000000 -n 1); CORES=1; random() {
    RANDOM=$4; awk -v min=$1 -v max=$2 -v seed=$RANDOM -v return_int=$3 'BEGIN {srand(seed); print (return_int == 1) ? int(min + rand() * (max - min + 1)) : min + rand() * (max - min)}'
}

while getopts "c:s:h" ARG; do
    case $ARG in
        c) CORES=$OPTARG;;
        s)     I=$OPTARG;;
        h) echo "USAGE: $0 [-c CORES] [-s SEED] [-h]"; exit 0;;
        *) echo "INVALID OPTION: $ARG"; exit 1;;
    esac
done && I0=$I

# ======================================================================================================================================================================================================
# SETTINGS
# ======================================================================================================================================================================================================

generate() { RANDOM_FRACTAL=1; RANDOM_ALGORITHM=1; RANDOM_COLORING=0; RANDOM_TRAP=1; RANDOM_FILL=1; RANDOM_LINEAR=1; RANDOM_SOLID=1; RANDOM_PERIODIC=1;

# ======================================================================================================================================================================================================
# FRACTAL
# ======================================================================================================================================================================================================

FRACTALS=("buffalo" "burningship" "julia" "mandelbrot" "manowar" "phoenix"); FRACTAL_INDEX=3

[ $RANDOM_FRACTAL -eq 1 ] && FRACTAL_INDEX=$(random 0 5 1 $I) && I=$((I+=1))

# ======================================================================================================================================================================================================
# ALGORITHM
# ======================================================================================================================================================================================================

ALGORITHMS=("density" "escape" "trap"); ALGORITHM_INDEX=1

[ $FRACTAL_INDEX -eq 0 ] && [ $RANDOM_ALGORITHM -eq 1 ] && ALGORITHM_INDEX=$(random 1 2 1 $I) && I=$((I+=1))
[ $FRACTAL_INDEX -eq 1 ] && [ $RANDOM_ALGORITHM -eq 1 ] && ALGORITHM_INDEX=$(random 1 2 1 $I) && I=$((I+=1))
[ $FRACTAL_INDEX -eq 2 ] && [ $RANDOM_ALGORITHM -eq 1 ] && ALGORITHM_INDEX=$(random 1 2 1 $I) && I=$((I+=1))
[ $FRACTAL_INDEX -eq 3 ] && [ $RANDOM_ALGORITHM -eq 1 ] && ALGORITHM_INDEX=$(random 1 2 1 $I) && I=$((I+=1))
[ $FRACTAL_INDEX -eq 4 ] && [ $RANDOM_ALGORITHM -eq 1 ] && ALGORITHM_INDEX=$(random 1 2 1 $I) && I=$((I+=1))
[ $FRACTAL_INDEX -eq 5 ] && [ $RANDOM_ALGORITHM -eq 1 ] && ALGORITHM_INDEX=$(random 1 2 1 $I) && I=$((I+=1))

SAMPLES=10000000

TRAP_INDEX=2; FILL=0
[ $RANDOM_TRAP -eq 1 ] && TRAP_INDEX=$(random 0 4 1 $I) && I=$((I+=1))
[ $RANDOM_FILL -eq 1 ] &&       FILL=$(random 0 1 1 $I) && I=$((I+=1))

# ======================================================================================================================================================================================================
# COLORING
# ======================================================================================================================================================================================================

COLORINGS=("linear" "periodic" "solid"); COLORING_INDEX=1

[ $ALGORITHM_INDEX -eq 0 ] && [ $RANDOM_COLORING -eq 1 ] && COLORING_INDEX=$(random 0 2 1 $I) && I=$((I+=1))
[ $ALGORITHM_INDEX -eq 1 ] && [ $RANDOM_COLORING -eq 1 ] && COLORING_INDEX=$(random 0 2 1 $I) && I=$((I+=1))
[ $ALGORITHM_INDEX -eq 2 ] && [ $RANDOM_COLORING -eq 1 ] && COLORING_INDEX=$(random 0 2 1 $I) && I=$((I+=1))

LINEAR_1=0; LINEAR_2=0; LINEAR_3=0; LINEAR_4=255; LINEAR_5=255; LINEAR_6=255;
[ $RANDOM_LINEAR -eq 1 ] && LINEAR_1=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_LINEAR -eq 1 ] && LINEAR_2=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_LINEAR -eq 1 ] && LINEAR_3=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_LINEAR -eq 1 ] && LINEAR_4=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_LINEAR -eq 1 ] && LINEAR_5=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_LINEAR -eq 1 ] && LINEAR_6=$(random 0 255 1 $I) && I=$((I+=1))

SOLID_1=255; SOLID_2=255; SOLID_3=255
[ $RANDOM_SOLID -eq 1 ] && SOLID_1=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_SOLID -eq 1 ] && SOLID_2=$(random 0 255 1 $I) && I=$((I+=1))
[ $RANDOM_SOLID -eq 1 ] && SOLID_3=$(random 0 255 1 $I) && I=$((I+=1))

PERIODIC_1=31.93; PERIODIC_2=6.26; PERIODIC_3=30.38; PERIODIC_4=5.86; PERIODIC_5=11.08; PERIODIC_6=0.81
[ $RANDOM_PERIODIC -eq 1 ] && PERIODIC_1=$(random 0 50.0 0 $I) && I=$((I+=1))
[ $RANDOM_PERIODIC -eq 1 ] && PERIODIC_2=$(random 0 6.28 0 $I) && I=$((I+=1))
[ $RANDOM_PERIODIC -eq 1 ] && PERIODIC_3=$(random 0 50.0 0 $I) && I=$((I+=1))
[ $RANDOM_PERIODIC -eq 1 ] && PERIODIC_4=$(random 0 6.28 0 $I) && I=$((I+=1))
[ $RANDOM_PERIODIC -eq 1 ] && PERIODIC_5=$(random 0 50.0 0 $I) && I=$((I+=1))
[ $RANDOM_PERIODIC -eq 1 ] && PERIODIC_6=$(random 0 6.28 0 $I) && I=$((I+=1))

# ======================================================================================================================================================================================================
# POSITION
# ======================================================================================================================================================================================================

[ $FRACTAL_INDEX -eq 0 ] && CENTER_REAL=-0.45 && CENTER_IMAG=0.35 && ZOOM=0.85 && PARAMETER=0.00
[ $FRACTAL_INDEX -eq 1 ] && CENTER_REAL=-0.45 && CENTER_IMAG=0.60 && ZOOM=1.20 && PARAMETER=0.00
[ $FRACTAL_INDEX -eq 2 ] && CENTER_REAL=-0.00 && CENTER_IMAG=0.00 && ZOOM=1.20 && PARAMETER=1.57
[ $FRACTAL_INDEX -eq 3 ] && CENTER_REAL=-0.75 && CENTER_IMAG=0.00 && ZOOM=1.20 && PARAMETER=0.00
[ $FRACTAL_INDEX -eq 4 ] && CENTER_REAL=-1.30 && CENTER_IMAG=0.00 && ZOOM=1.20 && PARAMETER=0.00
[ $FRACTAL_INDEX -eq 5 ] && CENTER_REAL=-0.00 && CENTER_IMAG=0.00 && ZOOM=1.65 && PARAMETER=3.14

[ $FRACTAL_INDEX -eq 1 ] && [ $ALGORITHM_INDEX -eq 2 ] && [ $TRAP_INDEX -eq 3 ] && CENTER_REAL=-0.45 && CENTER_IMAG=0.30 && ZOOM=0.95 && PARAMETER=0.00 # BURNINGSHIP TRAP-3
[ $FRACTAL_INDEX -eq 1 ] && [ $ALGORITHM_INDEX -eq 2 ] && [ $TRAP_INDEX -eq 4 ] && CENTER_REAL=-0.45 && CENTER_IMAG=0.30 && ZOOM=0.95 && PARAMETER=0.00 # BURNINGSHIP TRAP-4

# ======================================================================================================================================================================================================
# SAMPLE
# ======================================================================================================================================================================================================

}; generate

# ======================================================================================================================================================================================================
# SPECIAL CASES
# ======================================================================================================================================================================================================

[ $ALGORITHM_INDEX -eq 0 ] && [ $COLORING_INDEX -eq 2 ] && SAMPLES=1000000
[ $ALGORITHM_INDEX -eq 2 ] && [ $COLORING_INDEX -eq 2 ] && FILL=1
[ $FRACTAL_INDEX   -eq 2 ] && [ $COLORING_INDEX -eq 2 ] && exit 1

# ======================================================================================================================================================================================================
# PARAMETERS
# ======================================================================================================================================================================================================

ALGORITHM_PARAMS=("-d 80 10 $SAMPLES 1" "-e 80 10 1" "-t 80 100 $TRAP_INDEX $FILL")
COLORING_PARAMS=("-l $LINEAR_1 $LINEAR_2 $LINEAR_3 $LINEAR_4 $LINEAR_5 $LINEAR_6" "-p $PERIODIC_1 $PERIODIC_2 $PERIODIC_3 $PERIODIC_4 $PERIODIC_5 $PERIODIC_6" "-s $SOLID_1 $SOLID_2 $SOLID_3")

# ======================================================================================================================================================================================================
# GENERATION
# ======================================================================================================================================================================================================

echo -n "RANDOM=$I0 " && set -x && ./bin/fractal -c $CENTER_REAL $CENTER_IMAG -f ${FRACTALS[$FRACTAL_INDEX]} $PARAMETER -z $ZOOM ${ALGORITHM_PARAMS[$ALGORITHM_INDEX]} ${COLORING_PARAMS[$COLORING_INDEX]} -n $CORES
