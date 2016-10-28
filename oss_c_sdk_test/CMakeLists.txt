project (oss_c_sdk_test)

include_directories(${CMAKE_SOURCE_DIR})

configure_file (
  "${PROJECT_SOURCE_DIR}/test.h.in"
  "${PROJECT_SOURCE_DIR}/test.h"
)

set(SAMPLE_SOURCE_FILES CuTest.c oss_test_util.c oss_config.c test_oss_bucket.c 
    test_oss_object.c test_oss_multipart.c test_oss_live.c test_oss_image.c 
	test_oss_progress.c test_oss_callback.c test_oss_crc.c cjson_utils.c cjson.c
	test_aos.c test_all.c)

# find_path(APR_INCLUDE_DIR apr-1/apr_time.h)
# find_path(APR_UTIL_INCLUDE_DIR apr/include/apr-1/apr_md5.h)
# find_path(MINIXML_INCLUDE_DIR mxml.h)
# find_path(CURL_INCLUDE_DIR curl/curl.h)

include_directories (${APR_INCLUDE_DIR})
include_directories (${APR_UTIL_INCLUDE_DIR})
include_directories (${MINIXML_INCLUDE_DIR})
include_directories (${CURL_INCLUDE_DIR})
include_directories ("${CMAKE_SOURCE_DIR}/oss_c_sdk")

find_library(APR_LIBRARY apr-1 PATHS /usr/local/apr/lib/)
find_library(APR_UTIL_LIBRARY aprutil-1 PATHS /usr/local/apr/lib/)
find_library(MINIXML_LIBRARY mxml)
find_library(CURL_LIBRARY curl)
find_library(PTHREAD_LIBRARY pthread)
find_library(MATH_LIBRARY m)
#find_library(SSL_LIBRARY ssl)
#find_library(IDN_LIBRARY idn)
find_library(RT_LIBRARY rt)
#find_library(OSS_C_SDK_LIBRARY oss_c_sdk HINTS ${LIBRARY_OUTPUT_PATH})

add_executable(oss_c_sdk_test ${SAMPLE_SOURCE_FILES})

target_link_libraries(oss_c_sdk_test oss_c_sdk)
target_link_libraries(oss_c_sdk_test ${APR_UTIL_LIBRARY})
target_link_libraries(oss_c_sdk_test ${APR_LIBRARY})
target_link_libraries(oss_c_sdk_test ${MINIXML_LIBRARY})
target_link_libraries(oss_c_sdk_test ${CURL_LIBRARY})
target_link_libraries(oss_c_sdk_test ${PTHREAD_LIBRARY})
target_link_libraries(oss_c_sdk_test ${MATH_LIBRARY})
#target_link_libraries(oss_c_sdk_test ${SSL_LIBRARY})
#target_link_libraries(oss_c_sdk_test ${IDN_LIBRARY})
target_link_libraries(oss_c_sdk_test ${RT_LIBRARY})
