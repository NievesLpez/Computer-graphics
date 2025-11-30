#version 330

uniform vec3 u_color;
out vec4 fragColor;

in vec3 v_color;
uniform sampler2D u_texture;

uniform vec3 u_light_dir;	//light position
uniform vec3 u_cam_pos;		//camara position

uniform vec3 u_ambient;		//i_a
uniform vec3 u_diffuse;		//i_d
uniform vec3 u_specular;	//i_s
uniform float u_shininess;	//n

in vec2 v_uv;
in vec3 v_normal;
in vec3 v_vertex;

//uniform vec3 final_color;

void main(void)
{
	// We're just going to paint the interpolated colour from the vertex shader
	vec3 texture_color = texture( u_texture, v_uv).xyz;

	vec3 N = normalize(v_normal);
	vec3 L = normalize(u_light_dir - v_vertex);
	vec3 E = normalize(u_cam_pos - v_vertex);
	
	
	//vec3 R = normalize(-reflect(L, N) );
	//float RdotE = max( dot(R,E) , 0.0 );

	vec3 H = normalize( L + E );
	float NdotL = max(dot(N ,L), 0.0);
	float NdotH = max(dot(N, H), 0.0);

	vec3 ambient_color = texture_color * u_ambient;
	vec3 diffuse_color = texture_color * NdotL * u_diffuse;
	vec3 spec_color = texture_color * pow(NdotH , u_shininess) * u_specular;

	vec3 final_color = ambient_color + diffuse_color + spec_color;
	fragColor = vec4(final_color, 1.0);



}
