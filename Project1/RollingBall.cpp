#include "Angel-yjc.h"

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint program_light; /* light shader program object id */

GLuint shadow_buffer; /* vertex buffer object id for shadow */

GLuint ball_buffer;   /* vertex buffer object id for ball */
GLuint ball_buffer_filled;
GLuint ball_buffer_filled_smooth;

GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint floor_buffer_light; /* vertex buffer object id for floor with lighting*/

GLuint axis_buffer;		

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.1, zFar = 100.0;

mat4 accumulateRotation = Rotate(0.0, 1.0, 0.0, 0.0);
mat4 shadowMatrix = mat4(12, 0, 0, 0, 14, 0, 3, -1, 0, 0, 12, 0, 0, 0, 0, 12);

GLfloat angle = 0.0; // rotation angle in degrees
GLfloat delta = 2.0; // control rotate speed
GLuint path = 0;
GLfloat rotate_x = -3.0;
GLfloat rotate_y = 0.0;
GLfloat rotate_z = 2.0;
GLfloat unit_len = 2 * M_PI / 360.0;
GLfloat tx = 0;
GLfloat tz = 0;

vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

// keyboard control 
int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int animationStarted = 0; 

int shadow = 0;
int smooth_shading = 0;
int lighting = 0;
int lightSource = 1; // 1: spot light;  2: point light
int ballFlag = 0;   // 1: solid ball; 0: wireframe ball. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'

int ball_NumVertices; // depends on the sphere file
point3 ball_points[3500];
point4 ball_points_4[3500]; 
vec3   ball_normals[3500];
vec3   ball_normals_smooth[3500];
color3 ball_colors[3500];
color3 shadow_colors[3500];

const int floor_NumVertices = 6;		 //(1 face) * (2 triangles/face) * (3 vertices/triangle)
point3 floor_points[floor_NumVertices];
point4 floor_points_4[floor_NumVertices];  // positions for all floor vertices
color3 floor_colors[floor_NumVertices];	 // normals for all floor vertices
vec3   floor_normals[floor_NumVertices]; // normals for all floor vertices

point3 axis_points[6];
color3 axis_colors[6];

point4 vertices[6] = {
	point4( 0.0,  1.0,  0.0,  1.0),
	point4( 0.0,  0.0,  1.0,  1.0),
	point4( 1.0,  0.0,  0.0,  1.0),
	point4(-1.0,  0.0,  0.0,  1.0),
	point4( 0.0, -1.0,  0.0,  1.0),
	point4( 0.0,  0.0, -1.0,  1.0)
};

void floor()
{	
	// calculate the floor normal
	vec4 u = point4(5.0, 0.0, -4.0, 1.0) - point4(5.0, 0.0, 8.0, 1.0);
	vec4 v = point4(-5.0, 0.0, -4.0, 1.0) - point4(5.0, 0.0, 8.0, 1.0);

	vec3 floor_normal = normalize(cross(u, v));
	
	// generate 2 triangles: 6 vertices
	floor_colors[0] = color3(0.0, 1.0, 0.0);	floor_normals[0] = floor_normal; 
	floor_points[0] = point3(5.0, 0.0, 8.0);	floor_points_4[0] = point4(5.0, 0.0, 8.0, 1.0);

	floor_colors[1] = color3(0.0, 1.0, 0.0);	floor_normals[1] = floor_normal; 
	floor_points[1] = point3(5.0, 0.0, -4.0);	floor_points_4[1] = point4(5.0, 0.0, -4.0, 1.0);
	
	floor_colors[2] = color3(0.0, 1.0, 0.0);	floor_normals[2] = floor_normal; 
	floor_points[2] = point3(-5.0, 0.0, -4.0);	floor_points_4[2] = point4(-5.0, 0.0, -4.0, 1.0);

	floor_colors[3] = color3(0.0, 1.0, 0.0);	floor_normals[3] = floor_normal; 
	floor_points[3] = point3(-5.0, 0.0, -4.0);	floor_points_4[3] = point4(-5.0, 0.0, -4.0, 1.0);

	floor_colors[4] = color3(0.0, 1.0, 0.0);	floor_normals[4] = floor_normal; 
	floor_points[4] = point3(-5.0, 0.0, 8.0);	floor_points_4[4] = point4(-5.0, 0.0, 8.0, 1.0);

	floor_colors[5] = color3(0.0, 1.0, 0.0);	floor_normals[5] = floor_normal; 
	floor_points[5] = point3(5.0, 0.0, 8.0);	floor_points_4[5] = point4(5.0, 0.0, 8.0, 1.0);
}

