# CMake Version
cmake_minimum_required(VERSION 3.1.0)

# Project name
project(image_processing)

# File name
set(FILE_NAME image_processing)

# --- Libraiies --- #

# Add OpenCV
find_package(
    OpenCV 
    REQUIRED
    )

# --- Executable generation --- #

# Executable
add_executable(${FILE_NAME} ${FILE_NAME}.cpp)

# Indiques les bibliothèques desquelles dépend le script !
set(DEPENDENCIES ${OpenCV_LIBS})
target_link_libraries(${FILE_NAME} ${DEPENDENCIES})
