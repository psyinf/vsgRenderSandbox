find_program(conanexecutable "conan")
if(NOT conanexecutable)
    message(WARNING "Tool conan is not installed. Check README.md for build instructions without conan.")
else()
	file(COPY conanfile.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    message(STATUS "Found conan. Installing dependencies.")
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                      "${CMAKE_BINARY_DIR}/conan.cmake")
		message(STATUS "Finished downloading conan.cmake")
    endif()


    include(${CMAKE_BINARY_DIR}/conan.cmake)

    #if (NOT_DEFINED ${CMAKE_CONFIGURATION_TYPES})    	
    #	message(STATUS "TYPES: ${CMAKE_CONFIGURATION_TYPES}")
    #	message(STATUS "Building missing conan packages")
	#	conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})
	#	conan_cmake_install(PATH_OR_REFERENCE .
    #                   BUILD missing
    #                   REMOTE conancenter
    #                   SETTINGS ${settings})
    #else()
	foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
		message(STATUS "Building missing conan packages")
		conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})
		conan_cmake_install(PATH_OR_REFERENCE .
                        BUILD missing
                        REMOTE conancenter
                        SETTINGS ${settings})
	endforeach()
    #endif()	
    set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" ${CMAKE_MODULE_PATH})
	set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" ${CMAKE_PREFIX_PATH})
endif()
