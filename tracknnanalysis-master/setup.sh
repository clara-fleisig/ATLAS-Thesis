if [ "$(type -t setupATLAS)" = 'alias' ]; then
    setupATLAS
else # alias not set on condor
    source /cvmfs/sft.cern.ch/lcg/views/LCG_95a/x86_64-slc6-gcc62-opt/setup.sh
fi
cd ../build
asetup --restore
source x86_64-slc7-gcc8-opt/setup.sh
cd ../run
