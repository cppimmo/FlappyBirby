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

# find_package(SDL2 REQUIRED)
# find_package(SDL2_image REQUIRED)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL2_DIR})
# add_subdirectory(${SDL2_IMAGE_DIR})
add_subdirectory(${SDL2_MIXER_DIR})
# add_subdirectory(${SDL2_TTF_DIR})
	
add_executable(${PROJECT_NAME}
	src/main.c
	src/util.c
	src/util.h
	src/key.h
	src/anim.c
	src/anim.h)

include_directories(
	${SDL2_DIR}/include
	${SDL2_IMAGE_DIR}/include
	${SDL2_MIXER_DIR}/include
	${SDL2_TTF_DIR}/include)

link_directories(
	${SDL2_IMAGE_DIR}/build
	${SDL2_MIXER_DIR}/build
	${SDL2_TTF_DIR}/build
	lib/zlib-1.2.11)
	
target_link_libraries(${PROJECT_NAME}
	SDL2main SDL2-static SDL2_image SDL2_mixer SDL2_ttf) # SDL2_image

# https://stackoverflow.com/questions/23850472/how-to-use-sdl2-and-sdl-image-with-cmake
# https://stackoverflow.com/questions/28395833/using-sdl2-with-cmake
# https://stackoverflow.com/questions/61865717/how-to-link-sdl2-in-cmake
