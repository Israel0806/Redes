project( GradiusClone )

message( "Configuring main application - ${PROJECT_NAME} : " )

# OpenGL
find_package(OpenGL REQUIRED)

file(GLOB SOURCES "src/*.cpp" )
file(GLOB HEADERS "include/*.h" )
file(GLOB SHADERS "shaders/*.vs" "shaders/*.fs" "shaders/*.gs" )

include_directories( 
"${CMAKE_CURRENT_SOURCE_DIR}/src"
"${CMAKE_CURRENT_SOURCE_DIR}/include"

${DEPENDENCY_DIR}/include/glad/ 
${DEPENDENCY_DIR}/include/glfw/
${DEPENDENCY_DIR}/include/stb_image/
${DEPENDENCY_DIR}/include/glm
${DEPENDENCY_DIR}/include/assimp
)
message( "\nBuild mode test is: ${CMAKE_BUILD_TYPE}" )

if ( CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("\nOpenGL_GLUT_GLFW_Program -- Debug mode ****************")
	link_directories(
	${DEPENDENCY_DIR}/lib/glad/Debug/
	${DEPENDENCY_DIR}/lib/glfw/Debug/
	${DEPENDENCY_DIR}/lib/glm/Debug/
	${DEPENDENCY_DIR}/lib/assimp/Debug/
	)
else()
    message("\nOpenGL_GLUT_GLFW_Program -- Release mode ****************")
	link_directories(
	${DEPENDENCY_DIR}/lib/glad/Release/
	${DEPENDENCY_DIR}/lib/glfw/Release/
	${DEPENDENCY_DIR}/lib/glm/Release/
	${DEPENDENCY_DIR}/lib/assimp/Release/
	)
endif ()
 
add_executable( ${PROJECT_NAME}  ${HEADERS} ${SOURCES} ${SHADERS} )

if ( CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("\nOpenGL_GLUT_GLFW_Program -- Debug mode ****************")
	target_link_libraries( ${PROJECT_NAME} 
					   opengl32
					   glad.lib
					   glfw3.lib
					   glm_static.lib
					   assimp-vc140-mt.lib
					    )	
else()
	message("\nOpenGL_GLUT_GLFW_Program -- Release mode ****************")
	target_link_libraries( ${PROJECT_NAME} 
					   opengl32
					   glad.lib
        		                   glfw3.lib
					   glm_static.lib
					    )	
endif ()


                           