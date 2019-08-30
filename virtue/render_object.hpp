#ifndef _ViR2_RENDER_OBJECT
#define _ViR2_RENDER_OBJECT


#include <gl/glew.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <cstddef>
#include <shader.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <opencv2/opencv.hpp>
#include <openvr.h>

namespace ViR2 {

	typedef struct MeshGeometry {
		GLuint        vertexBufferObject;   
		GLuint        elementBufferObject;  
		GLuint        vertexArrayObject;    
		unsigned int  numTriangles;         

		glm::vec3     ambient;
		glm::vec3     diffuse;
		glm::vec3     specular;
		float         shininess;

		GLuint        texture;
		bool		  hasTexture;

		bool un_short;

		MeshGeometry() {}
		MeshGeometry(std::string fileName, Shader* _shader);
		MeshGeometry(const float coords[] , unsigned long long size, Shader* _shader);
		MeshGeometry(const vr::RenderModel_t* rm_model, const vr::RenderModel_TextureMap_t* rm_texture, Shader* _shader);

	} MeshGeometry;

    class RenderObject {
        public:
			RenderObject() {}
			RenderObject(MeshGeometry* _model, Shader* _shader);
			~RenderObject() {}

			virtual void draw(glm::mat4 P, glm::mat4 V, glm::mat4 M);
			//virtual void drawIndexed(glm::mat4 P, glm::mat4 V, glm::mat4 M);
			bool move(glm::vec3 movement);
			bool rotate(float angle);

        protected:
			Shader* shader = NULL;
			MeshGeometry* model = NULL;

			glm::vec3 position = glm::vec3(0.0f);
			float rotation = 0.0f;
            
	};

	class SkyboxObject : public RenderObject {
	public:
		SkyboxObject() {}
		SkyboxObject(MeshGeometry* _model, Shader* _shader, std::string texture_prefix);
		~SkyboxObject() {}

		void draw(glm::mat4 P, glm::mat4 V, glm::mat4 M);

	protected:
		GLuint textureID;
	};

	class FullScreenQuadObject : public RenderObject {
	public:
		FullScreenQuadObject() {}
		FullScreenQuadObject(MeshGeometry* _model, Shader* _shader, GLuint* _textureID) : RenderObject(_model, _shader), textureID(_textureID) {  };
		~FullScreenQuadObject() {}

		void draw(glm::mat4 P, glm::mat4 V, glm::mat4 M);

		protected:
			GLuint* textureID = NULL;
	};

};

#endif

