target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include)

# CMake doesn't allow linking of libraries outside of main CMake file
# Create a new project to link against later
project(spotify-qt-lib-third-party)
add_library(${PROJECT_NAME} STATIC)

# SQLite cache support
if (NOT DEFINED USE_DB_CACHE)
	set(USE_DB_CACHE ON)
endif ()

# TODO: On error, use bundled SQLite instead
if (USE_DB_CACHE)
	find_package(PkgConfig QUIET)
	if (PkgConfig_FOUND)
		pkg_check_modules(SQLITE sqlite3 QUIET)
		if (SQLITE_FOUND)
			target_link_directories(${PROJECT_NAME} PRIVATE "${SQLITE_LIBRARY_DIRS}")
			target_include_directories(${PROJECT_NAME} PRIVATE "${SQLITE_INCLUDE_DIRS}")
			target_link_libraries(${PROJECT_NAME} PRIVATE "${SQLITE_LIBRARIES}")
			message(STATUS "Using SQLite ${SQLITE_VERSION} (system)")
		else ()
			message(WARNING "SQLite error: sqlite3 not found")
		endif ()
	else ()
		message(WARNING "SQLite error: pkg-config not found")
	endif ()
endif ()