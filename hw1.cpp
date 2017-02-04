//Modified By: Robert Ripley
//Date: 1-6-17


//Author: Gordon Griesel
//Date: 2014-Present
//All Rights Resevered
//cs3350 Spring 2017 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <math.h>
#include <cmath>

extern "C" {
#include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 5000
#define GRAVITY 0.01
#define rnd() (float)rand() / (float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Bubbler {
	Shape box;
	bool  isOn;
};

struct Game {
	Shape box[5];
	Bubbler bubbler;
	Particle particle[50000];
	int n;
	Game() {n=0;}
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
bool checkBubblerClick(Game* game, int x, int y);
void check_mouse(XEvent *e, Game *game);
int  check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void makeParticle(Game *game, int x, int y);
void makeBox(float xSpot, float ySpot);
bool checkBoxCollision(Game *game, Shape s);
bool checkCircleCollision(Shape s);
Rect boxToRect(Shape s);

bool isOn;
int  boxHeight = 15;
int  boxWidth  = 25;

int main(void)
{

	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	Bubbler bubbler;
	game.bubbler = bubbler;
	game.n=0;

	//init Bubbler
	game.bubbler.box.width = 25;
	game.bubbler.box.height = 15;
	game.bubbler.box.center.x = WINDOW_WIDTH/5;
	game.bubbler.box.center.y = WINDOW_HEIGHT-WINDOW_HEIGHT/10 + 15 * 2;

	float w = 80;
	float h = 15;

	//declare a box shape
	Shape b1,b2,b3,b4,b5;

	b1.center.x	= WINDOW_WIDTH/5;
	b1.center.y = WINDOW_HEIGHT-WINDOW_HEIGHT/10 - h *5;

	b2.center.x = WINDOW_WIDTH/5 + w;
	b2.center.y = WINDOW_HEIGHT-WINDOW_HEIGHT/10 - h*10;

	b3.center.x = WINDOW_WIDTH/5 + w*2;
	b3.center.y = WINDOW_HEIGHT-WINDOW_HEIGHT/10 - h*15;

	b4.center.x = WINDOW_WIDTH/5 + w*3;
	b4.center.y = WINDOW_HEIGHT-WINDOW_HEIGHT/10 - h*20;

	b5.center.x = WINDOW_WIDTH/5 + w*4;
	b5.center.y = WINDOW_HEIGHT-WINDOW_HEIGHT/10 - h*25;

	game.box[0] = b1;
	game.box[1] = b2;
	game.box[2] = b3;
	game.box[3] = b4;
	game.box[4] = b5;

	for (int i = 0; i < 5; i++) {
		game.box[i].width = 80;
		game.box[i].width = 15;
	}

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}

		if (isOn) {
			makeParticle(&game, WINDOW_WIDTH/5, WINDOW_HEIGHT-WINDOW_HEIGHT/10 + 15 * 2);		
		}

		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanup_fonts();
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Allow Fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();

}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = -.05;
	p->velocity.x =  (rnd() * 0.05);
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			//makeParticle(game, e->xbutton.x, y);
			checkBubblerClick(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		//int y = WINDOW_HEIGHT - e->xbutton.y;
		for (int i = 0; i < 10; i++) {
			//makeParticle(game, e->xbutton.x, y);
		}

		if (++n < 10)
			return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.



	}
	return 0;
}

void movement(Game *game)
{

    for (int i = 0; i < game->n; i++) {

	Particle *p;

	if (game->n <= 0)
		return;

	p = &game->particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;

	//check for collision with shapes...
	//Shape *s;

	//check for off-screen
	if (p->s.center.y < 0.0) {
		//std::cout << "off screen" << std::endl;
		game->particle[i] = game->particle[game->n-1];
		game->n -= 1;
	}


	if (checkBoxCollision(game, p->s) || checkCircleCollision(p->s)) {
		//std::cout << "Collide\n";
		p->s.center.y++;
		p->velocity.y = -p->velocity.y/2;
	}

	//Apply Gravity
	p->velocity.y -= .00015;

    }

}

