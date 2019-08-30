#version 150

// Each version of OpenGL has its own version of the shader language with availability of a certain feature set and we will be using GLSL 1.50. This version number
// may seem a bit off when we're using OpenGL 3.2, but that's because shaders were only introduced in OpenGL 2.0 as GLSL 1.10. Starting from OpenGL 3.3, this problem
// was solved and the GLSL version is the same as the OpenGL version

// It was expressed that some drivers required this next line to function properly
precision highp float;

in vec4 ex_Color;
in vec2 tex_Coords;
in vec4 normals_transformed;

uniform sampler2D texSampler;
uniform bool useTexture;

//out vec4 fragColor;

void main(void) {
    //gl_FragColor = vec4(1.0,0.5,0.5,1.0);
	//vec2 dummy = tex_Coords;
	//gl_FragColor = vec4(tex_Coords.x,ex_Color.gb,1.0);
	//gl_FragColor = normals_transformed;

	if (useTexture) {
		gl_FragColor = texture(texSampler, tex_Coords);
	} else {
		gl_FragColor = normals_transformed;
	}

	//gl_FragColor = ex_Color;
}