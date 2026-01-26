//shader_vert
#version 330

//inputs
in vec3 a_vertex;
in vec3 a_color;
in vec2 a_uv;
in vec3 a_normal;

//uniforms
uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_lightSpace;		//shadow

//outputs
out vec2 v_uv;
out vec3 v_color;
out vec3 v_normal;
out vec3 v_vertex;
out vec3 v_fragPos;
out vec4 v_lightSpacePos;	//shadow



//------------------------------------------------------------

void main()
{

	// position of the vertex, original position
	gl_Position = u_projection * u_view * u_model * vec4( a_vertex , 1.0 );

	// pass the colour to the fragment shader
	v_color = a_color;

	// pass the uv coordinates to the fragment shader
	v_uv = a_uv;

	//world space
	v_vertex = (u_model * vec4(a_vertex, 1.0)).xyz;

	//shadow
	v_lightSpacePos = u_lightSpace * vec4(v_vertex, 1.0);

	//normal transform
	mat3 normalMatrix = transpose(inverse(mat3(u_model)));
	v_normal = normalize(normalMatrix * a_normal);
}