bool checkBoxCollision(Game *game, Shape s) {

	float xSpot = s.center.x;
	float ySpot = s.center.y;

	for (int i = 0; i < 5; i++) {

		//float bWidth  = game->box[i].width;
		//float bHeight = game->box[i].height;
		float bWidth  = 80;
		float bHeight = 15;
		float boxX    = game->box[i].center.x;
		float boxY    = game->box[i].center.y;

		//std::cout << "w " << bWidth << " h " << bHeight << " x " << boxX << " y " << boxY << std::endl;

		if (xSpot > boxX-bWidth && xSpot < boxX+bWidth) {

			if (ySpot > boxY-bHeight && ySpot < boxY+bHeight) {
				return true;
			}

		}

	}	

	return false;

}

bool checkCircleCollision(Shape s) {

	float xSpot = s.center.x;
	float ySpot = s.center.y;
	float cX	= WINDOW_WIDTH-120;
	float cY	= 10;
	float dist  = pow((xSpot - cX),2) + pow((ySpot - cY),2);
	dist	    = sqrt(dist);

	if (dist <= 120) {
		return true;
	}

	return false;

}

bool checkBubblerClick(Game *game, int x, int y) {
	
	float boxX, boxY, width, height;   
	boxX   = game->bubbler.box.center.x;
	boxY   = game->bubbler.box.center.y;
	width  = game->bubbler.box.width;
	height = game->bubbler.box.height; 

	if (x > boxX - width && x < boxX + width) {

		if (y > boxY - height && y < boxY + height) {

			if (isOn) {
				std::cout << "Turn Off \n";
				isOn = false;
			}

			else {
				std::cout << "Turn On \n";
				isOn = true;
			}

			return true;
		}
	
	}

	return false;

}

void makeBox(float xSpot, float ySpot) 
{

	float w,h;
	Shape shape;
	glColor3ub(90,140,90);
	shape.width    = 100;
	shape.height   = 15;
	shape.center.x = xSpot;
	shape.center.y = ySpot;
	glPushMatrix();
	glTranslatef(shape.center.x, shape.center.y, shape.center.z);
	w = shape.width;
	h = shape.height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();

}

void drawCircle(float cx, float cy, float r, int num_segments)
{

    glBegin(GL_LINE_LOOP);
    for(int ii = 0; ii < num_segments; ii++)
    {

        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle

        float x = r * cosf(theta);//calculate the x component
        float y = r * sinf(theta);//calculate the y component

        glVertex2f(x + cx, y + cy);//output vertex

    }
    glEnd();
}


Rect boxToRect(Shape s) {

	Rect rect;
	rect.centerx = s.center.x;
	rect.centery = s.center.y;
	rect.width	 = 160;
	rect.height  = 30;
	rect.bot	 = s.center.y - 15;
	rect.top 	 = s.center.y + 15;
	rect.left    = s.center.x - 70;
	rect.right   = s.center.x + 70;
	return rect;

}

void render(Game *game)
{

	float w, h;

	glClear(GL_COLOR_BUFFER_BIT);

	Rect r = boxToRect(game->box[0]);

	ggprint8b(&r, 16, 0x00dddd00, "B - Bigfoot");
	//Draw shapes...

	/*
	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	s = &game->box;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	*/

	//draw circle
	drawCircle(WINDOW_WIDTH-120,10,120,300);

	//draw box
	Shape *s1;
	glColor3ub(90,0,90);
	s1 = &game->bubbler.box;
	glPushMatrix();
	glTranslatef(s1->center.x, s1->center.y, s1->center.z);
	w = s1->width;
	h = s1->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();

	for (int i = 0; i < 5; i++) {
		float xSpot = game->box[i].center.x;
		float ySpot = game->box[i].center.y;
		makeBox(xSpot,ySpot);
	}

	//draw all particles here
	glPushMatrix();
	glColor3ub(150,160,220);

    for (int i = 0; i < game->n; i++) {

	Vec *c = &game->particle[i].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);

    }

	glEnd();
	glPopMatrix();
}



