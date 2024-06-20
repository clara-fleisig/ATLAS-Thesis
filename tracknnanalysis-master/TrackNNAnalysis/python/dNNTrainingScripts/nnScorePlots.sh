source /afs/cern.ch/user/a/agekow/public/myvenv/bin/activate
cd /eos/user/a/agekow/tracking/data/trainingPkl/
path="/eos/user/a/agekow/tracking/TrainedNN/dNN/"
files=(*)
## to run each Region's network over its own data
for entry in *
do
    model="${path}${entry}/DNN_64.h5"
    #echo "$model"
    python /home/alex/Research/trackml/nnScripts/nnScorePlots_fix.py --data "$entry" --model "$model"
done