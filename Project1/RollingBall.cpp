#include "Angel-yjc.h"

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint ball_buffer;   /* vertex buffer object id for ball */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;		

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.1, zFar = 100.0;

mat4 accumulateRotation = Rotate(0.0, 1.0, 0.0, 0.0);
mat4 temp;

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

int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'

int ballFlag = 0;   // 1: solid ball; 0: wireframe ball. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'

int ball_NumVertices; // depends on the sphere file
#if 0
point3 cube_points[cube_NumVertices]; // positions for all vertices
color3 cube_colors[cube_NumVertices]; // colors for all vertices
#endif
#if 1
point3 ball_points[400]; 
color3 ball_colors[400];
#endif

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices

point3 axis_points[6];
color3 axis_colors[6];

/*
// Vertices of a unit cube centered at origin, sides aligned with axes
point3 vertices[8] = {
    point3( -0.5, -0.5,  0.5),
    point3( -0.5,  0.5,  0.5),
    point3(  0.5,  0.5,  0.5),
    point3(  0.5, -0.5,  0.5),
    point3( -0.5, -0.5, -0.5),
    point3( -0.5,  0.5, -0.5),
    point3(  0.5,  0.5, -0.5),
    point3(  0.5, -0.5, -0.5)
};
*/

point3 vertices[6] = {
	point3( 0.0,  1.0,  0.0),
	point3( 0.0,  0.0,  1.0),
	point3( 1.0,  0.0,  0.0),
	point3(-1.0,  0.0,  0.0),
	point3( 0.0, -1.0,  0.0),
	point3( 0.0,  0.0, -1.0)
};
/*
// RGBA colors
color3 vertex_colors[8] = {
    color3( 0.0, 0.0, 0.0),  // black
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 1.0, 0.0),  // yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0)   // cyan
};
*/
//----------------------------------------------------------------------------
int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

/*
// quad(): generate two triangles for each face and assign colors to the vertices
void quad( int a, int b, int c, int d )
{
    cube_colors[Index] = color3(1.0, 0.84, 0); cube_points[Index] = vertices[a]; Index++;
    cube_colors[Index] = color3(1.0, 0.84, 0); cube_points[Index] = vertices[b]; Index++;
    cube_colors[Index] = color3(1.0, 0.84, 0); cube_points[Index] = vertices[c]; Index++;

    cube_colors[Index] = color3(1.0, 0.84, 0); cube_points[Index] = vertices[c]; Index++;
    cube_colors[Index] = color3(1.0, 0.84, 0); cube_points[Index] = vertices[d]; Index++;
    cube_colors[Index] = color3(1.0, 0.84, 0); cube_points[Index] = vertices[a]; Index++;
}
*/

//----------------------------------------------------------------------------

/*
// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}
*/
/*
void triangle(int a, int b, int c)
{
	ball_colors[Index] = color3(1.0, 0.84, 0);
	ball_points[Index] = vertices[a];
	Index++;
	ball_colors[Index] = color3(1.0, 0.84, 0);
	ball_points[Index] = vertices[b];
	Index++;
	ball_colors[Index] = color3(1.0, 0.84, 0);
	ball_points[Index] = vertices[c];
	Index++;
}

void colorTriangle() 
{
	triangle(0, 1, 2);
	triangle(3, 1, 0);
	triangle(4, 1, 3);
	triangle(2, 1, 4);
	triangle(5, 0, 2);
	triangle(5, 3, 0);
	triangle(5, 4, 3);
	triangle(5, 2, 4);
}
*/
//-------------------------------
// generate 2 triangles: 6 vertices
void floor()
{
    floor_colors[0] = color3(0.0, 1.0, 0.0); floor_points[0] = point3(5.0, 0.0, 8.0);
    floor_colors[1] = color3(0.0, 1.0, 0.0); floor_points[1] = point3(5.0, 0.0, -4.0);
    floor_colors[2] = color3(0.0, 1.0, 0.0); floor_points[2] = point3(-5.0, 0.0, -4.0);

    floor_colors[3] = color3(0.0, 1.0, 0.0); floor_points[3] = point3(-5.0, 0.0, -4.0);
    floor_colors[4] = color3(0.0, 1.0, 0.0); floor_points[4] = point3(-5.0, 0.0, 8.0);
    floor_colors[5] = color3(0.0, 1.0, 0.0); floor_points[5] = point3(5.0, 0.0, 8.0);
}
//----------------------------------------------------------------------------

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
	// colorTriangle();

#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif

 // Create and initialize a vertex buffer object for ball, to be used in display()
    glGenBuffers(1, &ball_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, ball_buffer);

#if 0
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_points) + sizeof(cube_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(cube_colors),
                    cube_colors);
#endif
#if 1
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(point3)*ball_NumVertices + sizeof(color3)*ball_NumVertices,
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    sizeof(point3) * ball_NumVertices, ball_points);
    glBufferSubData(GL_ARRAY_BUFFER, 
                    sizeof(point3) * ball_NumVertices, 
                    sizeof(color3) * ball_NumVertices,
                    ball_colors);
#endif

    floor();     
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);


	axes();
	// Create and initialize a vertex buffer object for 3 axes, to be used in display()
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors), axis_colors);



 // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader42.glsl", "fshader42.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor(0.529, 0.807, 0.92, 0);
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

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

