#version 330

uniform vec3 u_color;
out vec4 fragColor;

in vec3 v_color;
uniform sampler2D u_texture;

uniform vec3 u_light_dir;

uniform vec3 u_cam_pos;

uniform vec3 u_ambient;
uniform vec3 u_diffuse;
uniform vec3 u_specular;
uniform float u_shininess;

in vec2 v_uv;

in vec3 v_normal;
in vec3 v_vertex;

uniform vec3 final_color;

void main(void)
{
	// We're just going to paint the interpolated colour from the vertex shader
	vec3 texture_color = texture( u_texture, v_uv).xyz;

	//fragColor = vec4(texture_color.xyz, 1.0);	

	vec3 N = normalize(v_normal);
	//fragColor = vec4(N, 1.0);

	vec3 L = normalize(u_light_dir - v_vertex);
	float NdotL = max(dot(N ,L), 0.0);


	vec3 R = normalize(-reflect(L, N) );
	vec3 E = normalize(u_cam_pos - v_vertex);
	float RdotE = max( dot(R,E) , 0.0 );

	vec3 ambient_color = texture_color * u_ambient;
	vec3 diffuse_color = texture_color * NdotL * u_diffuse;
	vec3 spec_color = texture_color * pow ( RdotE , u_shininess);

	vec3 final_color = ambient_color + diffuse_color + spec_color;
	//vec3 final_color = texture_color.xyz * NdotL;
	fragColor = vec4(final_color, 1.0);

	//vec3 H = ( L + E ) / abs(L + E);
	//float BdotN = max (dot(H,N) 0.0);
	//vec3 blinn = 

}