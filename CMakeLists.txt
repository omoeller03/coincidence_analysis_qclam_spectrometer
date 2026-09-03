cmake_minimum_required(VERSION 3.10)
project(QClamGo4Analysis)


# modus QCLAM_SINGLE (kein Dagobert, DEFERA etc)
option(QCLAM_SINGLE "Build QCLAM_SINGLE" OFF)
# modus QCLAM_SINGLE (kein Dagobert, DEFERA etc)
option(QCLAM_DAGOBERT "Build QCLAM_DAGOBERT" ON)
# modus QCLAM_SINGLE_ROOT (nur QCLAM, mit Root-Input)
option(QCLAM_SINGLE_ROOT "Build QCLAM_SINGLE_ROOT" OFF)

if((QCLAM_SINGLE AND QCLAM_DAGOBERT) OR (QCLAM_SINGLE AND QCLAM_SINGLE_ROOT) OR (QCLAM_SINGLE_ROOT AND QCLAM_DAGOBERT))
	message(FATAL_ERROR "Nur einen Modus aktivieren: QCLAM_SINGLE || QCLAM_DAGOBERT || QCLAM_SINGLE_ROOT")
endif()

if(QCLAM_SINGLE)
	add_definitions(-DQCLAM_SINGLE)
endif()

if(QCLAM_SINGLE_ROOT)
	add_definitions(-DQCLAM_SINGLE_ROOT)
endif()

if(QCLAM_DAGOBERT)
	add_definitions(-DQCLAM_DAGOBERT)
endif()



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(yaml-cpp_DIR $ENV{YAML_CPP_DIR})

find_package(ROOT REQUIRED)
find_package(Go4 REQUIRED)
find_package(yaml-cpp REQUIRED)

include(${ROOT_USE_FILE})
set(CMAKE_PREFIX_PATH "$ENV{GO4SYS}" ${CMAKE_PREFIX_PATH})
link_directories("$ENV{GO4SYS}/lib")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/modules
    ${CMAKE_CURRENT_BINARY_DIR}
    $ENV{GO4SYS}/include
)

###----------- HEADER && SOURCES

#file(GLOB_RECURSE QCLAM_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
#file(GLOB_RECURSE QCLAM_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cxx)
#file(GLOB_RECURSE QCLAM_MODULES ${PROJECT_SOURCE_DIR}/modules/*.cpp)

set(QCLAM_HEADERS "")
set(QCLAM_SOURCES "")




