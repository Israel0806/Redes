

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Shader {
private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors (unsigned int shader, string type, string path = "") {

		int success;
		char infoLog[1024];
		if ( type != "PROGRAM" ) {
			glGetShaderiv (shader, GL_COMPILE_STATUS, &success);
			if ( !success ) {
				glGetShaderInfoLog (shader, 1024, NULL, infoLog);
				cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
			}
		} else {
			glGetProgramiv (shader, GL_LINK_STATUS, &success);
			if ( !success ) {
				glGetProgramInfoLog (shader, 1024, NULL, infoLog);
				cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
			}
		}
	}
public:
	unsigned int ID;
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	Shader (string vertexPath, string fragmentPath, string geometryPath = "") {
		// 1. retrieve the vertex/fragment source code from filePath
		string vertexCode;
		string fragmentCode;
		string geometryCode;

		ifstream vShaderFile;
		ifstream fShaderFile;
		ifstream gShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
		gShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
		try {
			// open files
			vShaderFile.open (vertexPath.c_str());
			fShaderFile.open (fragmentPath.c_str());
			stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf ();
			fShaderStream << fShaderFile.rdbuf ();
			// close file handlers
			vShaderFile.close ();
			fShaderFile.close ();
			// convert stream into string
			vertexCode = vShaderStream.str ();
			fragmentCode = fShaderStream.str ();

			// if geometry shader path is present, also load a geometry shader
			if ( geometryPath != "" ) {
				gShaderFile.open (geometryPath);
				stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf ();
				gShaderFile.close ();
				geometryCode = gShaderStream.str ();
			}
		} catch ( ifstream::failure &e ) {
			cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
		}

		const char *vShaderCode = vertexCode.c_str ();
		const char *fShaderCode = fragmentCode.c_str ();
		// 2. compile shaders
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader (GL_VERTEX_SHADER);
		glShaderSource (vertex, 1, &vShaderCode, NULL);
		glCompileShader (vertex);
		checkCompileErrors (vertex, "VERTEX", vertexPath);
		// fragment Shader
		fragment = glCreateShader (GL_FRAGMENT_SHADER);
		glShaderSource (fragment, 1, &fShaderCode, NULL);
		glCompileShader (fragment);
		checkCompileErrors (fragment, "FRAGMENT");

		unsigned int geometry;
		if ( geometryPath != "" ) {
			const char *gShaderCode = geometryCode.c_str ();
			geometry = glCreateShader (GL_GEOMETRY_SHADER);
			glShaderSource (geometry, 1, &gShaderCode, NULL);
			glCompileShader (geometry);
			checkCompileErrors (geometry, "GEOMETRY");
		}

		// shader Program
		ID = glCreateProgram ();
		glAttachShader (ID, vertex);
		glAttachShader (ID, fragment);
		if ( geometryPath != "" )
			glAttachShader (ID, geometry);
		glLinkProgram (ID);
		checkCompileErrors (ID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader (vertex);
		glDeleteShader (fragment);
		if ( geometryPath != "" )
			glDeleteShader (geometry);
	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use () {
		glUseProgram (ID);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool (const string &name, bool value) const {
		glUniform1i (glGetUniformLocation (ID, name.c_str ()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt (const string &name, int value) const {
		glUniform1i (glGetUniformLocation (ID, name.c_str ()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat (const string &name, float value) const {
		glUniform1f (glGetUniformLocation (ID, name.c_str ()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2 (const std::string &name, const glm::vec2 &value) const {
		glUniform2fv (glGetUniformLocation (ID, name.c_str ()), 1, &value[0]);
	}
	void setVec2 (const std::string &name, float x, float y) const {
		glUniform2f (glGetUniformLocation (ID, name.c_str ()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3 (const std::string &name, const glm::vec3 &value) const {
		glUniform3fv (glGetUniformLocation (ID, name.c_str ()), 1, &value[0]);
	}
	void setVec3 (const std::string &name, float x, float y, float z) const {
		glUniform3f (glGetUniformLocation (ID, name.c_str ()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4 (const std::string &name, const glm::vec4 &value) const {
		glUniform4fv (glGetUniformLocation (ID, name.c_str ()), 1, &value[0]);
	}
	void setVec4 (const std::string &name, float x, float y, float z, float w) const {
		glUniform4f (glGetUniformLocation (ID, name.c_str ()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2 (const std::string &name, const glm::mat2 &mat) const {
		glUniformMatrix2fv (glGetUniformLocation (ID, name.c_str ()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3 (const std::string &name, const glm::mat3 &mat) const {
		glUniformMatrix3fv (glGetUniformLocation (ID, name.c_str ()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4 (const std::string &name, const glm::mat4 &mat) const {
		glUniformMatrix4fv (glGetUniformLocation (ID, name.c_str ()), 1, GL_FALSE, &mat[0][0]);
	}
};
#endif

