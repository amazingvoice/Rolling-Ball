/***************************
 * File: vshader42new.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vColor;
in  vec3 vNormal;
out vec4 color;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 pAmbientProduct, pDiffuseProduct, pSpecularProduct;

uniform mat4 model_view;
uniform mat4 projection;

uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;			// Must be in Eye Frame
uniform vec4 pLightPositionStart;	// Must be in Eye Frame
uniform vec4 pLightPositionEnd;		// Must be in Eye Frame

uniform float Shininess;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation
uniform float CutoffAngle;  
uniform float LightSource;
uniform float SpotExponent;

void main() 
{
	vec3 pos = (model_view * vPosition).xyz;
	vec3 L;
	if(LightPosition.w == 1.0) {
		// Transform vertex position into eye coordinates
		L = normalize( LightPosition.xyz - pos );
	}
	else {
		L = normalize( -LightPosition.xyz );
	}

    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E ); 

	vec3 N = normalize(Normal_Matrix * vNormal);
	if ( dot(N, E) < 0 ) N = -N;

	// Compute terms in the illumination equation

    vec4 ambient = AmbientProduct;

    float d = max( dot(L, N), 0.0 );  // AOI [0.0, 1.0]
    vec4  diffuse = d * DiffuseProduct;

    float s = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s * SpecularProduct;
    
    if( dot(L, N) < 0.0 ) 
		specular = vec4(0.0, 0.0, 0.0, 1.0);

    gl_Position = projection * model_view * vPosition;

	// output COLOR
    color = ambient + diffuse + specular;

	/* =================================== */
	/* ====== for spot/point light ======= */
	/* =================================== */
	
	/*--- To Do: Compute attenuation ---*/
	float dist = length( pLightPositionStart.xyz - pos );
	float attenuation = 1.0 / (ConstAtt + LinearAtt * dist + QuadAtt * dist * dist); 
	
	L = normalize( pLightPositionStart.xyz - pos );
	E = normalize( -pos );
    H = normalize( L + E ); 

	if(LightSource == 1) {  // spotlight
		vec3 coneDirection = normalize((pLightPositionEnd - pLightPositionStart).xyz);
		float spotCos = dot(L, -coneDirection);
		float angle = degrees(acos(dot(L, -coneDirection)));

		if(angle > CutoffAngle)
			attenuation = 0.0;
		else 
			attenuation *= pow(spotCos, SpotExponent);
	}

	ambient = pAmbientProduct;

    d = max( dot(L, N), 0.0 );  // AOI [0.0, 1.0]
    diffuse = d * pDiffuseProduct;

    s = pow( max(dot(N, H), 0.0), Shininess );
    specular = s * pSpecularProduct;
    
    if( dot(L, N) < 0.0 ) 
		specular = vec4(0.0, 0.0, 0.0, 1.0);

	color += (attenuation * (diffuse + specular) + ambient);
}