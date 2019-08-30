#version 150

// Each version of OpenGL has its own version of the shader language with availability of a certain feature set and we will be using GLSL 1.50. This version number
// may seem a bit off when we're using OpenGL 3.2, but that's because shaders were only introduced in OpenGL 2.0 as GLSL 1.10. Starting from OpenGL 3.3, this problem
// was solved and the GLSL version is the same as the OpenGL version

// in_Position was bound to attribute index 0, in_Color was bound to attribute index 1 and in_Texcoords was bound to attribute index 2
in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_Texcoord;
 
uniform mat4 PVMmatrix;
uniform mat4 Mmatrix;

// We output the ex_Color and tex_Coords variables to the next shader in the chain
out vec4 ex_Color;
out vec4 normals_transformed;
out vec2 tex_Coords;

void main(void) {

    gl_Position = PVMmatrix * vec4(in_Position, 1.0f);
 
	normals_transformed = transpose(inverse(Mmatrix)) * vec4(in_Normal,1.0);

    // Pass the color and texture coordinates on to the fragment shader
    ex_Color = vec4(in_Normal,1.0);
	tex_Coords = in_Texcoord;
}