################################################################################
# Package: TrackNNEvaluator
################################################################################

# Declare the package name:
atlas_subdir( TrackNNEvaluator )


# External dependencies:
find_package( ROOT COMPONENTS MathCore RIO Matrix Physics RooFit RooFitCore Core Tree TreePlayer Hist pthread MathMore Minuit Minuit2 HistPainter Rint Graf Graf3d Gpad Html Postscript Gui GX11TTF GX11 )
find_package( lwtnn )
find_package( onnxruntime )
find_package( Eigen )


# Component(s) in the package:
atlas_add_library( TrackNNEvaluator
				   TrackNNEvaluator/*.h TrackNNEvaluator/atlasstyle/*.h Root/*.cxx Root/atlasstyle/*.C
  				   PUBLIC_HEADERS TrackNNEvaluator 
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} ${LWTNN_LIBRARIES} ${EIGEN_LIBRARIES} TrackNNCommon TrackNNEDM)

