# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "PWCFB")
set(SDL2_DIR lib/SDL)
set(SDL2_IMAGE_DIR lib/SDL_image)
set(SDL2_MIXER_DIR lib/SDL_mixer)
set(SDL2_TTF_DIR lib/SDL_ttf)

project(${PROJECT_NAME} 
		VERSION 1.1
		DESCRIPTION ""
		HOMEPAGE_URL ""
		LANGUAGES C)

add_subdirectory(SDL2_DIR)

# find_package(SDL2 REQUIRED)
# include_directories(${SDL2_INCLUDE_DIRS})

add_executable(PWCFB
	src/main.c
	src/util.c
	src/util.h
	src/key.h
	src/anim.c
	src/anim.h)

target_link_libraries(${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME} PRIVATE ${SDL2_DIR}/include)

