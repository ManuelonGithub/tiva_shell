
# declares the existance of a library
# will be compiled separately and linked into the bin
add_library(shell)

# setting a variable src_files will all .cpp files in the lib
set(src_files 
	${CMAKE_CURRENT_LIST_DIR}/shell.c
	${CMAKE_CURRENT_LIST_DIR}/shell_history.c
	# more files would go here
)

# setting a variable in_files will all .h files in the lib
set(inc_files 
	${CMAKE_CURRENT_LIST_DIR}/config/shell_cfg.h
	${CMAKE_CURRENT_LIST_DIR}/include/shell/shell.h
	${CMAKE_CURRENT_LIST_DIR}/include/shell/shell_history.h
	# more files would go here
)

# declaring the files in the library - 
target_sources(shell
	PRIVATE ${src_files}
	PUBLIC ${inc_files}
)

# this allows you to do #include "lib/srcA.h" when outside the library scope/source files
target_include_directories(shell PUBLIC include)

# this allows you to do #include "srcA.h" when inside the library scope/source files
target_include_directories(shell PRIVATE include/shell config) 

# establish private dependency on Tivaware/driverlib
target_link_libraries(shell PRIVATE stdio_hal)