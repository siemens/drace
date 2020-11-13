set(SOURCES
  src/DetectorPy
  src/bindings)

Python_add_library(dracepy MODULE ${SOURCES})
target_link_libraries(dracepy PUBLIC
  "drace.detector.fasttrack.standalone"
  Boost::python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
target_include_directories(dracepy PRIVATE include)

install(TARGETS dracepy
  RUNTIME DESTINATION ${DRACE_RUNTIME_DEST}/dracepy
  LIBRARY DESTINATION ${DRACE_RUNTIME_DEST}/dracepy
  ARCHIVE DESTINATION ${DRACE_ARCHIVE_DEST}/dracepy)
install(FILES example.py DESTINATION ${DRACE_RUNTIME_DEST}/dracepy)

if(WIN32)
  get_filename_component(BPL_BASE_NAME ${Boost_PYTHON${Python_VERSION_MAJOR}${Python_VERSION_MINOR}_LIBRARY_RELEASE} NAME_WE)
  get_filename_component(BPL_PATH ${Boost_PYTHON${Python_VERSION_MAJOR}${Python_VERSION_MINOR}_LIBRARY_RELEASE} DIRECTORY)
  install(FILES
    ${BPL_PATH}/${BPL_BASE_NAME}.dll
    DESTINATION ${DRACE_RUNTIME_DEST}/dracepy)
endif()
