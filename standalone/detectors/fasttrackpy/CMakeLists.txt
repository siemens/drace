Python_add_library(fasttrackpy MODULE fasttrack.cpp)
target_link_libraries(fasttrackpy PUBLIC
  "drace.detector.fasttrack.standalone"
  Boost::python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})

install(TARGETS fasttrackpy
  RUNTIME DESTINATION ${DRACE_RUNTIME_DEST}/fasttrack
  LIBRARY DESTINATION ${DRACE_RUNTIME_DEST}/fasttrack
  ARCHIVE DESTINATION ${DRACE_ARCHIVE_DEST}/fasttrack)
install(FILES fasttrack.py DESTINATION ${DRACE_RUNTIME_DEST}/fasttrack)

if(WIN32)
  get_filename_component(BPL_BASE_NAME ${Boost_PYTHON${Python_VERSION_MAJOR}${Python_VERSION_MINOR}_LIBRARY_RELEASE} NAME_WE)
  get_filename_component(BPL_PATH ${Boost_PYTHON${Python_VERSION_MAJOR}${Python_VERSION_MINOR}_LIBRARY_RELEASE} DIRECTORY)
  install(FILES
    ${BPL_PATH}/${BPL_BASE_NAME}.dll
    DESTINATION ${DRACE_RUNTIME_DEST}/fasttrack)
endif()
