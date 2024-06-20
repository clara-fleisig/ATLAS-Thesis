source /afs/cern.ch/user/a/agekow/public/myvenv/bin/activate
cd "/eos/user/a/agekow/tracking/data/trainingPkl/"
for entry in #Specify datasets to train over ex. Region_*"
do
    python dNNTraining.py --epochs 200 --data "$entry" --split
done 