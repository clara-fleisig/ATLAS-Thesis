source /afs/cern.ch/user/a/agekow/public/myvenv/bin/activate
cd /eos/user/a/agekow/tracking/data/trainingPkl/
files=(*)
#echo "${files[@]}"
python dNNTraining.py --epochs 2 --combined --dataFiles "${files[@]}" 