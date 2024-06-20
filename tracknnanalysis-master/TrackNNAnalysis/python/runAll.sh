#!/bin/bash
#dataPath=/home/sabidi/ATLAS/TrackNNAnalysis/run/PickleOutput/
dataPath=/usatlas/u/sabidi/usatlasdata/Tracking/TrackNNAnalysis/run/PickleOutput/

### For NN training
#for file in ${dataPath}ACTS*root
for file in ${dataPath}*root
do
   output=${file/${dataPath}/}
   output=${output/.root/}
   # output=${output/.root/}
   echo ${file}
   echo ${output}
   python3 utils/makeDatasetFromNTuple.py --trackType  HTTFakeTracks,HTTTrueTracks,TrueTracks  -i ${file} -o  ${output} -s 1000000 
   # python utils/makeDatasetFromNTuple.py --trackType TrueTruthTracks  -i ${file} -o  ${output} -s 1000000
   rm -rf tmp/
done
