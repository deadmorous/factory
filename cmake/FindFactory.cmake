find_library (FACTORY_LIBRARY NAMES libfactory.so factory.dll PATHS "${CMAKE_CURRENT_LIST_DIR}/../lib")
find_path(FACTORY_INCLUDE_DIRS "factory/Factory.hpp" PATHS "${CMAKE_CURRENT_LIST_DIR}/../include")
set (FACTORY_LIBRARIES ${FACTORY_LIBRARY})

