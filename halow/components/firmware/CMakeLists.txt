# Copyright 2026 Morse Micro
# SPDX-License-Identifier: Apache-2.0

if(NOT (CONFIG_MM_CHIP_MM6108 OR CONFIG_MM_CHIP_MM8108) OR
   NOT CONFIG_MM_BCF_FILE)
    # lib def and no FATAL_ERROR to prevent blocking set-target and menuconfig
    add_library(morsefirmware INTERFACE)
    add_custom_target(mm_fw_config_error ALL
        COMMAND ${CMAKE_COMMAND} -E echo cmake_echo_color --red --bold
            "ERROR: CONFIG_MM_CHIP and CONFIG_MM_BCF_FILE must be set. Set manually via menuconfig or use a sdkconfig.defaults found in managed_components/morsemicro__halow/configs"
        COMMAND ${CMAKE_COMMAND} -E false
    )
    return()
endif()

set(SDK_FW_ROOT "${MM_IOT_SDK}/framework/morsefirmware")

set(BCF_SEARCH_ROOTS
    "${SDK_FW_ROOT}"
    "${CMAKE_CURRENT_LIST_DIR}/morse-firmware/bcf"
)
set(FW_SEARCH_ROOTS
    "${SDK_FW_ROOT}"
)

if(CONFIG_MM_CHIP_MM6108)
    set(FW_FILE "mm6108.mbin")
elseif(CONFIG_MM_CHIP_MM8108)
    set(FW_FILE "mm8108b2-rl.mbin")
endif()

# SEARCH_ROOTS is a CMake list (semicolon-separated) of directories to search in order.
function(find_binary_blob OUT_VAR REQUESTED_NAME SEARCH_ROOTS)
    get_filename_component(req_dir  "${REQUESTED_NAME}" DIRECTORY)
    get_filename_component(req_name "${REQUESTED_NAME}" NAME)
    string(REGEX REPLACE "\\.(bin|mbin)$" "" base "${req_name}")

    # Candidate filenames, (user > mbin > bin).
    if(req_name MATCHES "\\.bin$")
        set(exts bin mbin)
    else()
        set(exts mbin bin)
    endif()
    list(GET exts 0 preferred_ext)
    set(candidates "")
    foreach(ext IN LISTS exts)
        list(APPEND candidates "${base}.${ext}")
    endforeach()

    set(found "")

    foreach(cand IN LISTS candidates)
        if(req_dir)
            set(rel "${req_dir}/${cand}")
        else()
            set(rel "${cand}")
        endif()
        get_filename_component(abs "${rel}" ABSOLUTE BASE_DIR "${PROJECT_DIR}")
        if(EXISTS "${abs}" AND NOT IS_DIRECTORY "${abs}")
            set(found "${abs}")
            break()
        endif()
    endforeach()

    if(NOT found)
        foreach(search_root IN LISTS SEARCH_ROOTS)
            foreach(cand IN LISTS candidates)
                file(GLOB_RECURSE matches LIST_DIRECTORIES false "${search_root}/${cand}")
                if(matches)
                    list(LENGTH matches n)
                    if(n GREATER 1)
                        message(WARNING "Multiple matches for '${cand}' under ${search_root}:\n"
                                        "  ${matches}\n"
                                        "Using the first one.")
                    endif()
                    list(GET matches 0 found)
                    break()
                endif()
            endforeach()
            if(found)
                break()
            endif()
        endforeach()
    endif()

    if(NOT found)
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()

    get_filename_component(found_ext "${found}" LAST_EXT)
    if(NOT found_ext STREQUAL ".${preferred_ext}")
        message(WARNING "${OUT_VAR}: exact match not found, "
                        "substituted '${found}' for requested '${base}.${preferred_ext}'")
    endif()
    message(STATUS "${OUT_VAR}: ${found}")

    # Convert .bin -> .mbin; an existing .mbin passes through unchanged.
    if(found_ext STREQUAL ".bin")
        idf_build_get_property(python PYTHON)
        get_filename_component(name "${found}" NAME)
        set(mbin "${CMAKE_CURRENT_BINARY_DIR}/${name}.mbin")
        add_custom_command(
            OUTPUT "${mbin}"
                COMMAND ${python} ${MM_IOT_SDK}/framework/tools/buildsystem/convert-bin-to-mbin.py
                    -t 1,2:2 -o "${mbin}" "${found}"
            DEPENDS "${found}"
        )
        message(STATUS "Converting ${name} to mbin")
        set(found "${mbin}")
    endif()

    set(${OUT_VAR} "${found}" PARENT_SCOPE)
endfunction()

find_binary_blob(BCF_MBIN "${CONFIG_MM_BCF_FILE}" "${BCF_SEARCH_ROOTS}")
find_binary_blob(FW_MBIN  "${FW_FILE}" "${FW_SEARCH_ROOTS}")

if(NOT BCF_MBIN)
    add_library(morsefirmware INTERFACE)
    add_custom_target(_mm_bcf_missing_error ALL
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red --bold
            "ERROR: BCF file not found. Check that the value of CONFIG_MM_BCF_FILE is a valid relative path to a BCF or that it exists in morse-firmware"
        COMMAND ${CMAKE_COMMAND} -E false
        VERBATIM
    )
    return()
endif()
if(NOT FW_MBIN)
    add_library(morsefirmware INTERFACE)
    add_custom_target(_mm_fw_missing_error ALL
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red --bold
        "ERROR: Morse Micro firmware binary is missing. Refresh the morsemicro__halow/components/mm-iot-sdk submodule"
        COMMAND ${CMAKE_COMMAND} -E false
        VERBATIM
    )
    return()
endif()

if(CONFIG_HALOW_COMPRESS_FIRMWARE)
    idf_build_get_property(python PYTHON)
    get_filename_component(name "${FW_MBIN}" NAME)
    set(FW_FINAL "${CMAKE_CURRENT_BINARY_DIR}/${FW_MBIN}.mbin")
    add_custom_command(
        OUTPUT "${FW_FINAL}"
        COMMAND
            ${python}
            ${COMPONENT_DIR}/tools/compress-mbin.py
            "${FW_MBIN}"
            -o "${FW_FINAL}"
        DEPENDS "${FW_MBIN}"
    )
else()
    set(FW_FINAL "${FW_MBIN}")
endif()

add_library(morsefirmware STATIC)
target_add_binary_data(morsefirmware "${FW_FINAL}"  BINARY RENAME_TO firmware_binary)
target_add_binary_data(morsefirmware "${BCF_MBIN}" BINARY RENAME_TO bcf_binary)
