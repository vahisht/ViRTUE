#version 150
#extension GL_ARB_explicit_attrib_location : enable

precision highp float;

in vec4 ex_Color;
in vec2 tex_Coords;
in vec4 normals_transformed;

uniform sampler2D texSampler;
uniform bool useTexture;
uniform float focalLength;
uniform float baseline;

layout(location = 0) out vec4 colorOut;
layout(location = 1) out vec4 disparityOut;

void main(void) {
    //gl_FragColor = vec4(1.0,0.5,0.5,1.0);
	//vec2 dummy = tex_Coords;
	//gl_FragColor = vec4(tex_Coords.x,ex_Color.gb,1.0);
	//gl_FragColor = normals_transformed;

	if (useTexture) {
		colorOut = texture(texSampler, tex_Coords);
	} else {
		colorOut = (normals_transformed / 2.0) + vec4(0.5, 0.5, 0.5, 0.5);
	}

	float f = 15.0f;
	float n = 0.1f;
	float depth = gl_FragCoord.z;
	float depth_linearized = (2.0 * n) / (f + n - depth * (f - n));
	//disparityOut = vec4(depth_linearized);
	//disparityOut = vec4((baseline*focalLength)/(depth));
	disparityOut = vec4((baseline*focalLength)/(depth_linearized));

	//gl_FragColor = ex_Color;
}