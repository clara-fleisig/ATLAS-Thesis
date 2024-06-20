################################################################################
# Package: TrackNNAnalysis
################################################################################

# Declare the package name:
atlas_subdir( TrackNNAnalysis )


# External dependencies:
find_package( ROOT COMPONENTS MathCore RIO Matrix Physics RooFit RooFitCore Core Tree TreePlayer Hist pthread MathMore Minuit Minuit2 HistPainter Rint Graf Graf3d Gpad Html Postscript Gui GX11TTF GX11 )
find_package( Eigen )


# Component(s) in the package:
atlas_add_library( TrackNNAnalysis
				   TrackNNAnalysis/*.h TrackNNAnalysis/atlasstyle/*.h Root/*.cxx Root/atlasstyle/*.C
  				   PUBLIC_HEADERS TrackNNAnalysis 
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${EIGEN_LIBRARIES} TrackNNCommon TrackNNEDM TrackNNDataReader TrackNNEvaluator PathResolver)


file(GLOB exe_files "utils/*.cxx")
foreach(_exe_file ${exe_files})
    get_filename_component(_theExec ${_exe_file} NAME_WE)
    atlas_add_executable(
        ${_theExec} ${_exe_file}
        INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
        LINK_LIBRARIES ${ROOT_LIBRARIES} ${EIGEN_LIBRARIES} TrackNNAnalysis
        )
endforeach()


file(GLOB exe_files "utils/testscripts/*.cxx")
foreach(_exe_file ${exe_files})
    get_filename_component(_theExec ${_exe_file} NAME_WE)
    atlas_add_executable(
        test_${_theExec} ${_exe_file}
        INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
        LINK_LIBRARIES ${ROOT_LIBRARIES} ${EIGEN_LIBRARIES}  TrackNNAnalysis
        )
endforeach()



# Install files from the package:
#atlas_install_scripts( python/*.py )
atlas_install_data( data/* )

