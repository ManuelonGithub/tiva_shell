cmake_minimum_required(VERSION 3.8.2)

# Toolchain file
include(tm4c123g/tm4c123g.cmake)

###############################################################################
###                       Things to change per project                      ###
###############################################################################

# Project name
project(tiva_shell)

#Source files
set(SOURCES 
	"tm4c123g/startup.c"
	"src/tiva_shell.c"
	$ENV{TIVAWARE_PATH}/utils/uartstdio.c
	# any futher sources go here
)

set(LIBS
	stdio_hal
	shell
)

add_subdirectory(stdio_hal)
add_subdirectory(shell)

# IMPORTANT -> requires TIVAWARE_PATH as a env variable
# e.g. export TIVAWARE_PATH=$HOME/tiva/tivaware

###############################################################################
###                   ! Do not touch ! Toolchain Set=up !                   ###
###############################################################################

# Give nice error message telling you what's wrong
if(NOT DEFINED ENV{TIVAWARE_PATH})
    message(FATAL_ERROR "You must set TIVAWARE_PATH environment variable")
endif()

# create a library variable to make the linking cleaner-looking
set(TIVAWARE_LIB
	$ENV{TIVAWARE_PATH}/usblib/gcc/libusb.a
	$ENV{TIVAWARE_PATH}/driverlib/gcc/libdriver.a
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
	${TIVAWARE_LIB}
	${LIBS}
)

target_include_directories(${PROJECT_NAME} PRIVATE $ENV{TIVAWARE_PATH})

# Required to generate both .axf and .bin files
tiva_add_outputs(${PROJECT_NAME})

# Add new rule to make clean to it erases the files generated above
set_property(
	TARGET ${PROJECT_NAME}
	APPEND PROPERTY 
		ADDITIONAL_CLEAN_FILES 
			${PROJECT_NAME}.axf 
			${PROJECT_NAME}.bin 
			${PROJECT_NAME}.map
)

