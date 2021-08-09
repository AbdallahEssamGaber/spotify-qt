target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include)

# SQLiteCpp
add_subdirectory(SQLiteCpp)

# Link
add_library(${PROJECT_NAME} STATIC)
target_link_libraries(${PROJECT_NAME} PRIVATE
	SQLiteCpp
	sqlite3)