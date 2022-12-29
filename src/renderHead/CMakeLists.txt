SET(APP_NAME renderHead)

set(SOURCES
    main.cpp
)

find_package(vsg 1.0.0 REQUIRED)
find_package(ModernCppKafka CONFIG REQUIRED)	
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} 
    vsg::vsg
    nlohmann_json::nlohmann_json
    ModernCppKafka::ModernCppKafka
)



install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION bin
)

