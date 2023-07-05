
set(SEG_GIF_LIBRARIES 
  _seg_GIF
)


foreach(library ${SEG_GIF_LIBRARIES} )

  add_library(${library} ${${PROJECT_NAME}_BUILD_TYPE}
    ${library}.cpp
    )
  add_dependencies(${library} Eigen)

  target_link_libraries(${library} 
    ${NIFTYREG_LIBRARIES}
    ${NIFTYSEG_LIBRARIES}
    )  

  install(TARGETS ${library}
    BUNDLE DESTINATION  lib
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

endforeach()


if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB __files1 "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  file(GLOB __files2 "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
  file(GLOB __files3 "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
  install(FILES ${__files1} ${__files2} ${__files3}
    DESTINATION include
    COMPONENT   Development
    )
endif(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
