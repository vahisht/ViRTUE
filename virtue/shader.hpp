#ifndef _ViR2_SHADER
#define _ViR2_SHADER


#include <gl/glew.h>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

namespace ViR2 {

    class Shader {
        public:
            Shader();
            Shader(std::string vertexShader, std::string fragmentShader);
            ~Shader();

			bool init();
            bool useProgram();
			GLuint getShader() { return program; };

            bool setMVPmatrix(const glm::mat4);
            bool setModelMatrix(const glm::mat4);
            bool setViewMatrix(const glm::mat4);
            bool setProjectionMatrix(const glm::mat4);

			GLint getPosLocation() { return posLocation; }
			GLint getNormalLocation() { return normalLocation; }
			GLint getTexCoordLocation() { return texCoordLocation; }
			GLint getTexSamplerLocation() { return texSamplerLocation; }


			GLuint getUseTextureLocation() { return useTextureLocation; }
			GLuint getPVMmatrixLocation() { return PVMmatrixLocation; }
			GLuint getMmatrixLocation() { return MmatrixLocation; }
			GLuint getVmatrixLocation() { return VmatrixLocation; }

			GLuint getUniformLocation(std::string name) { return glGetUniformLocation(program, name.c_str());}

        private:

            GLuint program;

            GLuint vertexShader;
            GLuint fragmentShader;
            
            std::string vertexShaderFile;
            std::string fragmentShaderFile;

			GLint posLocation;
			GLint normalLocation;
			GLint texCoordLocation;

			GLint texSamplerLocation;

			GLuint useTextureLocation;
			GLuint PVMmatrixLocation;
			GLuint VmatrixLocation;
			GLuint MmatrixLocation;
	};

};

#endif

