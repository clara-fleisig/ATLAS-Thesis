################################################################################
# Package: TrackNNDataReader
################################################################################

# Declare the package name:
atlas_subdir( TrackNNDataReader )

# External dependencies:
find_package( ROOT COMPONENTS MathCore RIO Matrix Physics RooFit RooFitCore Core Tree TreePlayer Hist pthread MathMore Minuit Minuit2 HistPainter Rint Graf Graf3d Gpad Html Postscript Gui GX11TTF GX11 )


# Component(s) in the package:
atlas_add_library( TrackNNDataReader
				   TrackNNDataReader/*.h  Root/*.cxx
  				   PUBLIC_HEADERS TrackNNDataReader 
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} TrackNNCommon TrackNNEDM NtupleAnalysisUtils)


