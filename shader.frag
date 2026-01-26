//shader_frag
#version 330

// ============================================================
// GLOBAL VARIABLES
// ============================================================
#define PI 3.14159265359

//uniforms cam-mat
uniform vec3 u_color;
uniform vec3 u_cam_pos;		

//uniforms text pbr
uniform sampler2D u_texture;
uniform sampler2D u_texture_normal;
uniform sampler2D u_texture_rough;
uniform sampler2D u_texture_ao;

//uniforms properties 
uniform float u_metallic; //empty
uniform float u_reflectance; 

//lights
uniform vec3 u_light_dir;	//light position 01
uniform vec3 u_light_dir2;  //light position 02
uniform vec3 u_light_dir3;	//light position 03
uniform vec3 u_light_dir4;	//light position 04
uniform vec3 u_ambient;		//ambient 
uniform vec3 u_diffuse;		//prop light1
uniform vec3 u_diffuse2;	//prop light2
uniform vec3 u_diffuse3;	//prop light3
uniform vec3 u_diffuse4;	//prop light4
uniform vec3 u_specular;	//blinn-phong

//shadow
uniform sampler2D u_shadowMap;

//inputs
in vec2 v_uv;
in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_color;
in vec4 v_lightSpacePos;	//shadow


//outputs
out vec4 fragColor;

// ============================================================
// FUNCTIONS
// ============================================================

//Tangent bitangent normal calculus
//Tangent space normals to world space

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{	// get edge vectors of the pixel triangle

	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the linear system
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
	return mat3(T * invmax, B * invmax, N);
}
// assume N, the interpolated vertex normal and
// V, the view vector (vertex to eye)

vec3 perturbNormal(vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel)
{
	normal_pixel = normal_pixel * 2.0 - 1.0;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}

//--------------------------------------------------
//PBR method Cook-Torrance calculus
//--------------------------------------------------

//fresnelSchlick - angle based aproximation

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Trowbridge Reitz / GGX - Checks how many microfacets reflect  light taking into account camera and light direction

float D_GGX(float NoH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float b = (NoH2 * (alpha2 - 1.0) + 1.0);
    return alpha2 / (PI * b * b);  
}

// Checks occlusion from camera perspective  (%light received)

float G1_GGX_Schlick(float NoV, float roughness) {
    float alpha = roughness * roughness;  
    float k = alpha / 2.0;                
    return max(NoV, 0.001) / (NoV * (1.0 - k) + k);
}

// Checks occlusion from camera and light perspectives (%light received)

float G_Smith(float NoV, float NoL, float roughness) {  
    return G1_GGX_Schlick(NoL, roughness) * G1_GGX_Schlick(NoV, roughness); 
}

// Torrance Cook Microfacet calculus complete

vec3 brdfMicrofacet(vec3 L, vec3 V, vec3 N, 
                    float metallic, float roughness, 
                    vec3 baseColor, float reflectance) {
    
    vec3 H = normalize(V + L); //Halfway vector
    
	//angles
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);
    
	//fresnel 0 (metal)
    vec3 f0 = vec3(0.16 * (reflectance * reflectance));
    f0 = mix(f0, baseColor, metallic);
    
	//specular
    vec3 F = fresnelSchlick(VoH, f0);  
    float D = D_GGX(NoH, roughness);
    float G = G_Smith(NoV, NoL, roughness);
    
    vec3 spec = (F * D * G) / (4.0 * max(NoV, 0.001) * max(NoL, 0.001));
    
	//diffuse
    vec3 kD = vec3(1.0) - F; 
    kD *= (1.0 - metallic);   
    vec3 diff = kD * baseColor / PI;
    
	//sum
    return (diff + spec) * NoL;
}

