set(SOURCES
    qnx-demo/curl_network_transport.cpp
    qnx-demo/main.cpp
    qnx-demo/test_data.cpp
) # QNX_DEMO_SOURCES

set(HEADERS
    qnx-demo/generic_network_transport.hpp
    qnx-demo/test_data.hpp
) # QNX_DEMO_HEADERS

add_executable(qnxdemo)
target_sources(qnxdemo PRIVATE ${SOURCES} ${HEADERS})

find_package(CURL REQUIRED)
target_link_libraries(qnxdemo PUBLIC CURL::libcurl)
