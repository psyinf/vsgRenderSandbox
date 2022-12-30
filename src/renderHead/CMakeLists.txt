SET(APP_NAME renderHead)
file(GLOB HEADER_FILES 	CONFIGURE_DEPENDS "*.h")
file(GLOB CPP_FILES 	CONFIGURE_DEPENDS "*.cpp")
set(SOURCES
    main.cpp
	${HEADER_FILES}
	${CPP_FILES}
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