void axes()
{
	// x
	axis_points[0] = point3(0.0, 0.0, 0.0); axis_colors[0] = color3(1.0, 0.0, 0.0);
	axis_points[1] = point3(1.0, 0.0, 0.0); axis_colors[1] = color3(1.0, 0.0, 0.0);
	// y
	axis_points[2] = point3(0.0, 0.0, 0.0); axis_colors[2] = color3(1.0, 0.0, 1.0);
	axis_points[3] = point3(0.0, 1.0, 0.0); axis_colors[3] = color3(1.0, 0.0, 1.0);
	// z
	axis_points[4] = point3(0.0, 0.0, 0.0); axis_colors[4] = color3(0.0, 0.0, 1.0);
	axis_points[5] = point3(0.0, 0.0, 1.0); axis_colors[5] = color3(0.0, 0.0, 1.0);
}

// OpenGL initialization
void init()
{
	// Create and initialize a vertex buffer object for SHADOW, to be used in display()
	glGenBuffers(1, &shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * ball_NumVertices + sizeof(color3) * ball_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * ball_NumVertices, ball_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * ball_NumVertices, sizeof(color3) * ball_NumVertices, shadow_colors);
	

	// Create and initialize a vertex buffer object for BALL, to be used in display()
	// without lighting
    glGenBuffers(1, &ball_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, ball_buffer);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point3) + sizeof(color3)) * ball_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * ball_NumVertices, ball_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * ball_NumVertices, sizeof(color3) * ball_NumVertices, ball_colors);
    // with lighting
	glGenBuffers(1, &ball_buffer_filled);
	glBindBuffer(GL_ARRAY_BUFFER, ball_buffer_filled);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) + sizeof(vec3)) * ball_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * ball_NumVertices, ball_points_4);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * ball_NumVertices, sizeof(vec3) * ball_NumVertices, ball_normals);

	glGenBuffers(1, &ball_buffer_filled_smooth);
	glBindBuffer(GL_ARRAY_BUFFER, ball_buffer_filled_smooth);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) + sizeof(vec3)) * ball_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * ball_NumVertices, ball_points_4);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * ball_NumVertices, sizeof(vec3) * ball_NumVertices, ball_normals_smooth);

	// Create and initialize a vertex buffer object for FLOOR, to be used in display()
	floor();
	// with lighting
    glGenBuffers(1, &floor_buffer_light);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer_light);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points_4) + sizeof(floor_normals), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points_4), floor_points_4);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points_4), sizeof(floor_normals), floor_normals);
	//without lighting
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point3) + sizeof(color3)) * floor_NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);

	// Create and initialize a vertex buffer object for 3 axes, to be used in display()
	axes();
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors), axis_colors);

	// Load shaders and create a shader program (to be used in display())
    program_light = InitShader("vshader42new.glsl", "fshader42.glsl");
	program = InitShader("vshader42.glsl", "fshader42.glsl");
    glEnable( GL_DEPTH_TEST );  // enable the z-buffer for depth test
    glClearColor(0.529, 0.807, 0.92, 0);
    glLineWidth(1.0);
}

