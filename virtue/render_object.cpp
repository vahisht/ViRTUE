#include <render_object.hpp>


ViR2::MeshGeometry::MeshGeometry(std::string fileName, Shader* _shader) {

	// ASSIMP MODEL LOADING
	Assimp::Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);
	const aiScene * scn = importer.ReadFile(fileName.c_str(), 0 | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	if (scn == NULL) {
		
		std::cout << "Unable to init model " << fileName << std::endl;

		return;
	}
	if (scn->mNumMeshes != 1) {
		std::cout << "Only one mesh in obj allowed" << std::endl;
		return;
	}
	const aiMesh * mesh = scn->mMeshes[0];

	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float)*mesh->mNumVertices, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float)*mesh->mNumVertices, mesh->mVertices);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float)*mesh->mNumVertices, 3 * sizeof(float)*mesh->mNumVertices, mesh->mNormals);

	float *textureCoords = new float[2 * mesh->mNumVertices];
	float *currentTextureCoord = textureCoords;


	aiVector3D vect;

	if (mesh->HasTextureCoords(0)) {

		for (unsigned int idx = 0; idx < mesh->mNumVertices; idx++) {
			vect = (mesh->mTextureCoords[0])[idx];
			*currentTextureCoord++ = vect.x;
			*currentTextureCoord++ = vect.y;
		}
	}


	glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float)*mesh->mNumVertices, 2 * sizeof(float)*mesh->mNumVertices, textureCoords);

	unsigned int *indices = new unsigned int[mesh->mNumFaces * 3];
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
		indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
		indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
		indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
	}
	glGenBuffers(1, &elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

	const aiMaterial *mat = scn->mMaterials[mesh->mMaterialIndex];
	aiColor3D color;
	aiString name;

	mat->Get(AI_MATKEY_NAME, name);

	mat->Get<aiColor3D>(AI_MATKEY_COLOR_DIFFUSE, color);
	diffuse = glm::vec3(color.r, color.g, color.b);

	//std::cout << color.r << " " << color.g << " " << color.b << std::endl;

	mat->Get<aiColor3D>(AI_MATKEY_COLOR_AMBIENT, color);
	ambient = glm::vec3(color.r, color.g, color.b);

	//std::cout << color.r << " " << color.g << " " << color.b << std::endl;

	mat->Get<aiColor3D>(AI_MATKEY_COLOR_SPECULAR, color);
	specular = glm::vec3(color.r, color.g, color.b);

	//std::cout << color.r << " " << color.g << " " << color.b << std::endl;

	float shininess_temp;

	mat->Get<float>(AI_MATKEY_SHININESS, shininess_temp);
	shininess = shininess_temp;

	
	texture = 0;
	/*
	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
		mat->Get<aiString>(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), name);
		std::string textureName = name.data;

		size_t found = fileName.find_last_of("/\\");
		if (found != std::string::npos) {
			textureName.insert(0, fileName.substr(0, found + 1));
		}

		std::cout << "Loading texture: " << textureName << std::endl;
		texture = pgr::createTexture(textureName);
	}
	else {
		std::cout << "No texture image being imported " << std::endl;
	}
	*/
	hasTexture = false;

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

	glEnableVertexAttribArray(_shader->getPosLocation());
	glVertexAttribPointer(_shader->getPosLocation(), 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	if (_shader->getNormalLocation() >= 0) {
		glEnableVertexAttribArray(_shader->getNormalLocation());
		glVertexAttribPointer(_shader->getNormalLocation(), 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * mesh->mNumVertices));
	}

	if (_shader->getTexCoordLocation() >= 0) {
		glEnableVertexAttribArray(_shader->getTexCoordLocation());
		glVertexAttribPointer(_shader->getTexCoordLocation(), 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(float) * mesh->mNumVertices));
	}

	glBindVertexArray(0);


	numTriangles = mesh->mNumFaces;

	un_short = false;

	std::cout << "Everything seems ok" << std::endl;
}

ViR2::MeshGeometry::MeshGeometry(const float coords[], unsigned long long size, Shader* _shader) {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	/*for (size_t i = 0; i < 16; i++)
	{
		std::cout << coords[i] << " ";
	}
	std::cout << std::endl;*/
	//std::cout << sizeof(coords) << " vs " << size << std::endl;

	glGenBuffers(1, &vertexBufferObject); 
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, size, coords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(_shader->getPosLocation());
	glVertexAttribPointer(_shader->getPosLocation(), 2, GL_FLOAT, GL_FALSE, 0, 0);

	un_short = false;

	glBindVertexArray(0);
	glUseProgram(0);
}