float ShadowPCF(vec4 lightSpacePos) //softShadows
{
    vec3 projCoords = lightSpacePos.xyz / max(lightSpacePos.w, 0.0001);
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = 0.003;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap, 0));

    for (int x = -2; x <= 2; ++x)
    for (int y = -2; y <= 2; ++y)
    {
        float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
    }
    shadow /= 25.0;

    return shadow; // 0 = no sombra, 1 = totalmente en sombra
}


//------------------------------------------------------------

void main(void)
{
	// We're just going to paint the interpolated colour from the vertex shader

	vec4 tex = texture(u_texture, v_uv);
	vec3 texture_color = tex.rgb;
	float alpha = tex.a;


	//Textures

	//normal
	vec3 texture_normal = texture(u_texture_normal,v_uv).xyz;
	vec3 N = normalize(v_normal);

	//originalnormal
	vec3 N_orig = N;

	//call the function to modify the normal
	N = perturbNormal(N,v_vertex,v_uv,texture_normal);

	//mix the original normal with the new normal
	N  = mix(N_orig, N, 1.0f);
	vec3 E = normalize(u_cam_pos - v_vertex);

	//roughness
	float roughness = texture(u_texture_rough, v_uv).r;

	//ao
	float ao = texture(u_texture_ao, v_uv).r;

	//metallic
	float metallic = u_metallic; //no hay mapa
	
	/*--------------------------------------------------
	Blinn-phong method

	//Calc de luces

	//Light01
	vec3 L1 = normalize(u_light_dir - v_vertex);
	vec3 H1 = normalize(L1 + E);
	float NdotL1 = max(dot(N, L1), 0.0);
	float NdotH1 = max(dot(N, H1), 0.0);

	//vec3 diffuse_color1 = texture_color * NdotL1 * u_diffuse;
	//vec3 spec_color1 = texture_color * pow(NdotH1, shininess) * u_specular;

	//Light2
	vec3 L2 = normalize(u_light_dir2 - v_vertex);
	vec3 H2 = normalize(L2 + E);
	float NdotL2 = max(dot(N, L2), 0.0);
	float NdotH2 = max(dot(N, H2), 0.0);
	
	vec3 diffuse_color2 = texture_color * NdotL2 * u_diffuse2;
	vec3 spec_color2 = texture_color * pow(NdotH2, shininess) * u_specular;

	vec3 ambient_color = texture_color * u_ambient * ao;

	vec3 final_color = ambient_color + diffuse_color1 + spec_color1 + diffuse_color2 + spec_color2;
	*/

	//----------------------------------------------
	//PBR method Cook-Torrance
	//----------------------------------------------

	//Light1
	vec3 L1 = normalize(u_light_dir - v_vertex);
	vec3 pbr_light1 = brdfMicrofacet(L1, E, N, metallic, roughness, texture_color, u_reflectance);
	pbr_light1 *= u_diffuse;
	
	//shadow 
	float shadow = ShadowPCF(v_lightSpacePos);
	pbr_light1 *= (1.0 - shadow);

	// Light2
	vec3 L2 = normalize(u_light_dir2 - v_vertex);
	vec3 pbr_light2 = brdfMicrofacet(L2, E, N, metallic, roughness, texture_color, u_reflectance);
	pbr_light2 *= u_diffuse2;

	// Light3
	vec3 L3 = normalize(u_light_dir3 - v_vertex);
	vec3 pbr_light3 = brdfMicrofacet(L3, E, N, metallic, roughness, texture_color, u_reflectance);
	pbr_light3 *= u_diffuse3;

	//Light4
	vec3 L4 = normalize(u_light_dir4 - v_vertex);
	vec3 pbr_light4 = brdfMicrofacet(L4, E, N, metallic, roughness, texture_color, u_reflectance);
	pbr_light4 *= u_diffuse4;

	//final color
	vec3 ambient = texture_color * u_ambient * ao;
	vec3 final_color = ambient + pbr_light1 + pbr_light2 + pbr_light3 + pbr_light4;
	fragColor = vec4(final_color, alpha);
}