void SetUp_Lighting_Uniform_Vars(mat4 mv)
{

	/*----- Shader Lighting Parameters -----*/
	color4 light_ambient(0.2, 0.2, 0.2, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);
	float const_att = 2.0;
	float linear_att = 0.01;
	float quad_att = 0.001;
	// In World frame.
	// Needs to transform it to Eye Frame before sending it to the shader(s).
	point4 light_position(-14.0, 12.0, -3.0, 1.0);

	color4 material_ambient(0.2, 0.2, 0.2, 1.0);
	color4 material_diffuse(0.0, 1.0, 0.0, 1.0);
	color4 material_specular(0.0, 0.0, 0.0, 1.0);
	float  material_shininess = 100.0;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program_light, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program_light, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program_light, "SpecularProduct"), 1, specular_product);

	// The Light Position in Eye Frame
	vec4 light_position_eyeFrame = mv * light_position;
	glUniform4fv(glGetUniformLocation(program_light, "LightPosition"), 1, light_position_eyeFrame);
	glUniform1f(glGetUniformLocation(program_light, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program_light, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program_light, "QuadAtt"), quad_att);
	glUniform1f(glGetUniformLocation(program_light, "Shininess"), material_shininess);
}

void SetUp_Lighting_Uniform_Vars_Ground(mat4 mv)
{
	/* ----- Directional Shader Lighting Parameters ----- */
	color4 light_ambient(1.0, 1.0, 1.0, 1.0);
	color4 light_diffuse(0.8, 0.8, 0.8, 1.0);
	color4 light_specular(0.2, 0.2, 0.2, 1.0);
	point4 light_position(0.1, 0.0, -1.0, 0.0); // direction
	
	/* ----- Positional Shader Lighting Parameters ----- */
	color4 pos_light_ambient(0.0, 0.0, 0.0, 1.0);
	color4 pos_light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 pos_light_specular(1.0, 1.0, 1.0, 1.0);

	point4 pos_light_position(-14.0, 12.0, -3.0, 1.0); // world coordination system
	point4 pos_light_endpoint(-6.0, 0.0, -4.5, 1.0);  // world coordination system

	float const_att = 2.0;
	float linear_att = 0.01;
	float quad_att = 0.001;

	float cutoff_angle = 20.0;
	float spot_exp = 15.0;

	/* ----- Material Parameters ----- */
	color4 material_ambient(0.2, 0.2, 0.2, 1.0);
	color4 material_diffuse(0.0, 1.0, 0.0, 1.0);
	color4 material_specular(0.0, 0.0, 0.0, 1.0);
	float  material_shininess = 0.0;


	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	color4 pos_ambient_product = pos_light_ambient * material_ambient;
	color4 pos_diffuse_product = pos_light_diffuse * material_diffuse;
	color4 pos_specular_product = pos_light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program_light, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program_light, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program_light, "SpecularProduct"), 1, specular_product);

	glUniform4fv(glGetUniformLocation(program_light, "pAmbientProduct"), 1, pos_ambient_product);
	glUniform4fv(glGetUniformLocation(program_light, "pDiffuseProduct"), 1, pos_diffuse_product);
	glUniform4fv(glGetUniformLocation(program_light, "pSpecularProduct"), 1, pos_specular_product);

	//vec4 light_position_eyeFrame = mv * light_position;
	glUniform4fv(glGetUniformLocation(program_light, "LightPosition"), 1, light_position);
	glUniform4fv(glGetUniformLocation(program_light, "pLightPositionStart"), 1, mv * pos_light_position);
	glUniform4fv(glGetUniformLocation(program_light, "pLightPositionEnd"), 1, mv * pos_light_endpoint);
	glUniform1f(glGetUniformLocation(program_light, "Shininess"), material_shininess);

	glUniform1f(glGetUniformLocation(program_light, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program_light, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program_light, "QuadAtt"), quad_att);
	glUniform1f(glGetUniformLocation(program_light, "CutoffAngle"), cutoff_angle);
	glUniform1f(glGetUniformLocation(program_light, "LightSource"), lightSource);
	glUniform1f(glGetUniformLocation(program_light, "SpotExponent"), spot_exp);
}