ViR2::MeshGeometry::MeshGeometry(const vr::RenderModel_t* rm_model, const vr::RenderModel_TextureMap_t* rm_texture, Shader* _shader) {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * rm_model->unVertexCount, rm_model->rVertexData, GL_STATIC_DRAW);

	numTriangles = rm_model->unTriangleCount;

	glEnableVertexAttribArray(_shader->getPosLocation());
	glVertexAttribPointer(_shader->getPosLocation(),		3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(_shader->getNormalLocation());
	glVertexAttribPointer(_shader->getNormalLocation(),		3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(_shader->getTexCoordLocation());
	glVertexAttribPointer(_shader->getTexCoordLocation(),	2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// Create and populate the index buffer
	glGenBuffers(1, &elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * rm_model->unTriangleCount * 3, rm_model->rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// create and populate the texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	hasTexture = true;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rm_texture->unWidth, rm_texture->unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, rm_texture->rubTextureMapData);

	// If this renders black ask McJohn what's wrong. <-- A tohle je jako co??
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
	/*
	std::ofstream myfile;
	myfile.open("1.obj");
	for (size_t i = 0; i < rm_model->unVertexCount; i++)
	{
		myfile << "v " << rm_model->rVertexData[i].vPosition.v[0] << " " << rm_model->rVertexData[i].vPosition.v[1] << " " << rm_model->rVertexData[i].vPosition.v[2] << std::endl;
	}
	for (size_t i = 0; i < rm_model->unTriangleCount; i++)
	{
		myfile << "f " << rm_model->rIndexData[3*i]+1 << " " << rm_model->rIndexData[3 * i + 1]+1 << " " << rm_model->rIndexData[3 * i + 2]+1 << std::endl;
	}
	myfile.close();*/

	un_short = true;
}

ViR2::RenderObject::RenderObject(MeshGeometry* _model, Shader* _shader) {
	model = _model;
	shader = _shader;
}

ViR2::SkyboxObject::SkyboxObject(MeshGeometry* _model, Shader* _shader, std::string texture_prefix) : RenderObject(_model, _shader) {
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	//const char * suffixes[] = { "rt", "lf", "up", "dn", "bk", "ft" };
	const char * suffixes[] = { "ft", "bk", "up", "dn", "rt", "lf" };
	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	for (int i = 0; i < 6; i++) {
		//std::string texName = std::string(SKYBOX_CUBE_TEXTURE_FILE_PREFIX) + "_" + suffixes[i] + ".jpg";
		std::string texName = std::string(texture_prefix) + "_" + suffixes[i] + ".png";
		std::cout << "Loading cube map texture: " << texName << std::endl;
		cv::Mat skybox_image = cv::imread(texName);

		if (skybox_image.empty()) { // Image not found or otherwise not loaded
			std::cout << "ERROR: SKYBOX LOADING FAILED" << std::endl;
		}
		else {
			//cv::flip(skybox_image, skybox_image, 0); // OpenCV stores top to bottom, but we need the image bottom to top for OpenGL
			cv::cvtColor(skybox_image, skybox_image, cv::COLOR_BGR2RGB); // OpenCV uses BGR format, need to convert it to RGB for OpenGL
			glTexImage2D(targets[i], 0, GL_RGB, skybox_image.cols, skybox_image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, skybox_image.ptr());
			//background_image.setUniformSampler2D("texture", image.cols, image.rows, image.ptr());
		}

	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

};

bool ViR2::RenderObject::rotate(float angle) {
	rotation += angle;

	return true;
}

bool ViR2::RenderObject::move(glm::vec3 movement) {
	position += movement;

	return true;
}

void ViR2::RenderObject::draw(glm::mat4 P, glm::mat4 V, glm::mat4 M ) {
	shader->useProgram();

	glm::mat4 modelMat = glm::rotate(glm::mat4(1.0f), rotation , glm::vec3(0.0f, 1.0f, 0.0f) );
	modelMat = glm::translate(modelMat, position);

	glm::mat4 PVM = P * V * M * modelMat;
	glUniformMatrix4fv(shader->getPVMmatrixLocation(), 1, GL_FALSE, glm::value_ptr(PVM));
	glUniformMatrix4fv(shader->getMmatrixLocation(), 1, GL_FALSE, glm::value_ptr(M * modelMat));
	//glUniformMatrix4fv(testingShader.getVmatrixLocation(), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	if ( model->hasTexture ) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model->texture );
		glUniform1i(shader->getTexSamplerLocation(), 0);
		glUniform1i(shader->getUseTextureLocation(), true);
	} else {
		glUniform1i(shader->getUseTextureLocation(), false);
	}


	glBindVertexArray(model->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, model->numTriangles * 3, model->un_short ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//std::cout << model->numTriangles << std::endl;

	glUseProgram(0);
}

/*void ViR2::RenderObject::drawIndexed(glm::mat4 P, glm::mat4 V, glm::mat4 M) {
	shader->useProgram();

	glm::mat4 PVM = P * V * M;
	glUniformMatrix4fv(shader->getPVMmatrixLocation(), 1, GL_FALSE, glm::value_ptr(PVM));
	//glUniformMatrix4fv(testingShader.MmatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	//glUniformMatrix4fv(testingShader.getVmatrixLocation(), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glBindVertexArray(model->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, model->numTriangles * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}*/

void ViR2::FullScreenQuadObject::draw(glm::mat4 P, glm::mat4 V, glm::mat4 M) {
	shader->useProgram();


	glBindVertexArray(model->vertexArrayObject);

	// Texture handling
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *(textureID));
	glUniform1i(shader->getTexSamplerLocation(), 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);
}

void ViR2::SkyboxObject::draw(glm::mat4 P, glm::mat4 V, glm::mat4 M) {
	shader->useProgram();

	glBindVertexArray(model->vertexArrayObject);

	glm::mat4 viewRotation = V;
	viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 inversePVmatrix = glm::inverse(P * viewRotation);

	glUniformMatrix4fv(shader->getPVMmatrixLocation(), 1, GL_FALSE, glm::value_ptr(inversePVmatrix));

	// Texture handling
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glUniform1i(shader->getTexSamplerLocation(), 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);
}