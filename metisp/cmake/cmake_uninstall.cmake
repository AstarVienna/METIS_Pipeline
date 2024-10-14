cmake_policy(PUSH)
cmake_policy(SET CMP0153 NEW)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: ${CMAKE_BINARY_DIR}/install_manifest.txt")
endif()

file(READ "${CMAKE_BINARY_DIR}/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
    message(STATUS "Uninstalling $ENV{DESTDIR}${file}")
    if(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E remove "$ENV{DESTDIR}${file}"
            OUTPUT_VARIABLE output
            RESULT_VARIABLE result)
        if(NOT "${result}" STREQUAL 0)
            message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${file}")
        endif()
    else(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
        message(STATUS "File $ENV{DESTDIR}${file} does not exist.")
    endif()
endforeach()
cmake_policy(POP)