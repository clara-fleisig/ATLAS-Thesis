## First Time  Setup
```
    setupATLAS
    mkdir TrackNNAnalysis
    cd TrackNNAnalysis
    mkdir build run
    git clone --recursive https://gitlab.cern.ch/sabidi/tracknnanalysis.git source
    cd build
    asetup AnalysisBase,24.2.30,here
    cmake ../source  && make -j 
    source ../build/x86_64-*-opt/setup.sh
```
### Every Day
```
    setupATLAS
    cd <your-path-to-the-build-directory> 
    asetup --restore
    source ../build/x86_64-*-opt/setup.sh
    cd ../run
```


## Instructions
The general step that are needed to run this code from start to end are:
    - 1 - Get track hit combination - This is something that either HT folks gave us, or something that was prepared running on RDO using a custom Algo - This is it the point which we will have to bring raw track hits combination to
    - 2 - Prepare flat root where each entry is a track candidate that is labelled as true or fake
    - 3 - Convert the above root files into pkl
    - 4 - Train NN using the above pkl
    - 5 - Convert the NN into an ONNX interface file
    - 6 - Evaluate performance 
    - 7 - Make the plots



## Step 1 - Hit Combination
Nothing exist at the moment in this framework for this

## Step 2 - Flat root version
The general philosophy for this process is that there are DataReader interface class that is designed to read the track and hit information (for which there are dedicated classes). This output then gets handed to an OutputManager, which makes this output root file.

There are helper scripts in submission provided for batch submission of these jobs


### Inputs from Athena RDO
This will process the hit input files are created in the RDO -> XXX analysis and these are available at /eos/user/a/agekow/tracking/data

There are few types of input files:
    - With pileup - These have a r12063 tag and are typically much larger files. These **can** be used to produce fake tracks to train against. And they typically have track at low  
    - Without pileup - These have an r12064 or "user.sabidi" tag (Latter are produced privately). These are clean low activity events with good track reconstruction. Depending on the underlying process, they have have flat, single pT or falling pT specturm. These **can not** be used to make fake tracks


**To run the code**:
```
processForPickle --inputFile <path/to/root/file> \
                 --detectorGeoFile /eos/user/a/agekow/tracking/data/DetectorGeo/v1/DetectorGeo.root \
                 --outputFile <path/to/output/file> \
                 --nEvents (-1 if you want all, otherwise some n > 0)
                 --fakeTypes (comma separated list of all the fakes you want generated. If blank, which is default, no fakes will be generated)
                 --LayerRegion (Keyword option that picks which layers will be used. See Root/DataReader.cxx for the options)
                 --HTTLayerHits <minimum number of hits a track must have>
                 --lwrEtaLim <to limit the eta region on the low side>
                 --uprEtaLim <to limit the eta region on the high side>

```

### Inputs from HTT Framework
Similarly as above, there is a dedicated script for splitting up HTT input
```
processForPickleHTT --inputFile <path/to/root/file> \
                    --detectorGeoFile /eos/user/a/agekow/tracking/data/DetectorGeo/v1/DetectorGeo.root \
                    --outputFile <path/to/output/file> \
                    --nEvents (-1 if you want all, otherwise some n > 0)
                    --doSubSample (true/false, does a pT dependant subsampling. This is to ensure that we have less events than what python pickling can handle at the moment. Need to fix this)

```

### Submitting to a cluster
There is a handy script that is provided to submit the pickling jobs to batch clusters or running a lot of jobs in parallel. To run this script, first you need to configure the joblist in source/submission/jobListPickle.py and source/submission/ListPickles (The first script imports small jobs sets from those found in the folder). There are many examples you can find to build off from. To submit these jobs you have to run
```
python ../source/TrackNNAnalysis/submission/submit.py --doPickle <look at the file to see supported running information>
``` 

## Step 3 - Convert root into pkl
We need pkl files that further post-process the root files into a format that is readable by the NN training. A script has been proved in python/utils/makeDatasetFromNTuple.py

It is designed to use the input from step 2. There are some keyword features in the py script to figure out how many hits the output should have, so please have a look.

 To run it:

```
# python utils/makeDatasetFromNTuple.py 
    --trackType <List of track types you want to train again eg. TrueTracks,TrueTruthTracks,HTTFakeTracks, Check py script for the latest supported tracks>  
    -i  <path/to/root/file>  
    -o <path/to/output/file>

```
## Step 4 - Train NN
Currently, there are many scripts to train the NN. Alex has his set and I have a modified version at https://gitlab.cern.ch/bciungu/H4lMachineLearning

There is a good README for the H4lMachineLearning already. Would suggest to take a look at the submission https://gitlab.cern.ch/bciungu/H4lMachineLearning/-/blob/master/submission/jobList_Trktraining.py


## Step 5 - Convert to ONNX
Keras output .h5 files that need to be converted to ONNX. A small helper script is provided at python/utils/convert.py

Also, if you are doing anything new in the input structure of the NN, support will need to be added in Root/NNEvalUtils.cxx for this


## Step 6 - Eval performance

At this point, we have to evaluate the performance of the NN training. For this, again helper scripts are provide that eval the NN and apply the cut and the HW algorithm. Dedicated scripts should be there for evaluating a different input structure. Right now, only the HTT one is up and running

```
evalEffHTT      --inputFile <path/to/file> \
                --detectorGeoFile /eos/user/a/agekow/tracking/data/DetectorGeo/v1/DetectorGeo.root \
                --outputFile <path/to/output/file> \
                --nEvents (-1 if you want all, otherwise some n > 0)
                --onnxFile <path/to/onnx/file> \
                --nHits ( number of hits used for the NN training)
                --NNCutVal ( value for the NN cut)
                --nHitOverlapThres ( value for the HW threshold)
                --lwrPtLim <to limit the pT of the tracks on low side>
                --uprPtLim <to limit the pT of the tracks on high side>
                --trackType <truth/reco - which track collection to evaluate the efficiency against>
                --dropOverlay <true/false - to drop all the extra overlay tracks that are added in the HTT code to increase true track statistics>
                --overlayOnly <true/false - to only use the extra overlay tracks that are added in the HTT code to increase true track statistics>
```
Again, a helper script in submission is provided for batch submission. Output from different files can be hadd together to increase statistics

### Submitting to a cluster
There is a handy script that is provided to submit the pickling jobs to batch clusters or running a lot of jobs in parallel. To run this script, first you need to configure the joblist in source/submission/jobListEval.py and source/submission/ListPEval (The first script imports small jobs sets from those found in the folder). There are many examples you can find to build off from. To submit these jobs you have to run
```
python ../source/TrackNNAnalysis/submission/submit.py --doEval <look at the file to see supported running information>
``` 


## Step 7 - Plotting performance

Many different helpers scripts have been made for this

### ROC curve
```
plotROC   --fileList <comma separated list of files>
          --outFolder <path/to/output/folder> 
          --nHitOverlapThres <comma separated of HW threshold you want plotted>
```

### All summary plot from one training
```
plotSummaryPlots    --inputFile <path/to/file>
                    --NNCutVal (NN cut val you want plotted) 
                    --nHitOverlapThres (HW threshold you want plotted)
```

### Compare different training/NN/HW 
```
plotComapre     --fileList <comma separated list of files>
                --fileLabel <comma separated list of the legend entry for each file>
                --nHitOverlapThres <comma separated of HW threshold you want plotted>
                --NNCutVal <comma separated of NN cut you want plotted>
```
