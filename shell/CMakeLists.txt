
# declares the existance of a library
# will be compiled separately and linked into the bin
set(LIBRARY_NAME shell)

add_library(${LIBRARY_NAME})

# setting a variable src_files will all .cpp files in the lib
set(src_files 
	${CMAKE_CURRENT_LIST_DIR}/shell.c
	${CMAKE_CURRENT_LIST_DIR}/shell_history.c
	${CMAKE_CURRENT_LIST_DIR}/shell_readline.c
	# more files would go here
)

# setting a variable in_files will all .h files in the lib
set(inc_files 
	${CMAKE_CURRENT_LIST_DIR}/config/shell_cfg.h
	${CMAKE_CURRENT_LIST_DIR}/include/shell/shell.h
	# more files would go here
)

set(prv_inc_files
	${CMAKE_CURRENT_LIST_DIR}/include/prv/shell_readline.h
	${CMAKE_CURRENT_LIST_DIR}/include/prv/shell_readline_prv.h
	${CMAKE_CURRENT_LIST_DIR}/include/prv/shell_history.h
)

# declaring the files in the library - 
target_sources(${LIBRARY_NAME}
	PRIVATE ${src_files} ${prv_inc_files}
	PUBLIC ${inc_files}
)

# this allows you to do #include "lib/srcA.h" when outside the library scope/source files
target_include_directories(${LIBRARY_NAME} PUBLIC include)

# this allows you to do #include "srcA.h" when inside the library scope/source files
target_include_directories(${LIBRARY_NAME} PRIVATE config include/prv include/shell) 

# establish private dependency on Tivaware/driverlib
target_link_libraries(${LIBRARY_NAME} PRIVATE stdio_hal)