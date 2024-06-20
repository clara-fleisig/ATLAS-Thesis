dNNTraining.py:

	Flags to specify:
		epochs = number of epochs to train for
		batch = batch size to trian with
		learning_rate = learning rate to train with
		nodes = number of nodes in the first layer. The second layer is half of this, but the model can be easily modified in the createModel() function
		
		split: if split flag is used, the data will be split 50/50 True/Fake
		combined: if combined flag is used, the specified data files will all be combined
			This requires the --dataFiles flag to be used. Pass it a bash array of file names.
		
		if combined is not used, you must specify a signle data file to train with. Convenient to do this in a bash loop to loop over the 3 regions for a specified dataset

		*All Data Files (--data and --dataFiles) must be given as the name of the pickle file, NOT the full path 

dNNTraining.sh:
	This does not use the combined flag and will loop over the 3 regions of a specified dataset
	can use or not use --split
	
dNNTraining_combined.sh:
	this uses the combined flag and will combine the 3 regions of a specified dataset:
	can use or not use --split

nnScorePlots.py
	Mostly the same flags as dNNTraining. Flags specify which dataset to evaluate on
	--model = model to evaluate with. Pass the name of the directory where the model is located as dataSet/model_name.
		ex. Region_0p7_0p9_HTTFake_HTTTrueMu_SingleP_9L_Stubs_v6.pkl/DNN_64_split.h5
		
nnPlots.sh
	This will make a ROC curve and NN-Score plot using each regions trained network, and the data from that region

nnPlots_combined.sh
	This will make a ROC curve and NN-Score plot using the combined regions trained network, and the combined data from all 3 regions
	
nnPlots_combined_singleRegion.sh
	This will make a ROC curve and NN-Score plot using the combined regions trained network for each of the 3 eta regions independently


