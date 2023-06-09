
# Processor specific definitions
add_definitions(-DPART_TM4C123GH6PM)
add_definitions(-DTARGET_IS_TM4C123_RA1)
add_definitions(-Dgcc)

#Set cross compilation information
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# GCC toolchain prefix
set(TOOLCHAIN_PREFIX "arm-none-eabi")

# Make sure we find all the toolchains we need to compile 
find_program(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++ REQUIRED)
find_program(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-as REQUIRED)
find_program(CMAKE_AR ${TOOLCHAIN_PREFIX}-ar REQUIRED)
find_program(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}-objcopy REQUIRED)
find_program(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}-objdump REQUIRED)

enable_language(ASM)

# Set Compile Flags - don't override cache though
set(CPU "-mcpu=cortex-m4")
set(FPU "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(Oxx "-Os -ffunction-sections -fdata-sections")
set(Wxx "-Wall -pedantic")

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mthumb ${CPU} ${FPU} -MD")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb ${CPU} ${FPU} -std=gnu99 ${Oxx} -MD ${Wxx} ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb ${CPU} ${FPU} ${Oxx} -MD ${Wxx} -fno-exceptions -fno-rtti")

# Generate Map File
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map,${CMAKE_PROJECT_NAME}.map")
set(CMAKE_CXX_LINK_FLAGS "$CMAKE_CXX_LINK_FLAGS} -Wl,-Map,${CMAKE_PROJECT_NAME}.map")

# Add Linker Scrript and specs file
set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_LIST_DIR}/tm4c123g.ld -specs=${CMAKE_CURRENT_LIST_DIR}/tiva.specs")

# Function generates all desired outputs of a build
# By default the binary generated by cmake has no extension
# So this functions add the extension 
# It also generates a .bin file that can flashed onto the board
function(tiva_add_outputs TARGET)
	add_custom_command(TARGET ${TARGET} POST_BUILD
		COMMAND mv ${TARGET} ${TARGET}.axf
		COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET}.axf ${CMAKE_PROJECT_NAME}.bin
	)
endfunction()

# Give nice error message telling you what's wrong
if(NOT DEFINED ENV{LM4FLASH_PATH})
    message(FATAL_ERROR "LM4FLASH_PATH environment variable not set!")
endif()

# Find the flash tool
find_program(LM4FLASH "$ENV{LM4FLASH_PATH}/lm4flash" REQUIRED)

# add new make target for flashing - make flash
ADD_CUSTOM_TARGET("flash"
  COMMAND ${LM4FLASH} ${CMAKE_PROJECT_NAME}.bin
)

# add new make target for flashing - make flash
ADD_CUSTOM_TARGET("tivaware"
  COMMAND make clean -C $ENV{TIVAWARE_PATH}
  COMMAND make -C $ENV{TIVAWARE_PATH} FPU=${FPU}
)