# Copyright 2025 Robert Carey
# Copyright 2025-2026 Morse Micro
# SPDX-License-Identifier: Apache-2.0

set(src "mmhalow.c")

set(inc ".")

set(reqs
	esp_netif
	esp_wifi
)

idf_component_register(
	INCLUDE_DIRS
	${inc}
	SRCS
	${src}
	REQUIRES
	${reqs}
)

idf_component_get_property(component_version ${COMPONENT_NAME} COMPONENT_VERSION)
target_compile_definitions(
    ${COMPONENT_LIB}
    PUBLIC HALOW_VERSION="${component_version}"
)

add_subdirectory(components)
add_dependencies(${COMPONENT_TARGET} mmpktmem shims mmutils morselib regdb)
target_link_libraries(${COMPONENT_TARGET} PUBLIC mmpktmem shims mmutils morselib regdb)
