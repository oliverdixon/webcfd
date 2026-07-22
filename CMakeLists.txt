# EchoMap CMakeLists.txt
#
# Author: Oliver Dixon
# Date: 2026-06-05

cmake_minimum_required(VERSION 4.0)
project(EchoMap)

# The DummyTarget provides a target to IntelliJ IDEs to manage. We extract the source vector for the real targets.
add_library(DummyTarget INTERFACE EXCLUDE_FROM_ALL
        src/EchoMap.cpp
        src/EchoMap.hpp
        src/App.cpp
        src/panels/IPanel.hpp
        src/RobotoMedium.hpp
        src/utility/Logger.cpp
        src/utility/Logger.hpp
        src/errors/ConfigurationError.cpp
        src/errors/ConfigurationError.hpp
        src/objects/factories/SignalFactory.cpp
        src/objects/factories/SignalFactory.hpp
        src/objects/Signal.cpp
        src/objects/Signal.hpp
        src/objects/Sensor.cpp
        src/objects/Sensor.hpp
        src/panels/ProjectPanel.cpp
        src/panels/ProjectPanel.hpp
        src/panels/MenuPanel.cpp
        src/panels/MenuPanel.hpp
        src/objects/Project.cpp
        src/objects/Project.hpp
        src/objects/BidirectionalUnorderedMapping.hpp
        src/objects/Object.hpp
        src/objects/IDAllocator.hpp
        src/panels/ErrorModal.cpp
        src/panels/ErrorModal.hpp
        src/SurfaceFactory.cpp
        src/SurfaceFactory.hpp
        src/errors/LocatableError.cpp
        src/errors/LocatableError.hpp
        src/objects/persistence/JSONSerialiser.cpp
        src/objects/persistence/JSONSerialiser.hpp
        src/objects/persistence/ISerialiser.hpp
        src/objects/persistence/IDeserialiser.hpp
        src/objects/persistence/JSONDeserialiser.cpp
        src/objects/persistence/JSONDeserialiser.hpp
        src/panels/SignalWaveformPanel.cpp
        src/panels/SignalWaveformPanel.hpp
        src/panels/SensorGeometryPanel.cpp
        src/panels/SensorGeometryPanel.hpp
        src/panels/ChannelMappingPanel.cpp
        src/panels/ChannelMappingPanel.hpp
        src/signals/ThreadSafeQueue.hpp
        src/signals/tasks/ITask.hpp
        src/signals/tasks/LoadProjectTask.cpp
        src/signals/tasks/LoadProjectTask.hpp
        src/signals/Worker.cpp
        src/signals/Worker.hpp
        src/signals/results/ErrorResult.cpp
        src/signals/results/ErrorResult.hpp
        src/signals/results/LoadProjectResult.cpp
        src/signals/results/LoadProjectResult.hpp
        src/signals/tasks/DownsampleTask.cpp
        src/signals/tasks/DownsampleTask.hpp
        src/signals/results/DownsampleResult.cpp
        src/signals/results/DownsampleResult.hpp
        src/objects/FrequencySpectrum.cpp
        src/objects/FrequencySpectrum.hpp
        src/objects/factories/FrequencySpectrumFactory.cpp
        src/objects/factories/FrequencySpectrumFactory.hpp
        src/panels/SignalDFTPanel.cpp
        src/panels/SignalDFTPanel.hpp
        src/signals/tasks/DFTTask.cpp
        src/signals/tasks/DFTTask.hpp
        src/signals/results/DFTResult.cpp
        src/signals/results/DFTResult.hpp
        src/notifications/AddChannelMappingNotification.hpp
        src/notifications/ModifySensorPositionNotification.hpp
        src/notifications/ModifySensorColourNotification.hpp
        src/objects/factories/FFTWBuffers.cpp
        src/objects/factories/FFTWBuffers.hpp
        src/signals/tasks/ITask.cpp
        src/signals/WorkerResultDespatcher.cpp
        src/signals/WorkerResultDespatcher.hpp
        src/signals/ResultChannel.hpp
        src/objects/factories/WindowFunctions.hpp
        src/objects/factories/WindowFunctions.cpp
        src/utility/VariantHelpers.hpp
        src/web/EmscriptenExtra.hpp
        src/signals/tasks/LoadSignalFileTask.cpp
        src/signals/tasks/LoadSignalFileTask.hpp
        src/signals/results/LoadSignalFileResult.cpp
        src/signals/results/LoadSignalFileResult.hpp
        src/panels/IndividualUploadModal.cpp
        src/panels/IndividualUploadModal.hpp
        src/actions/ActionController.hpp
        src/actions/ActionControllerBase.hpp
        src/StaticInstanceController.cpp
        src/StaticInstanceController.hpp
        src/errors/IgnoredWarning.cpp
        src/errors/IgnoredWarning.hpp
        src/notifications/ProjectSelectedNotification.hpp
        src/notifications/CompleteProjectLoadNotification.hpp
        src/notifications/RegisterVFSMappingNotification.hpp
        src/notifications/RegisterVFSMappingNotification.cpp
        src/notifications/CompleteProjectLoadNotification.cpp
        src/notifications/AddChannelMappingNotification.cpp
        src/notifications/ModifySensorPositionNotification.cpp
        src/notifications/ModifySensorColourNotification.cpp
        src/signals/results/WorkerResult.hpp
        src/utility/Colour.hpp
        src/utility/Position.hpp
        src/notifications/CancelProjectLoadNotification.cpp
        src/notifications/CancelProjectLoadNotification.hpp
        src/notifications/AllNotifications.hpp
        src/panels/NativeFileChooser.cpp
        src/panels/NativeFileChooser.hpp
)

if (EMSCRIPTEN OR ECHOMAP_BUILD_DOCUMENTATION)
    target_sources(DummyTarget PRIVATE
            src/actions/JSActionController.cpp
            src/actions/JSActionController.hpp
    )
endif ()

if (NOT EMSCRIPTEN OR ECHOMAP_BUILD_DOCUMENTATION)
    target_sources(DummyTarget PRIVATE
            src/actions/NativeActionController.cpp
            src/actions/NativeActionController.hpp
    )
endif ()

get_target_property(ECHOMAP_SOURCES DummyTarget SOURCES)

include(cmake/ApplicationTargets.cmake)
include(cmake/DoxygenTarget.cmake)
