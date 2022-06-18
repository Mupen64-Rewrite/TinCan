file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_config/")
configure_file("${CMAKE_CURRENT_LIST_DIR}/config.hpp.in" "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_config/config.hpp")
set(CONFIG_DIR "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_config/")