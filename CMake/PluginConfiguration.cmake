#
#  This file provides the standard configuration
#  for a generic plugin
#
MACRO(CONFIGURE_V3D_PLUGIN PLUGIN_GROUP_NAME PLUGIN_NAME)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} )

SET(QtITK_SRCS ${PLUGIN_NAME}.cxx )

QT4_WRAP_CPP(QT_MOC_SRCS ${PLUGIN_NAME}.h)

ADD_LIBRARY(${PLUGIN_NAME} SHARED ${QtITK_SRCS} ${QT_MOC_SRCS})
TARGET_LINK_LIBRARIES(${PLUGIN_NAME} ITKIO ${QT_LIBRARIES})

SET(PLUGIN_DESTINATION_DIR ${INSTALLATION_DIRECTORY}/${PLUGIN_GROUP_NAME} )

INSTALL(TARGETS ${PLUGIN_NAME}
  LIBRARY DESTINATION ${PLUGIN_DESTINATION_DIR} COMPONENT RuntimeLibraries
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
  )


IF(NOT SOLVED_RPATH_ISSUE)

  SET(LIBRARY_BUILD_DIR ${LIBRARY_OUTPUT_PATH})

  ADD_CUSTOM_COMMAND(
    SOURCE ${PLUGIN_NAME}
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${LIBRARY_BUILD_DIR}/${CMAKE_SHARED_MODULE_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_MODULE_SUFFIX} ${PLUGIN_DESTINATION_DIR}/${PLUGIN_NAME}${CMAKE_SHARED_MODULE_SUFFIX}
    TARGET ${PLUGIN_NAME}
    OUTPUTS ${LIBRARY_DESTINATION_DIR}/${PLUG}${CMAKE_SHARED_MODULE_SUFFIX}
    )

ENDIF(NOT SOLVED_RPATH_ISSUE)

ENDMACRO(CONFIGURE_V3D_PLUGIN)