void drawLine(GLuint buffer, int num_vertices)
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
							BUFFER_OFFSET(sizeof(point3) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

  /* Draw a sequence of geometric objs (lines) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_LINES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

//----------------------------------------------------------------------------
void display( void )
{
  GLuint  model_view;  // model-view matrix uniform shader variable location
  GLuint  projection;  // projection matrix uniform shader variable location

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );

/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);

    mat4  mv = LookAt(eye, at, up);


/*----- Set Up the Model-View matrix for the BALL -----*/
#if 0 // The following is to verify the correctness of the function NormalMatrix():
      // Commenting out Rotate() and un-commenting mat4WithUpperLeftMat3() 
      // gives the same result.
      mv = mv * Translate(0.0, 0.5, 0.0) * Scale (1.4, 1.4, 1.4) 
              * Rotate(angle, 0.0, 0.0, 2.0); 
           // * mat4WithUpperLeftMat3(NormalMatrix(Rotate(angle, 0.0, 0.0, 2.0), 1));
#endif
#if 1 // The following is to verify that Rotate() about (0,2,0) is RotateY():
      // Commenting out Rotate() and un-commenting RotateY()
      // gives the same result.
  //
  // The set-up below gives a new scene (scene 2), using Correct LookAt().
	  mv = mv * Translate(3.0 + tx, 1.0, 5.0 + tz) * Scale(1.0, 1.0, 1.0)
		   * Rotate(angle, rotate_x, rotate_y, rotate_z) * accumulateRotation;
	   // * RotateY(angle);
  //
  // The set-up below gives the original scene (scene 1), using Correct LookAt().
  //  mv = Translate(0.0, 0.5, 0.0) * mv * Scale (1.4, 1.4, 1.4) 
  //               * Rotate(angle, 0.0, 2.0, 0.0);
	        // * RotateY(angle); 
  //
  // The set-up below gives the original scene (scene 1), when using previously 
  //     Incorrect LookAt() (= Translate(1.0, 1.0, 0.0) * correct LookAt() )
  //  mv = Translate(-1.0, -0.5, 0.0) * mv * Scale (1.4, 1.4, 1.4) 
  //               * Rotate(angle, 0.0, 2.0, 0.0);
	        // * RotateY(angle);
  //
#endif
#if 0  // The following is to verify that Rotate() about (3,0,0) is RotateX():
       // Commenting out Rotate() and un-commenting RotateX()
       // gives the same result.
      mv = mv * Translate(0.0, 0.5, 0.0) * Scale (1.4, 1.4, 1.4)
                    * Rotate(angle, 3.0, 0.0, 0.0);
                 // * RotateX(angle);
#endif
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    if (ballFlag == 1) // Filled ball
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe ball
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(ball_buffer, ball_NumVertices);  // draw the ball

/*----- Set up the Mode-View matrix for the floor -----*/
 // The set-up below gives a new scene (scene 2), using Correct LookAt() function
       mv = LookAt(eye, at, up);
 //		* Translate(0.0, 0.0, 0.0) * Scale (1.0, 1.0, 1.0);
 // The set-up below gives the original scene (scene 1), using Correct LookAt()
 //    mv = Translate(0.0, 0.0, 0.3) * LookAt(eye, at, up) * Scale (1.6, 1.5, 3.3);
 //
 // The set-up below gives the original scene (scene 1), when using previously 
 //       Incorrect LookAt() (= Translate(1.0, 1.0, 0.0) * correct LookAt() ) 
 //    mv = Translate(-1.0, -1.0, 0.3) * LookAt(eye, at, up) * Scale (1.6, 1.5, 3.3);
 //
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    if (floorFlag == 1) // Filled floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(floor_buffer, floor_NumVertices);  // draw the floor



/*----- Set up the Mode-View matrix for xyz axes -----*/
 // The set-up below gives a new scene (scene 2), using Correct LookAt() function
	mv = LookAt(eye, at, up) * Translate(0.0, 0.0, 0.0) * Scale(10.0, 10.0, 10.0);
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
	    animationFlag = 1 -  animationFlag;
            if (animationFlag == 1) glutIdleFunc(idle);
            else                    glutIdleFunc(NULL);
            break;
	   
        case 'c': case 'C': // Toggle between filled and wireframe cube
	    ballFlag = 1 -  ballFlag;   
            break;

        case 'f': case 'F': // Toggle between filled and wireframe floor
	    floorFlag = 1 -  floorFlag; 
            break;

	case ' ':  // reset to initial viewer/eye position
	    eye = init_eye;
	    break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape (int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void readFile (void) {
	using namespace std;
	ifstream in;
	int user_input, num_of_triangle, num, count = 0;
	GLfloat x, y, z;

	cout << "Choose a file to generate the ball:" << endl;
	cout << "(1) sphere.8	(2) sphere.128" << endl;
	cout << "Your choice (integer 1 or 2 only): ";
	cin >> user_input;

	if (user_input == 1) 
		in.open("sphere.8");
	else			
		in.open("sphere.128");

	in >> num_of_triangle;
	ball_NumVertices = 3 * num_of_triangle;
	cout << "Number of triangles to make up the ball: " << num_of_triangle << endl;

	for (int i = 0; i < num_of_triangle; i++) {
		in >> num;
		for (int i = 0; i < num; i++) {
			in >> x >> y >> z;
			cout << "read in: " << x << " " << y << " " << z << endl;
			ball_points[count] = point3(x, y, z);
			ball_colors[count++] = color3(1.0, 0.84, 0.0);
		}
	}
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Color Ball");

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
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);

	readFile(); // read in point coordinates
    init();
    glutMainLoop();
    return 0;
}
