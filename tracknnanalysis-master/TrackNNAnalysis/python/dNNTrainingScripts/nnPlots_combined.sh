source /afs/cern.ch/user/a/agekow/public/myvenv/bin/activate
cd /eos/user/a/agekow/tracking/data/trainingPkl/
path="/eos/user/a/agekow/tracking/TrainedNN/dNN/"
files=(*)

# evaluate combined model on combined data:
model="${path}"#model name ex. combined8L_Nom.pkl/DNN_64_combined.h5"
python nnScorePlots.py --combined --dataFiles "${files[@]}"  --model "$model" --split