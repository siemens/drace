add_library("drace.detector.tracebinary" SHARED "TraceBinary")
target_link_libraries("drace.detector.tracebinary" "drace-common")
configure_DynamoRIO_standalone("drace.detector.tracebinary")

install(TARGETS "drace.detector.tracebinary"
	RUNTIME DESTINATION ${DRACE_RUNTIME_DEST} COMPONENT Runtime
	LIBRARY DESTINATION ${DRACE_ARCHIVE_DEST} COMPONENT ARCHIVE)
