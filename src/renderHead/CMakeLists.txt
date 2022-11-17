SET(APP_NAME renderHead)

set(SOURCES
    main.cpp
)

find_package(vsg 1.0.0 REQUIRED)


add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} vsg::vsg)



install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION bin
)

