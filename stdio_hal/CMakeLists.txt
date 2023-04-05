
# declares the existance of a library
# will be compiled separately and linked into the bin
add_library(stdio_hal)

# setting a variable src_files will all .cpp files in the lib
set(src_files 
	${CMAKE_CURRENT_LIST_DIR}/stdio_hal.c
	${CMAKE_CURRENT_LIST_DIR}/stdio_hal_extension.c
	$ENV{TIVAWARE_PATH}/utils/uartstdio.c
	# more files would go here
)

# setting a variable in_files will all .h files in the lib
set(inc_files 
	${CMAKE_CURRENT_LIST_DIR}/include/stdio_hal/stdio_hal.h
	${CMAKE_CURRENT_LIST_DIR}/include/stdio_hal/stdio_hal_extension.h
	# more files would go here
)

# declaring the files in the library - 
target_sources(stdio_hal
	PRIVATE ${src_files}
	PUBLIC ${inc_files}
)

# this allows you to do #include "lib/srcA.h" when outside the library scope/source files
target_include_directories(stdio_hal PUBLIC include)

# this allows you to do #include "srcA.h" when inside the library scope/source files
target_include_directories(stdio_hal PRIVATE include/stdio_hal $ENV{TIVAWARE_PATH})

# Tivaware files & libraries
include_directories($ENV{TIVAWARE_PATH})

# establish private dependency on Tivaware/driverlib
target_link_libraries(stdio_hal PRIVATE $ENV{TIVAWARE_PATH}/driverlib/gcc/libdriver.a)