void SetUp_Lighting_Uniform_Vars_Ball_Filled(mat4 mv)
{

	/*----- Shader Lighting Parameters -----*/
	color4 light_ambient(1.0, 1.0, 1.0, 1.0);
	color4 light_diffuse(0.8, 0.8, 0.8, 1.0);
	color4 light_specular(0.2, 0.2, 0.2, 1.0);

	point4 light_position(0.1, 0.0, -1.0, 0.0);

	color4 pos_light_ambient(0.0, 0.0, 0.0, 1.0);
	color4 pos_light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 pos_light_specular(1.0, 1.0, 1.0, 1.0);

	color4 material_ambient(0.2, 0.2, 0.2, 1.0);
	color4 material_diffuse(1.0, 0.84, 0.0, 1.0);
	color4 material_specular(1.0, 0.84, 0.0, 1.0);
	float  material_shininess = 125.0;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	color4 pos_ambient_product = pos_light_ambient * material_ambient;
	color4 pos_diffuse_product = pos_light_diffuse * material_diffuse;
	color4 pos_specular_product = pos_light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program_light, "AmbientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program_light, "DiffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program_light, "SpecularProduct"), 1, specular_product);

	glUniform4fv(glGetUniformLocation(program_light, "pAmbientProduct"), 1, pos_ambient_product);
	glUniform4fv(glGetUniformLocation(program_light, "pDiffuseProduct"), 1, pos_diffuse_product);
	glUniform4fv(glGetUniformLocation(program_light, "pSpecularProduct"), 1, pos_specular_product);

	//vec4 light_position_eyeFrame = mv * light_position;
	glUniform4fv(glGetUniformLocation(program_light, "LightPosition"), 1, light_position);
	glUniform1f(glGetUniformLocation(program_light, "Shininess"), material_shininess);
}


// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer 
//   object "buffer" and has "num_vertices" vertices.
void drawObj(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point3) * num_vertices) );
	  // the offset is the (total) size of the previous vertex attribute array(s)
	
	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	   (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

void drawObj_light(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program_light, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program_light, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)


	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	   (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
}

void drawLine(GLuint buffer, int num_vertices)
{
	glLineWidth(2.0);
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, 
							BUFFER_OFFSET(sizeof(point3) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

  /* Draw a sequence of geometric objs (lines) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_LINES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
	glLineWidth(1.0);
}

//----------------------------------------------------------------------------
void display( void )
{
	GLuint  model_view;  // model-view matrix uniform shader variable location
	GLuint  projection;  // projection matrix uniform shader variable location

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4 at(0.0, 0.0, 0.0, 1.0);
    vec4 up(0.0, 1.0, 0.0, 0.0);
    mat4 mv = LookAt(eye, at, up);
	mat4 mv_ball;
	mat4 mv_shadow;

	/*	Decaling:
		1. Disable depth buffer updates
		2. Draw the base polygon
		3. Draw the decal polygons
		4. Disable color buffer updates
		5. Enable depth buffer updates
		6. Draw the base polygon
	*/

		/* ----------------------------------------------------- */
		/* ----- Set up the Mode-View matrix for the floor ----- */
		/* ----------------------------------------------------- */

	mat3 normal_matrix;
	mat4 p;

	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (lighting) {
		glUseProgram(program_light); // Use the shader program
		model_view = glGetUniformLocation(program_light, "model_view");
		projection = glGetUniformLocation(program_light, "projection");

		/*---  Set up and pass on Projection matrix to the shader ---*/
		p = Perspective(fovy, aspect, zNear, zFar);
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		SetUp_Lighting_Uniform_Vars_Ground(mv);
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		normal_matrix = NormalMatrix(mv, 1);
		glUniformMatrix3fv(glGetUniformLocation(program_light, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

		drawObj_light(floor_buffer_light, floor_NumVertices);  // draw the floor
	}
	else { // without lighting
		glUseProgram(program); // Use the shader program
		model_view = glGetUniformLocation(program, "model_view");
		projection = glGetUniformLocation(program, "projection");

		/*---  Set up and pass on Projection matrix to the shader ---*/
		p = Perspective(fovy, aspect, zNear, zFar);
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		drawObj(floor_buffer, floor_NumVertices);  // draw the floor
	}
	
		// ----------------------------------------------------- 
		// ---- Set Up the Model-View matrix for the SHADOW ---- 
		// ----------------------------------------------------- 

	glUseProgram(program); // Use the shader program
	model_view = glGetUniformLocation(program, "model_view");
	projection = glGetUniformLocation(program, "projection");

	/*---  Set up and pass on Projection matrix to the shader ---*/
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

	if (shadow && eye[1] > 0) {
		mv_shadow = mv * shadowMatrix * Translate(3.0 + tx, 1.0, 5.0 + tz) * Scale(1.0, 1.0, 1.0)
			* Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;

		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv_shadow); // GL_TRUE: matrix is row-major
		if (ballFlag == 1) // Filled ball
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe ball
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		drawObj(shadow_buffer, ball_NumVertices);  // draw the ball
	}
	
		// -----------------------------------------------------
		// ----- Set up the Mode-View matrix for the floor -----
		// -----------------------------------------------------
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	if (lighting) {
		glUseProgram(program_light); // Use the shader program
		model_view = glGetUniformLocation(program_light, "model_view");
		projection = glGetUniformLocation(program_light, "projection");

		/*---  Set up and pass on Projection matrix to the shader ---*/
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		SetUp_Lighting_Uniform_Vars_Ground(mv);
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		normal_matrix = NormalMatrix(mv, 1);
		glUniformMatrix3fv(glGetUniformLocation(program_light, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

		drawObj_light(floor_buffer_light, floor_NumVertices);  // draw the floor
	}
	else {
		glUseProgram(program); // Use the shader program
		model_view = glGetUniformLocation(program, "model_view");
		projection = glGetUniformLocation(program, "projection");

		/*---  Set up and pass on Projection matrix to the shader ---*/
		p = Perspective(fovy, aspect, zNear, zFar);
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		drawObj(floor_buffer, floor_NumVertices);  // draw the floor
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
		/* ----------------------------------------------------- */
		/* ----- Set Up the Model-View matrix for the BALL ----- */
		/* ----------------------------------------------------- */
	if (ballFlag == 0) { // Wireframe
		glUseProgram(program); // Use the shader program
		model_view = glGetUniformLocation(program, "model_view");
		projection = glGetUniformLocation(program, "projection");

		/*---  Set up and pass on Projection matrix to the shader ---*/
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		// The set-up below gives a new scene (scene 2), using Correct LookAt().

		mv_ball = mv * Translate(3.0 + tx, 1.0, 5.0 + tz) * Scale(1.0, 1.0, 1.0)
			* Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;

		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv_ball); // GL_TRUE: matrix is row-major
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// normal_matrix = NormalMatrix(model_view, 1);
		// glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

		drawObj(ball_buffer, ball_NumVertices);  // draw the ball
	}
	else { // filled
		if (lighting) {
			glUseProgram(program_light); // Use the shader program
			model_view = glGetUniformLocation(program_light, "model_view");
			projection = glGetUniformLocation(program_light, "projection");

			/*---  Set up and pass on Projection matrix to the shader ---*/
			glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

			// Set up lighting & shading variables
			SetUp_Lighting_Uniform_Vars_Ball_Filled(mv);

			// The set-up below gives a new scene (scene 2), using Correct LookAt().
			mv_ball = mv * Translate(3.0 + tx, 1.0, 5.0 + tz) * Scale(1.0, 1.0, 1.0)
				* Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;

			glUniformMatrix4fv(model_view, 1, GL_TRUE, mv_ball); // GL_TRUE: matrix is row-major

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			normal_matrix = NormalMatrix(mv_ball, 1);
			glUniformMatrix3fv(glGetUniformLocation(program_light, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);

			// draw the ball
			if (!smooth_shading)
				drawObj_light(ball_buffer_filled, ball_NumVertices);
			else
				drawObj_light(ball_buffer_filled_smooth, ball_NumVertices);
		}
		else { // without lighting
			glUseProgram(program); // Use the shader program
			model_view = glGetUniformLocation(program, "model_view");
			projection = glGetUniformLocation(program, "projection");

			/*---  Set up and pass on Projection matrix to the shader ---*/
			glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

			// The set-up below gives a new scene (scene 2), using Correct LookAt().
			mv_ball = mv * Translate(3.0 + tx, 1.0, 5.0 + tz) * Scale(1.0, 1.0, 1.0)
				* Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;

			glUniformMatrix4fv(model_view, 1, GL_TRUE, mv_ball); // GL_TRUE: matrix is row-major

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			drawObj(ball_buffer, ball_NumVertices);
		}
	}

	/* ---------------------------------------------------- */
	/* ----- Set up the Mode-View matrix for xyz axes ----- */
	/* ---------------------------------------------------- */

	glUseProgram(program); // Use the shader program
	model_view = glGetUniformLocation(program, "model_view");
	projection = glGetUniformLocation(program, "projection");

	/*---  Set up and pass on Projection matrix to the shader ---*/
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

	// The set-up below gives a new scene (scene 2), using Correct LookAt() function
	mv = LookAt(eye, at, up) * Scale(10.0, 10.0, 10.0);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

	drawLine(axis_buffer, 6);

	
	glutSwapBuffers();
}
//----------------------------------------------------------------------------
void idle(void)
{
	if (path == 0) // Path A -> B
	{
		if (tx <= -5.0)
		{
			accumulateRotation = Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;
			path = (path + 1) % 3;
			angle = 0.0;
			rotate_x = -3.0;
			rotate_y = 0.0;
			rotate_z = -8.0;
		}	
		else
		{
			angle += delta;    // rotate the ball
			if (angle > 360.0)
				angle -= 360.0;

			tx -= delta * unit_len * 2 / sqrt(13);
			tz -= delta * unit_len * 3 / sqrt(13);
		}
	}
	else if (path == 1) // Path B -> C
	{
		if (tx >= -1.0)
		{
			accumulateRotation = Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;
			path = (path + 1) % 3;
			angle = 0.0;
			rotate_x = 9.0;
			rotate_y = 0.0;
			rotate_z = -1.0;
		}
		else
		{
			angle += delta;    // rotate the ball
			if (angle > 360.0)
				angle -= 360.0;

			tx += delta * unit_len * 8 / sqrt(73);
			tz -= delta * unit_len * 3 / sqrt(73);
		}
			
	}
	else // path == 2      Path C -> A
	{
		if (tx >= 0.0)
		{
			accumulateRotation = Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;
			path = (path + 1) % 3;
			angle = 0.0;
			rotate_x = -3.0;
			rotate_y = 0.0;
			rotate_z = 2.0;
		}
		else
		{
			angle += delta;    // rotate the ball
			if (angle > 360.0)
				angle -= 360.0;

			tx += delta * unit_len * 1 / sqrt(82);
			tz += delta * unit_len * 9 / sqrt(82);
		}
	}


    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
		case 'b':
		case 'B':
			animationStarted = 1;
			animationFlag = 1;
			glutIdleFunc(idle);
			break;
		case 033: // Escape Key
		case 'q': case 'Q':
			exit( EXIT_SUCCESS );
			break;

        case 'X': eye[0] += 1.0; break;
		case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
		case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
		case 'z': eye[2] -= 1.0; break;

        case 'a': case 'A': // Toggle between animation and non-animation
	    animationFlag = 1 - animationFlag;
            if (animationFlag == 1) 
				glutIdleFunc(idle);
            else                    
				glutIdleFunc(NULL);
            break;
	   
        case 'c': case 'C': // Toggle between filled and wireframe cube
	    ballFlag = 1 - ballFlag;   
            break;

        case 'f': case 'F': // Toggle between filled and wireframe floor
	    floorFlag = 1 - floorFlag; 
            break;

		case ' ':  // reset to initial viewer/eye position
			eye = init_eye;
			break;
    }
    glutPostRedisplay();
}
// ---------------------------------------------------------------------------
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && animationStarted) 
		animationFlag = !animationFlag;
	
	if (animationFlag) {
		glutIdleFunc(idle);
	}
	else {
		glutIdleFunc(NULL);
	}
}
// ---------------------------------------------------------------------------
void reshape (int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
// ---------------------------------------------------------------------------
void readFile (void) {
	using namespace std;
	ifstream in;
	int user_input, num_of_triangle, num, count = 0;
	GLfloat x, y, z;

	cout << "Choose a file to generate the ball:" << endl;
	cout << "(1) sphere.8	(2) sphere.128" << endl;
	cout << "(3) sphere.256	(4) sphere.1024" << endl;
	cout << "Your choice (integer 1, 2, 3 or 4 only): ";
	cin >> user_input;

	if (user_input == 1)
		in.open("sphere.8");
	else if (user_input == 2)
		in.open("sphere.128");
	else if (user_input == 3)
		in.open("sphere.256");
	else
		in.open("sphere.1024");

	in >> num_of_triangle;
	ball_NumVertices = 3 * num_of_triangle;
	cout << "Number of triangles to make up the ball: " << num_of_triangle << endl;

	for (int i = 0; i < num_of_triangle; i++) {
		in >> num;  // number of vertices in a triangle
		for (int i = 0; i < num; i++) {
			in >> x >> y >> z;
			// cout << "read in: " << x << " " << y << " " << z << endl;
			
			shadow_colors[count] = color3(0.25, 0.25, 0.25);
			ball_colors[count] = color3(1.0, 0.84, 0.0);
			ball_points[count] = point3(x, y, z);
			ball_points_4[count++] = point4(x, y, z, 1.0);

			// calculate the normal for the triangle
			vec4 temp = (ball_points_4[count - 1] - point4(0.0, 0.0, 0.0, 1.0));
			ball_normals_smooth[count - 1] = point3(temp.x, temp.y, temp.z);
		}

		// calculate the normal for the triangle
		vec4 u = ball_points_4[count - 3] - ball_points_4[count - 1];
		vec4 v = ball_points_4[count - 2] - ball_points_4[count - 1];
		vec3 ball_normal = normalize(cross(u, v));
		ball_normals[count - 1] = ball_normals[count - 2] = ball_normals[count - 3] = ball_normal;
	}

	in.close();
}
// ---------------------------------------------------------------------------

// menu functions
void main_menu(int index)
{
	switch (index)
	{
		case 0:
			eye = init_eye;
			break;
		case 1:
			exit(1);
			break;
		case 2:
			ballFlag = !ballFlag;
			break;
	}
	display();
};

void shadow_menu(int index) {
	shadow = (index == 1) ? false : true;
	display();
};

void lighting_menu(int index) {
	lighting = (index == 1) ? false : true;
	display();
}

void shading_menu(int index) {
	smooth_shading = (index == 1) ? false : true;
	display();
}

void source_menu(int index) {
	lightSource = index;
	display();
}

void createMenu() {
	
	int shadow = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int lighting = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int shade = glutCreateMenu(shading_menu);
	glutAddMenuEntry("Flat shading", 1);
	glutAddMenuEntry("Smooth shading", 2);

	int source = glutCreateMenu(source_menu);
	glutAddMenuEntry("Spot Light", 1);
	glutAddMenuEntry("Point Source", 2);

	glutCreateMenu(main_menu);
	glutAddMenuEntry("Default View Point", 0);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Wire Frame Sphere", 2);
	glutAddSubMenu("Shadow", shadow);
	glutAddSubMenu("Shading", shade);
	glutAddSubMenu("Enable Lighting", lighting);
	glutAddSubMenu("Light Source", source);

	glutAttachMenu(GLUT_LEFT_BUTTON);
}

int main( int argc, char **argv )
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(650, 650);
    glutCreateWindow("Rolling Ball");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    { 
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
        exit(1);
    }
#endif

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	createMenu();
	readFile(); // read in point coordinates
    init();
    glutMainLoop();
    return 0;
}
