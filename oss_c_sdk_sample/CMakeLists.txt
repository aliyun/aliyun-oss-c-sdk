project (oss_c_sdk_sample)

include_directories(${CMAKE_SOURCE_DIR})

set(OSS_SAMPLE_SOURCE_FILES oss_sample_util.c oss_config.c main.c 
    oss_put_object_sample.c oss_append_object_sample.c oss_get_object_sample.c
    oss_head_object_sample.c oss_delete_object_sample.c oss_multipart_upload_sample.c
    oss_crc_sample.c oss_image_sample.c oss_progress_sample.c oss_callback_sample.c
    oss_list_object_sample.c oss_resumable_sample.c oss_get_object_meta_sample.c 
    oss_put_object_acl_sample.c oss_get_object_acl_sample.c)

include_directories (${APR_INCLUDE_DIR})
include_directories (${APR_UTIL_INCLUDE_DIR})
include_directories (${MINIXML_INCLUDE_DIR})
include_directories (${CURL_INCLUDE_DIR})
include_directories ("${CMAKE_SOURCE_DIR}/oss_c_sdk")

add_executable(oss_c_sdk_sample ${OSS_SAMPLE_SOURCE_FILES})

if (CMAKE_HOST_WIN32)
    target_link_libraries(oss_c_sdk_sample ws2_32)
else()
	find_library(APR_LIBRARY apr-1 PATHS /usr/local/apr/lib/)
	find_library(APR_UTIL_LIBRARY aprutil-1 PATHS /usr/local/apr/lib/)
	find_library(MINIXML_LIBRARY mxml)
	find_library(CURL_LIBRARY curl)
	find_library(PTHREAD_LIBRARY pthread)
    target_link_libraries(oss_c_sdk_sample ${PTHREAD_LIBRARY})
endif()

target_link_libraries(oss_c_sdk_sample oss_c_sdk)
target_link_libraries(oss_c_sdk_sample ${APR_UTIL_LIBRARY})
target_link_libraries(oss_c_sdk_sample ${APR_LIBRARY})
target_link_libraries(oss_c_sdk_sample ${MINIXML_LIBRARY})
target_link_libraries(oss_c_sdk_sample ${CURL_LIBRARY})
