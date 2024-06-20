source /afs/cern.ch/user/a/agekow/public/myvenv/bin/activate
cd /eos/user/a/agekow/tracking/data/trainingPkl/ 
path="/eos/user/a/agekow/tracking/TrainedNN/dNN/"
files=(*)

# evaluate combined model on each eta slice:
for entry in *
do
    substring="$(echo $entry| cut -d'_' -f 4)"
    model="${path}/combined${entry}/DNN_64.h5"
    #echo "$model"
    python nnScorePlots.py --data "$entry" --model "$model" --split
#done