# --- dateien, die unabhaengig vom modus eingebunden werden ---
file(GLOB GEOMETRY_HEADERS include/geometry/*.h)
file(GLOB PARAM_HEADERS include/parameter/*.h)
file(GLOB FIT_HEADERS include/fitmodels/*.h)
file(GLOB UTILS_HEADERS include/utils/*.h)
file(GLOB MODULE_HEADERS modules/*.h)

file(GLOB GEOMETRY_SOURCES src/geometry/*.cxx)
file(GLOB PARAM_SOURCES src/parameter/*.cxx)
file(GLOB FIT_SOURCES src/fitmodels/*.cxx)
file(GLOB UTILS_SOURCES src/utils/*.cxx)
file(GLOB MODULE_SOURCES modules/*.cpp)

list(APPEND QCLAM_HEADERS
	TQClamAnalysis.h
	TQClamConstants.h
	structs/TQClamMbsHeader.h
	structs/TQClamScaler.h
	structs/TQClamTriggerDetectors.h
	source/TQClamTcpSource.h
	evt/TQClamHitEvent.h
	evt/TQClamClusterEvent.h
	evt/TQClamTrackEvent.h
	evt/TQClamTcpEvent.h
	evt/TQClamBackgroundRateEvent.h
	processor/TQClamHitProc.h
	processor/TQClamClusterProc.h
	processor/TQClamTrackProc.h
	processor/TQClamBackgroundRateProc.h
	store/TQClamDrifttimeCalibStore.h
	store/TQClamTrackStore.h
	store/TQClamTrackTestingStore.h
	store/TQClamDriftlengthCalibStore.h
	store/TQClamDagoStore.h

	
	${GEOMETRY_HEADERS}
	${FIT_HEADERS}
	${UTILS_HEADERS}
	${MODULE_HEADERS}
	#	include/geometry/*.h
	#	include/parameter/*.h
	#	include/fitmodels/*.h
	#	include/utils/*.h
	#	modules/*.h
	#	include/geometry/TQClamGeometry.h
	#	include/geometry/TQClamDriftcham.h
	#	include/geometry/TQClamWirePlane.h
	#	include/geometry/TQClamWire.h
	
	parameter/TQClamFitParameter.h
	parameter/TQClamClusterConditionParameter.h
	parameter/TQClamDisplaySettingParameter.h
	parameter/TQClamDriftchamCalibrationParameter.h
	parameter/TQClamDriftchamGeomParameter.h
	parameter/TQClamDrifttimeCalibParameter.h
	parameter/TQClamDriftToZOffsetParameter.h
	parameter/TQClamEdgeMappingParameter.h
	parameter/TQClamFaradayCupCurrentCalibrationParameter.h
	parameter/TQClamFocalPlaneParameter.h
	parameter/TQClamHitConditionParameter.h
	parameter/TQClamHitProcDebugOutputParameter.h
	parameter/TQClamMasterParameter.h
	parameter/TQClamScalerValueParameter.h
	parameter/TQClamSourceParameter.h
	parameter/TQClamTrackConditionParameter.h
	parameter/TQClamTriggerCalibrationParameter.h
	parameter/TQClamUnpackParameter.h
	parameter/TQClamStoreParameter.h
	parameter/TQClamHistogramParameter.h
	parameter/TQClamSpectrumCalibrationParameter.h
	parameter/TQClamSieveSlitCalibParameter.h
	#	include/fitmodels/TQClamFitModelDrifttimeDist.h
	#	include/fitmodels/TQClamFitModelEMG.h
	#	include/fitmodels/TQClamFitModelFermiDirac.h
	#	include/fitmodels/TQClamFitModelGaussTail.h
	#	include/fitmodels/TQClamFitModelTail1OverX.h
	#	include/utils/TQClamDriftCorrLoader.h


	# macro-wrapper
	)

list(APPEND QCLAM_SOURCES
	src/TQClamAnalysis.cxx
	src/source/TQClamTcpSource.cxx
	src/evt/TQClamHitEvent.cxx
	src/evt/TQClamClusterEvent.cxx
	src/evt/TQClamTrackEvent.cxx
	src/evt/TQClamTcpEvent.cxx
	src/evt/TQClamBackgroundRateEvent.cxx
	src/processor/TQClamHitProc.cxx
	src/processor/TQClamClusterProc.cxx
	src/processor/TQClamTrackProc.cxx
	src/processor/TQClamBackgroundRateProc.cxx

	src/store/TQClamDrifttimeCalibStore.cxx
	src/store/TQClamTrackStore.cxx
	src/store/TQClamTrackTestingStore.cxx
	src/store/TQClamDriftlengthCalibStore.cxx
	src/store/TQClamDagoStore.cxx

	src/parameter/TQClamFitParameter.cxx
	src/parameter/TQClamClusterConditionParameter.cxx
	src/parameter/TQClamDisplaySettingParameter.cxx
	src/parameter/TQClamDriftchamCalibration.cxx
	src/parameter/TQClamDriftchamGeomParameter.cxx
	src/parameter/TQClamDrifttimeCalibParameter.cxx
	src/parameter/TQClamDriftToZOffsetParameter.cxx
	src/parameter/TQClamEdgeMappingParameter.cxx
	src/parameter/TQClamFaradayCupCurrentCalibration.cxx
	src/parameter/TQClamFocalPlaneParameter.cxx
	src/parameter/TQClamHitConditionParameter.cxx
	src/parameter/TQClamHitProcDebugOutputParameter.cxx
	src/parameter/TQClamMasterParameter.cxx
	src/parameter/TQClamScalerValueParameter.cxx
	src/parameter/TQClamSourceParameter.cxx
	src/parameter/TQClamTrackConditionParameter.cxx
	src/parameter/TQClamTriggerCalibrationParameter.cxx
	src/parameter/TQClamUnpackParameter.cxx
	src/parameter/TQClamStoreParameter.cxx
	src/parameter/TQClamHistogramParameter.cxx
	src/parameter/TQClamSpectrumCalibrationParameter.cxx
	src/parameter/TQClamSieveSlitCalibParameter.cxx



	${GEOMETRY_SOURCES}
	${FIT_SOURCES}
	${UTILS_SOURCES}
	${MODULE_SOURCES}
	#	src/geometry/*.cxx
	#	src/fitmodels/*.cxx
	#	src/parameter/*.cxx
	#	src/utils/*.cxx
	#	modules/*.cxx
	)

# --- QCLAM_SINGLE ---
if(QCLAM_SINGLE)
	list(APPEND QCLAM_HEADERS
		evt/TQClamRawEvent.h
		processor/TQClamUnpackProc.h
		)
	list(APPEND QCLAM_SOURCES
		src/evt/TQClamRawEvent.cxx
		src/processor/TQClamUnpackProc.cxx
		)
endif()

if(QCLAM_SINGLE_ROOT)
	list(APPEND QCLAM_HEADERS
		source/TQClamOfflineRootSource.h
		evt/TQClamRawEvent.h
		)
	list(APPEND QCLAM_SOURCES
		src/source/TQClamOfflineRootSource.cxx
		src/evt/TQClamRawEvent.cxx
		)
endif()

if(QCLAM_DAGOBERT)
	list(APPEND QCLAM_HEADERS
		evt/TQClamDagobertRawEvent.h
		evt/TDagobertHitEvent.h
		evt/TQClamDagobertCoincidenceEvent.h
		parameter/TQClamTcpParameter.h
		parameter/TDagobertCalibrationParameter.h
		processor/TQClamDagobertUnpackProc.h
		processor/TDagobertHitProc.h
		processor/TQClamDagobertCoincidenceProc.h
		provider/TQClamDagobertProviderProc.h
		provider/TQClamDagobertDummyEvent.h
		)
	list(APPEND QCLAM_SOURCES
		src/evt/TQClamDagobertRawEvent.cxx
		src/evt/TDagobertHitEvent.cxx
		src/evt/TQClamDagobertCoincidenceEvent.cxx
		src/parameter/TQClamTcpParameter.cxx
		src/parameter/TDagobertCalibrationParameter.cxx
		src/processor/TQClamDagobertUnpackProc.cxx
		src/processor/TDagobertHitProc.cxx
		src/processor/TQClamDagobertCoincidenceProc.cxx
		src/provider/TQClamDagobertProviderProc.cxx
		src/provider/TQClamDagobertDummyEvent.cxx
		)
endif()


add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# --- Dictionary erzeugen ---
ROOT_GENERATE_DICTIONARY(QClamGo4Dict
    ${QCLAM_HEADERS}
    LINKDEF ${PROJECT_SOURCE_DIR}/QClamLinkDef.h
)

# --- EINE Library: Code + Dictionary ---
add_library(QClamGo4Analysis SHARED
    ${QCLAM_SOURCES}
    ${QCLAM_MODULES}
    QClamGo4Dict.cxx
)

target_link_libraries(QClamGo4Analysis PUBLIC
    ${ROOT_LIBRARIES}
    Go4Analysis
    Go4Base
    Go4ThreadManager
    Go4TaskHandler
    Go4Fit
    yaml-cpp::yaml-cpp
)

set_target_properties(QClamGo4Analysis PROPERTIES OUTPUT_NAME Go4UserAnalysis)

install(TARGETS QClamGo4Analysis DESTINATION lib)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libQClamGo4Dict.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libQClamGo4Dict_rdict.pcm
    DESTINATION lib
)

add_custom_command(TARGET QClamGo4Analysis POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:QClamGo4Analysis>
        ${CMAKE_CURRENT_BINARY_DIR}/libQClamGo4Dict.so
)
