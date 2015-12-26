project (oss_c_sdk_sample)

include_directories(${CMAKE_SOURCE_DIR})

set(PUT_OBJECT_SOURCE_FILES oss_sample_util.c 
			    oss_config.c 
			    oss_put_object_sample.c)

set(APPEND_OBJECT_SOURCE_FILES oss_sample_util.c 
			       oss_config.c 
			       oss_append_object_sample.c)

set(GET_OBJECT_SOURCE_FILES oss_sample_util.c 
			    oss_config.c 
			    oss_get_object_sample.c)

set(HEAD_OBJECT_SOURCE_FILES oss_sample_util.c 
			     oss_config.c 
			     oss_head_object_sample.c)

set(DELETE_OBJECT_SOURCE_FILES oss_sample_util.c 
  			       oss_config.c 
			       oss_delete_object_sample.c)

set(MULTIPART_UPLOAD_SOURCE_FILES oss_sample_util.c 
  				  oss_config.c 
				  oss_multipart_upload_sample.c)

include_directories (${APR_INCLUDE_DIR})
include_directories (${APR_UTIL_INCLUDE_DIR})
include_directories (${MINIXML_INCLUDE_DIR})
include_directories (${CURL_INCLUDE_DIR})
include_directories ("${CMAKE_SOURCE_DIR}/src")

find_library(APR_LIBRARY apr-1 PATHS /usr/local/apr/lib/)
find_library(APR_UTIL_LIBRARY aprutil-1 PATHS /usr/local/apr/lib/)
find_library(MINIXML_LIBRARY mxml)
find_library(CURL_LIBRARY curl)
find_library(PTHREAD_LIBRARY pthread)
find_library(RT_LIBRARY rt)

function(_TARGET_SAMPLE_LIBRARIES SAMPLE_BIN_NAME SOURCE_FILES)
	 add_executable(${SAMPLE_BIN_NAME} ${SOURCE_FILES})
	 target_link_libraries(${SAMPLE_BIN_NAME} oss_c_sdk)
	 target_link_libraries(${SAMPLE_BIN_NAME} ${APR_UTIL_LIBRARY})
	 target_link_libraries(${SAMPLE_BIN_NAME} ${APR_LIBRARY})
	 target_link_libraries(${SAMPLE_BIN_NAME} ${MINIXML_LIBRARY})
	 target_link_libraries(${SAMPLE_BIN_NAME} ${CURL_LIBRARY})
	 target_link_libraries(${SAMPLE_BIN_NAME} ${PTHREAD_LIBRARY})
	 target_link_libraries(${SAMPLE_BIN_NAME} ${RT_LIBRARY})      
endfunction()

_TARGET_SAMPLE_LIBRARIES(oss_put_object_sample "${PUT_OBJECT_SOURCE_FILES}")
_TARGET_SAMPLE_LIBRARIES(oss_get_object_sample "${GET_OBJECT_SOURCE_FILES}")
_TARGET_SAMPLE_LIBRARIES(oss_append_object_sample "${APPEND_OBJECT_SOURCE_FILES}")
_TARGET_SAMPLE_LIBRARIES(oss_head_object_sample "${HEAD_OBJECT_SOURCE_FILES}")
_TARGET_SAMPLE_LIBRARIES(oss_delete_object_sample "${DELETE_OBJECT_SOURCE_FILES}")
_TARGET_SAMPLE_LIBRARIES(oss_multipart_upload_sample "${MULTIPART_UPLOAD_SOURCE_FILES}")
