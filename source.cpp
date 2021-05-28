#include <Eigen/Eigen>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <map>
#include <set>
#include <deque>
#include <queue>
#include <stack>
#include <bitset>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <GL/freeglut.h>

using namespace std;
using namespace Eigen;

#define sp system("pause")
#define FOR(i,a,b) for(int i=a;i<=b;++i)
#define FORD(i,a,b) for(int i=a;i>=b;--i)
#define REP(i,n) for(int i=0;i<n;++i)
#define ll long long

#define CUBE_SIZE 3

int layer = 0;
int width = 1200, height = 750;

class point {
public:
	double x, y, z;
	point() {

	}
	point(double px, double py, double pz) {
		x = px;
		y = py;
		z = pz;
	}
};

// for selecting colors for each face of the cube
class color {
public:
	float r, g, b;

	color() {
		r = 0;
		g = 0;
		b = 0;
	}

	color(float ir, float ig, float ib) {
		r = ir;
		g = ig;
		b = ib;
	}

	float * getArray() {
		float c[3];
		c[0] = r;
		c[1] = g;
		c[2] = b;
		return c;
	}
};

class button {
	static float xelev;
	static float yelev;
	static float xshadowMul;
	static float yshadowMul;
	static float clickdelay;
public:
	bool is3D, isAnimating;
	float x, y;
	float xoff, yoff;
	float w, h;
	color bg, fg;
	color yshadow, xshadow;
	string text;

	button(float px, float py, float iw, float ih, color background, color foreground, string disptext) {
		x = px;
		y = py;
		w = iw;
		h = ih;
		is3D = false;
		isAnimating = false;
		bg = background;
		xshadow = color(bg.r*xshadowMul, bg.g*xshadowMul, bg.b*xshadowMul);
		yshadow = color(bg.r*yshadowMul, bg.g*yshadowMul, bg.b*yshadowMul);
		fg = foreground;
		text = disptext;
		xoff = yoff = 0;
	}
	//Draw function is called when a button has to be drawn
	void draw() {

		glColor3fv(bg.getArray());
		glBegin(GL_POLYGON);
		glVertex3f(x + xoff, y + yoff, 1);
		glVertex3f(x + w + xoff, y + yoff, 1);
		glVertex3f(x + w + xoff, y - h + yoff, 1);
		glVertex3f(x + xoff, y - h + yoff, 1);
		glEnd();
		glPushMatrix();
		glTranslatef(x + xoff + (w - text.size()*0.135) / 2, y + yoff - (h + 0.12) / 2, 1.1);
		glScalef(1 / 800.0, 1 / 800.0, 0);
		glLineWidth(2);
		glColor3fv(fg.getArray());

		REP(i, text.size())
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, text[i]);
		glPopMatrix();

	}

	// Method helps check which button is being pressed
	bool collision(float px, float py) {
		if (!isAnimating && px > x + xoff && px<x + w + xoff && py>y - h + yoff && py < y + yoff)
			return true;
		return false;
	}

	void animate(void actionEvent()) {
		//xoff is the value that defines the displacement of the button when clicked in x direction
		//yoff is the value that defines the displacement of the button when clicked in y direction

		if (isAnimating) {
			xoff += xelev / clickdelay;
			yoff += yelev / clickdelay;

			if (xoff + x >= x + xelev) {
				isAnimating = false;
				actionEvent();
			}
		}
		else {
			xoff = max(0, xoff - xelev / clickdelay);
			yoff = max(0, yoff - yelev / clickdelay);
		}
	}

};

float button::xelev = 0.07;//0.07
float button::yelev = 0.05;//0.05
float button::xshadowMul = 0.7;
float button::yshadowMul = 0.5;
float button::clickdelay = 50; // defines after click duration

class State {

public:
	int faces[6][CUBE_SIZE][CUBE_SIZE]; //denotes the six faces of a cube

	State() {
		REP(k, 6) {
			REP(i, CUBE_SIZE)
				REP(j, CUBE_SIZE)
				faces[k][i][j] = k;
		}
	}
	// checks if all the cubes on each face have the same color
	bool isSolved() {

		REP(k, 6) {
			int color = faces[k][0][0];
			REP(j, CUBE_SIZE) {
				REP(i, CUBE_SIZE) {
					if (faces[k][j][i] != color)
						return false;
				}
			}
		}
		return true;

	}

	//the following blocks are used to swap face values among different faces by use of a temporary variable
	void front_anticlock() {

		int t[CUBE_SIZE];
		//sideface values are changed
		REP(i, CUBE_SIZE) {
			t[i] = faces[4][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i];
			faces[4][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i] = faces[3][CUBE_SIZE - 1 - i][layer];
			faces[3][CUBE_SIZE - 1 - i][layer] = faces[5][layer][i];
			faces[5][layer][i] = faces[2][i][CUBE_SIZE - 1 - layer];
			faces[2][i][CUBE_SIZE - 1 - layer] = t[i];
		}
		// front face is adjusted 
		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[0][l][CUBE_SIZE - 1 - i];
					faces[0][l][CUBE_SIZE - 1 - i] = faces[0][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l];
					faces[0][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l] = faces[0][CUBE_SIZE - 1 - l][i];
					faces[0][CUBE_SIZE - 1 - l][i] = faces[0][i][l];
					faces[0][i][l] = t[i];
				}
			}
		}

	}

	void front_clock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][CUBE_SIZE - 1 - layer][i];
			faces[4][CUBE_SIZE - 1 - layer][i] = faces[2][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer];
			faces[2][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer] = faces[5][layer][CUBE_SIZE - 1 - i];
			faces[5][layer][CUBE_SIZE - 1 - i] = faces[3][i][layer];
			faces[3][i][layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[0][l][i];
					faces[0][l][i] = faces[0][CUBE_SIZE - 1 - i][l];
					faces[0][CUBE_SIZE - 1 - i][l] = faces[0][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i];
					faces[0][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i] = faces[0][i][CUBE_SIZE - 1 - l];
					faces[0][i][CUBE_SIZE - 1 - l] = t[i];
				}
			}
		}

	}

	void back_anticlock() {
		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][layer][i];
			faces[4][layer][i] = faces[2][CUBE_SIZE - 1 - i][layer];
			faces[2][CUBE_SIZE - 1 - i][layer] = faces[5][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i];
			faces[5][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i] = faces[3][i][CUBE_SIZE - 1 - layer];
			faces[3][i][CUBE_SIZE - 1 - layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[1][l][CUBE_SIZE - 1 - i];
					faces[1][l][CUBE_SIZE - 1 - i] = faces[1][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l];
					faces[1][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l] = faces[1][CUBE_SIZE - 1 - l][i];
					faces[1][CUBE_SIZE - 1 - l][i] = faces[1][i][l];
					faces[1][i][l] = t[i];
				}
			}
		}
	}

	void back_clock() {
		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][layer][CUBE_SIZE - 1 - i];
			faces[4][layer][CUBE_SIZE - 1 - i] = faces[3][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer];
			faces[3][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer] = faces[5][CUBE_SIZE - 1 - layer][i];
			faces[5][CUBE_SIZE - 1 - layer][i] = faces[2][i][layer];
			faces[2][i][layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[1][l][i];
					faces[1][l][i] = faces[1][CUBE_SIZE - 1 - i][l];
					faces[1][CUBE_SIZE - 1 - i][l] = faces[1][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i];
					faces[1][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i] = faces[1][i][CUBE_SIZE - 1 - l];
					faces[1][i][CUBE_SIZE - 1 - l] = t[i];
				}
			}
		}
	}

	void left_anticlock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][CUBE_SIZE - 1 - i][layer];
			faces[4][CUBE_SIZE - 1 - i][layer] = faces[0][CUBE_SIZE - 1 - i][layer];
			faces[0][CUBE_SIZE - 1 - i][layer] = faces[5][CUBE_SIZE - 1 - i][layer];
			faces[5][CUBE_SIZE - 1 - i][layer] = faces[1][i][CUBE_SIZE - 1 - layer];
			faces[1][i][CUBE_SIZE - 1 - layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[2][l][CUBE_SIZE - 1 - i];
					faces[2][l][CUBE_SIZE - 1 - i] = faces[2][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l];
					faces[2][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l] = faces[2][CUBE_SIZE - 1 - l][i];
					faces[2][CUBE_SIZE - 1 - l][i] = faces[2][i][l];
					faces[2][i][l] = t[i];
				}
			}
		}
	}

	void left_clock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][i][layer];
			faces[4][i][layer] = faces[1][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer];
			faces[1][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer] = faces[5][i][layer];
			faces[5][i][layer] = faces[0][i][layer];
			faces[0][i][layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[2][l][i];
					faces[2][l][i] = faces[2][CUBE_SIZE - 1 - i][l];
					faces[2][CUBE_SIZE - 1 - i][l] = faces[2][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i];
					faces[2][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i] = faces[2][i][CUBE_SIZE - 1 - l];
					faces[2][i][CUBE_SIZE - 1 - l] = t[i];
				}
			}
		}

	}

	void right_anticlock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][i][CUBE_SIZE - 1 - layer];
			faces[4][i][CUBE_SIZE - 1 - layer] = faces[1][CUBE_SIZE - 1 - i][layer];
			faces[1][CUBE_SIZE - 1 - i][layer] = faces[5][i][CUBE_SIZE - 1 - layer];
			faces[5][i][CUBE_SIZE - 1 - layer] = faces[0][i][CUBE_SIZE - 1 - layer];
			faces[0][i][CUBE_SIZE - 1 - layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[3][l][CUBE_SIZE - 1 - i];
					faces[3][l][CUBE_SIZE - 1 - i] = faces[3][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l];
					faces[3][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l] = faces[3][CUBE_SIZE - 1 - l][i];
					faces[3][CUBE_SIZE - 1 - l][i] = faces[3][i][l];
					faces[3][i][l] = t[i];
				}
			}
		}

	}

	void right_clock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[4][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer];
			faces[4][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer] = faces[0][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer];
			faces[0][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer] = faces[5][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer];
			faces[5][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - layer] = faces[1][i][layer];
			faces[1][i][layer] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[3][l][i];
					faces[3][l][i] = faces[3][CUBE_SIZE - 1 - i][l];
					faces[3][CUBE_SIZE - 1 - i][l] = faces[3][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i];
					faces[3][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i] = faces[3][i][CUBE_SIZE - 1 - l];
					faces[3][i][CUBE_SIZE - 1 - l] = t[i];
				}
			}
		}

	}

	void up_anticlock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[1][layer][i];
			faces[1][layer][i] = faces[3][layer][i];
			faces[3][layer][i] = faces[0][layer][i];
			faces[0][layer][i] = faces[2][layer][i];
			faces[2][layer][i] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[4][l][CUBE_SIZE - 1 - i];
					faces[4][l][CUBE_SIZE - 1 - i] = faces[4][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l];
					faces[4][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l] = faces[4][CUBE_SIZE - 1 - l][i];
					faces[4][CUBE_SIZE - 1 - l][i] = faces[4][i][l];
					faces[4][i][l] = t[i];
				}
			}
		}
	}

	void up_clock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[1][layer][CUBE_SIZE - 1 - i];
			faces[1][layer][CUBE_SIZE - 1 - i] = faces[2][layer][CUBE_SIZE - 1 - i];
			faces[2][layer][CUBE_SIZE - 1 - i] = faces[0][layer][CUBE_SIZE - 1 - i];
			faces[0][layer][CUBE_SIZE - 1 - i] = faces[3][layer][CUBE_SIZE - 1 - i];
			faces[3][layer][CUBE_SIZE - 1 - i] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[4][l][i];
					faces[4][l][i] = faces[4][CUBE_SIZE - 1 - i][l];
					faces[4][CUBE_SIZE - 1 - i][l] = faces[4][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i];
					faces[4][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i] = faces[4][i][CUBE_SIZE - 1 - l];
					faces[4][i][CUBE_SIZE - 1 - l] = t[i];
				}
			}
		}
	}

	void down_anticlock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[0][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i];
			faces[0][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i] = faces[3][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i];
			faces[3][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i] = faces[1][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i];
			faces[1][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i] = faces[2][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i];
			faces[2][CUBE_SIZE - 1 - layer][CUBE_SIZE - 1 - i] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[5][l][CUBE_SIZE - 1 - i];
					faces[5][l][CUBE_SIZE - 1 - i] = faces[5][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l];
					faces[5][CUBE_SIZE - 1 - i][CUBE_SIZE - 1 - l] = faces[5][CUBE_SIZE - 1 - l][i];
					faces[5][CUBE_SIZE - 1 - l][i] = faces[5][i][l];
					faces[5][i][l] = t[i];
				}
			}
		}

	}

	void down_clock() {

		int t[CUBE_SIZE];

		REP(i, CUBE_SIZE) {
			t[i] = faces[0][CUBE_SIZE - 1 - layer][i];
			faces[0][CUBE_SIZE - 1 - layer][i] = faces[2][CUBE_SIZE - 1 - layer][i];
			faces[2][CUBE_SIZE - 1 - layer][i] = faces[1][CUBE_SIZE - 1 - layer][i];
			faces[1][CUBE_SIZE - 1 - layer][i] = faces[3][CUBE_SIZE - 1 - layer][i];
			faces[3][CUBE_SIZE - 1 - layer][i] = t[i];
		}

		if (layer == 0) {
			REP(l, CUBE_SIZE / 2) {
				FOR(i, l, CUBE_SIZE - 2 - l) {
					t[i] = faces[5][l][i];
					faces[5][l][i] = faces[5][CUBE_SIZE - 1 - i][l];
					faces[5][CUBE_SIZE - 1 - i][l] = faces[5][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i];
					faces[5][CUBE_SIZE - 1 - l][CUBE_SIZE - 1 - i] = faces[5][i][CUBE_SIZE - 1 - l];
					faces[5][i][CUBE_SIZE - 1 - l] = t[i];
				}
			}
		}

	}

	bool operator<(const State& rhs) const { // rhs holds s [s holds contents after bfs makes a move]
		bool returnType;
		REP(k, 6) {
			REP(j, CUBE_SIZE) {
				REP(i, CUBE_SIZE) {
					if (faces[k][j][i] != rhs.faces[k][j][i]) { // faces holds moves made by user
						returnType = faces[k][j][i] < rhs.faces[k][j][i];
						return returnType;
					}
				}
			}
		}
		return false;

	}

	bool operator>(const State& rhs) const {

		REP(k, 6) {
			REP(j, 3) {
				REP(i, 3) {
					if (faces[k][j][i] != rhs.faces[k][j][i])
						return faces[k][j][i] > rhs.faces[k][j][i];
				}
			}
		}
		return false;
	}

	bool operator==(const State& rhs) const {

		REP(k, 6) {
			REP(j, 3) {
				REP(i, 3) {
					if (faces[k][j][i] != rhs.faces[k][j][i])
						return false;
				}
			}
		}
		return true;
	}

	/*
	0 - Front
	1 - Back
	2 - Left
	3 - Right
	4 - Up
	5 - Down
	*/

};

bool hollow = false;
bool change = false;
bool isTransition = false;
bool isMousePressed = false;

int px = -1, py = -1;
float xscreen = 0, fromXScreen = 0, toXScreen = 0;
float yscreen = 0, fromYScreen = 0, toYScreen = 0;
float transition_percent = 0;
float cameraX = -0, cameraY = 0, cameraZ = 0;
float viewportX = 8, viewportY = 5;

double transitionSpeed = min(1, pow(10, floor((CUBE_SIZE - 3) / 10)) / 1000);

int rotationType = 0;
const double PI = 3.1415926535;
double rorationSpeed = .30;

double totalRotation = 0;
Vector3d rotationAxis;

button bstart = button(-0.5, 0.75, 0.8, 0.3, color(0, 0.8, 0), color(0, 0, 0), "START"); //
button babout = button(-3.5, 0.5, 0.8, 0.3, color(1, 0.8, 0), color(0, 0, 0), "ABOUT");
button bhelp = button(-0.5, 0.0, 0.8, 0.3, color(0, 0.6, 1), color(0, 0, 0), "HELP");
button bexit = button(-0.5, -0.75, 0.8, 0.3, color(1, 0.1, 0.1), color(0, 0, 0), "EXIT");

button bback10 = button(4.5, -2, 0.8, 0.3, color(1, 0.1, 0.1), color(0, 0, 0), "BACK");

button bsolve = button(4.5, 1.5, 0.8, 0.3, color(1, 0, 0.6), color(0, 0, 0), "SOLVE");

button bback01 = button(-3.5, 3, 0.8, 0.3, color(1, 0.1, 0.1), color(0, 0, 0), "BACK");

button bback0_1 = button(-3.5, -7, 0.8, 0.3, color(1, 0.1, 0.1), color(0, 0, 0), "BACK");

State cube1;
Quaterniond camera = Quaterniond{ AngleAxisd{1, Vector3d{0,0,0}} };

double *matrix = new double[16];
vector<char> moveList;
vector<point> rotationQueue;
color colorList[7] = { color(0.3,0.8,0), color(0,0.5,1), color(1,0.8,0), color(0.9,0.9,0.9),  color(1,0.4,0), color(0.9,0,0), color(0.2,0.2,0.2) };

Quaterniond cubesRotation[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];

//Used to display text on screen
void printText(float x, float y, string text, float size, void *font = GLUT_STROKE_ROMAN, color fg = color(1, 1, 1), float stroke = 2) {

	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size / 800.0, size / 800.0, 0);
	glLineWidth(stroke);
	glColor3fv(fg.getArray());

	REP(i, text.size())
		glutStrokeCharacter(font, text[i]);
	glPopMatrix();

}
//sets the transition flag for changing pages 
void enableTransition() {
	isTransition = true;
}

//On exit button click
void exitProgram() {

	printf("Exiting");
	Sleep(1000);
	exit(0);
}

//Used to display the moves to be performed to solve the cube
void BFS(vector<char> &moves) {

	State cur;
	set<State> visited;
	queue<State> q;
	map<State, pair<char, State>> parent;


	visited.insert(cube1); // cube1 is the current state of the cube
	q.push(cube1);

	while (!q.empty()) {

		State s = q.front();
		State copy = s;
		q.pop();

		if (s.isSolved()) { //checks if the all the faces have the same color 
			cur = s;
			break;
		}

		// Clockwise turns
		s.front_clock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('q', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.front_anticlock();

		s.back_clock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('w', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.back_anticlock();

		s.left_clock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('a', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.left_anticlock();

		s.right_clock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('s', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.right_anticlock();

		s.up_clock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('z', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.up_anticlock();

		s.down_clock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('x', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.down_anticlock();

		// Anti-Clockwise turns
		s.front_anticlock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('Q', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.front_clock();

		s.back_anticlock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('W', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.back_clock();

		s.left_anticlock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('A', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.left_clock();

		s.right_anticlock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('S', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.right_clock();

		s.up_anticlock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('Z', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.up_clock();

		s.down_anticlock();
		if (visited.find(s) == visited.end()) {
			visited.insert(s);
			parent[s] = make_pair('X', copy);
			q.push(s);
			if (s.isSolved()) {
				cur = s;
				break;
			}
		}
		s.down_clock();

	}

	while (parent.count(cur)) {
		pair<char, State> s = parent[cur];
		moves.push_back(s.first);
		cur = s.second;
	}

	reverse(moves.begin(), moves.end()); //displays the moves in the right order that have to be performed

}


void solve() {
	moveList.clear(); //clears the moves list everytime the solve-button is clicked is click
	BFS(moveList);	//calls BFS to store moves
}

void animateButtons() {
	// The if condition checks which screen the user is on and decides which screen they are on.

	if ((ceil(xscreen) == 0 || floor(xscreen) == 0) &&
		(ceil(yscreen) == 0 || floor(yscreen) == 0)) {
		// Home screen
		bstart.animate(enableTransition);
		bhelp.animate(enableTransition);
		bexit.animate(exitProgram);
	}
	if ((ceil(xscreen) == 1 || floor(xscreen) == 1) &&
		(ceil(yscreen) == 0 || floor(yscreen) == 0)) {
		// Cube screen
		bback10.animate(enableTransition);
		bsolve.animate(solve);
	}
	if ((ceil(xscreen) == 0 || floor(xscreen) == 0) &&
		(ceil(yscreen) == -1 || floor(yscreen) == -1)) {
		// Help screen
		bback0_1.animate(enableTransition);
	}
}

// used to call the swap function
void changeState() {

	REP(a, rotationQueue.size()) {
		point mci = rotationQueue[a];
		int i = mci.z, j = mci.y, k = mci.x;
		cubesRotation[i][j][k] = Quaterniond{ AngleAxisd{ 1, Vector3d{ 0,0,0 } } };
	}

	rotationQueue.clear();
	totalRotation = 0;

	if (rotationType == 1)
		cube1.front_anticlock();
	else if (rotationType == 2)
		cube1.back_anticlock();
	else if (rotationType == 3)
		cube1.left_anticlock();
	else if (rotationType == 4)
		cube1.right_anticlock();
	else if (rotationType == 5)
		cube1.up_anticlock();
	else if (rotationType == 6)
		cube1.down_anticlock();

	else if (rotationType == 7)
		cube1.front_clock();
	else if (rotationType == 8)
		cube1.back_clock();
	else if (rotationType == 9)
		cube1.left_clock();
	else if (rotationType == 10)
		cube1.right_clock();
	else if (rotationType == 11)
		cube1.up_clock();
	else if (rotationType == 12)
		cube1.down_clock();


	rotationType = 0;

}
// converts degree to radians
inline double degtorad(double deg) {
	return PI * deg / 180;
}

double* getRotationMatrix(Quaterniond &q) { // returns a column-major ordered matrix

	Matrix3d rotMat = q.normalized().toRotationMatrix();
	//defines the matrix for translation operation
	matrix[0] = rotMat(0, 0);	matrix[4] = rotMat(0, 1);	matrix[8] = rotMat(0, 2);	matrix[12] = 0;
	matrix[1] = rotMat(1, 0);	matrix[5] = rotMat(1, 1);	matrix[9] = rotMat(1, 2);	matrix[13] = 0;
	matrix[2] = rotMat(2, 0);	matrix[6] = rotMat(2, 1);	matrix[10] = rotMat(2, 2);	matrix[14] = 0;
	matrix[3] = 0;				matrix[7] = 0;				matrix[11] = 0;				matrix[15] = 1;

	return matrix;

}

// handles keyboard events
void keyboard(unsigned char key, int x, int y) {
	if (xscreen == 1 && yscreen == 0) {  // if on the cube screen . Xscreen == 1 indicates the cube screen to restrict the input if on other screen.
		if (rotationType == 0) {

			if (key == 'Q') {
				rotationAxis = { 0, 0, 1 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(j, i, layer));
				}
				if (rorationSpeed < 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 1;
			}
			else if (key == 'q') {
				rotationAxis = { 0, 0, 1 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(j, i, layer));
				}
				if (rorationSpeed > 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 7;
			}
			else if (key == 'W') {
				rotationAxis = { 0, 0, 1 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(j, i, CUBE_SIZE - 1 - layer));
				}
				if (rorationSpeed > 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 2;
			}
			else if (key == 'w') {
				rotationAxis = { 0, 0, 1 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(j, i, CUBE_SIZE - 1 - layer));
				}
				if (rorationSpeed < 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 8;
			}
			else if (key == 'A') {
				rotationAxis = { 1, 0, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(layer, i, j));
				}
				if (rorationSpeed > 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 3;
			}
			else if (key == 'a') {
				rotationAxis = { 1, 0, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(layer, i, j));
				}
				if (rorationSpeed < 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 9;//speed=9
			}
			else if (key == 'S') {
				rotationAxis = { 1, 0, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(CUBE_SIZE - 1 - layer, i, j));
				}
				if (rorationSpeed < 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 4;
			}
			else if (key == 's') {
				rotationAxis = { 1, 0, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(CUBE_SIZE - 1 - layer, i, j));
				}
				if (rorationSpeed > 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 10;
			}
			else if (key == 'Z') {
				rotationAxis = { 0, 1, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(i, layer, j));
				}
				if (rorationSpeed < 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 5;
			}
			else if (key == 'z') {
				rotationAxis = { 0, 1, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(i, layer, j));
				}
				if (rorationSpeed > 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 11;
			}
			else if (key == 'X') {
				rotationAxis = { 0, 1, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(i, CUBE_SIZE - 1 - layer, j));
				}
				if (rorationSpeed > 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 6;
			}
			else if (key == 'x') {
				rotationAxis = { 0, 1, 0 };
				REP(i, CUBE_SIZE) {
					REP(j, CUBE_SIZE)
						rotationQueue.push_back(point(i, CUBE_SIZE - 1 - layer, j));
				}
				if (rorationSpeed < 0)
					rorationSpeed = -rorationSpeed;
				rotationType = 12;
			}
		}
	}

}
// mouse events are handling

void mouse(int button, int state, int x, int y) {

	if (isMousePressed == false) {
		float glx = (x - (float)width / 2)*viewportX / width + cameraX;
		float gly = ((float)height / 2 - y)*viewportY / height + cameraY;

		printf("Mouse Click at %f, %f\n", glx, gly);

		if (xscreen == 0 && yscreen == 0) {
			if (bstart.collision(glx, gly)) {
				bstart.isAnimating = true;
				xscreen = 0.5;
				fromXScreen = 0;
				toXScreen = 1;
				yscreen = 0;
				fromYScreen = 0;
				toYScreen = 0;
			}
			else if (bhelp.collision(glx, gly)) {
				bhelp.isAnimating = true;
				xscreen = 0;
				fromXScreen = 0;
				toXScreen = 0;
				yscreen = -0.5;
				fromYScreen = 0;
				toYScreen = -1;
			}
			else if (bexit.collision(glx, gly)) {
				bexit.isAnimating = true;
			}
		}
		if (xscreen == 1 && yscreen == 0) {
			if (bback10.collision(glx, gly)) {
				bback10.isAnimating = true;
				xscreen = 0.5; //check later
				fromXScreen = 1;
				toXScreen = 0;
				yscreen = 0;
				fromYScreen = 0;
				toYScreen = 0;
			}

			if (bsolve.collision(glx, gly)) {
				bsolve.isAnimating = true;
			}
		}

		if (xscreen == 0 && yscreen == -1) {
			if (bback0_1.collision(glx, gly)) {
				bback0_1.isAnimating = true;
				xscreen = 0;
				fromXScreen = 0;
				toXScreen = 0;
				yscreen = -0.5;
				fromYScreen = -1;
				toYScreen = 0;
			}
		}
	}


	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		px = x;
		py = y;
		isMousePressed = false;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		py = px = -1;
		isMousePressed = false;
	}



}

void motion(int x, int y) {

	if (xscreen == 1 && yscreen == 0) {
		if (px != -1 && py != -1) {
			double factor = 0.005;
			Quaterniond qx = Quaterniond{ AngleAxisd{ (y - py)*factor, Vector3d{ 1, 0, 0 } } };
			Quaterniond qy = Quaterniond{ AngleAxisd{ (x - px)*factor, Vector3d{ 0, 1, 0 } } };
			camera = qx * qy*camera;
		}
	}
	px = x;
	py = y;

}
//handles change in windows size
void reshape(int w, int h) {

	double widthScale = (double)w / width, heightScale = (double)h / height;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-viewportX / 2 * widthScale, viewportX / 2 * widthScale, -viewportY / 2 * heightScale, viewportY / 2 * heightScale, -5, 5);
	glMatrixMode(GL_MODELVIEW);
}

// used to draw a particular face
void drawFace(float *a, float *b, float *c, float *d, int face) {

	glColor3fv(colorList[face].getArray());
	glBegin(GL_QUADS);
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);
	glVertex3fv(d);
	glEnd();

}

void drawCube(float *a, float *b, float *c, float *d,
	float *e, float *f, float *g, float *h,
	int x, int y, int z) {

	int front = 6, back = 6, left = 6, right = 6, up = 6, down = 6;

	/*
	0 - Front
	1 - Back
	2 - Left
	3 - Right
	4 - Up
	5 - Down
	*/

	if (z == 0)
		front = cube1.faces[0][y][x];
	if (z == CUBE_SIZE - 1)
		back = cube1.faces[1][y][CUBE_SIZE - 1 - x];

	if (y == 0)
		up = cube1.faces[4][CUBE_SIZE - 1 - z][x];
	if (y == CUBE_SIZE - 1)
		down = cube1.faces[5][z][x];
	if (x == 0)
		left = cube1.faces[2][y][CUBE_SIZE - 1 - z];
	if (x == CUBE_SIZE - 1)
		right = cube1.faces[3][y][z];
	if (!hollow || front != 6)
		drawFace(a, b, c, d, front);	// Front
	if (!hollow || back != 6)
		drawFace(f, e, h, g, back);		// Back
	if (!hollow || left != 6)
		drawFace(e, a, d, h, left);		// Left
	if (!hollow || right != 6)
		drawFace(b, f, g, c, right);	// Right
	if (!hollow || up != 6)
		drawFace(e, f, b, a, up);		// Up
	if (!hollow || down != 6)
		drawFace(d, c, g, h, down);		// Down

}

void buildRubiksCube() {

	double big_szie = 2.4; // decides the size of the cube
	double small_size = 0.8; //small_size = 0.8
	double intercube_spacing = small_size * 0.05;

	double start = big_szie / 2 + intercube_spacing * (CUBE_SIZE - 1) / 2;
	// the following for loops draw the cube
	for (double z = start; z > -start; z -= small_size + intercube_spacing) {
		for (double y = start; y > -start; y -= small_size + intercube_spacing) {
			for (double x = -start; x < start; x += small_size + intercube_spacing) {

				int cx = (int)round((x + big_szie / 2) / (small_size + intercube_spacing));
				int cy = (int)round((-y + big_szie / 2) / (small_size + intercube_spacing));
				int cz = (int)round((-z + big_szie / 2) / (small_size + intercube_spacing));

				glPushMatrix();
				glTranslatef(8, 0, 0); // pushes the matrix out of the current
				glMultMatrixd(getRotationMatrix(cubesRotation[cz][cy][cx]));
				glTranslatef(-8, 0, 0);
				float v[8][3] = {
					{ x + 8, y, z },
					{ x + small_size + 8, y, z },
					{ x + small_size + 8, y - small_size, z },
					{ x + 8, y - small_size, z },
					{ x + 8, y, z - small_size },
					{ x + small_size + 8, y, z - small_size },
					{ x + small_size + 8, y - small_size, z - small_size },
					{ x + 8, y - small_size, z - small_size }
				};
				drawCube(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], cx, cy, cz);
				glPopMatrix();
			}
		}
	}


}

void updateRotation() {

	REP(a, rotationQueue.size()) {
		point mci = rotationQueue[a];
		int i = mci.z, j = mci.y, k = mci.x;
		Quaterniond qr = Quaterniond{ AngleAxisd{ degtorad(rorationSpeed), rotationAxis} };
		cubesRotation[i][j][k] = qr * cubesRotation[i][j][k];
	} // rotates for rotationspeed[0.3]

	totalRotation += rorationSpeed;

	if (totalRotation >= 90 || totalRotation <= -90) {
		changeState();
	}

}

// handles page transitions
void doTransition(float fromX, float toX, float fromY, float toY) {

	if (isTransition) {

		if ((ceil(xscreen) == 1 || floor(xscreen) == 1) &&
			(ceil(yscreen) == 0 || floor(yscreen) == 0)) {
			cameraX += transitionSpeed * (toX - fromX)*(8);
			cameraY += transitionSpeed * (toY - fromY)*(5);
			transition_percent += transitionSpeed;

			if (transition_percent >= 1) {
				isTransition = false;
				fromXScreen = xscreen = toXScreen;
				fromYScreen = yscreen = toYScreen;
				transition_percent = 0;
			}
		}
		else {
			cameraX += 0.001*(toX - fromX)*(8);
			cameraY += 0.001*(toY - fromY)*(5);
			transition_percent += 0.001;

			if (transition_percent >= 1) {
				isTransition = false;
				fromXScreen = xscreen = toXScreen;
				fromYScreen = yscreen = toYScreen;
				transition_percent = 0;
			}
		}
	}

	glTranslatef(-cameraX, -cameraY, -cameraZ);
}

//Displays the header "RUBIKS"
void displayTitle() {

	printText(-2, 1.5, "R", 5, GLUT_STROKE_MONO_ROMAN, colorList[5], 4);
	printText(-1.4, 1.5, "U", 5, GLUT_STROKE_MONO_ROMAN, colorList[4], 4);
	printText(-0.8, 1.5, "B", 5, GLUT_STROKE_MONO_ROMAN, colorList[2], 4);
	printText(-0.2, 1.5, "I", 5, GLUT_STROKE_MONO_ROMAN, colorList[0], 4);
	printText(0.4, 1.5, "K", 5, GLUT_STROKE_MONO_ROMAN, colorList[1], 4);
	printText(1, 1.5, "S", 5, GLUT_STROKE_MONO_ROMAN, colorList[3], 4);

}

// To display the contents of help
void displayHelp() {

	printText(-3.8, -3.2, "INSTRUCTIONS", 3, GLUT_STROKE_ROMAN, color(0, 0.6, 1), 3);
	printText(-3.5, -3.55, "1. KEYS :", 1.5, GLUT_STROKE_ROMAN, color(0.8, 0.8, 0.8), 2.5);
	printText(-3.2, -3.8, "Q : Front clockwise", 1, GLUT_STROKE_ROMAN, colorList[0], 1);
	printText(-3.2, -4.0, "W : Back clockwise", 1, GLUT_STROKE_ROMAN, colorList[1], 1);
	printText(-3.2, -4.2, "A : Left clockwise", 1, GLUT_STROKE_ROMAN, colorList[2], 1);
	printText(-3.2, -4.4, "S : Right clockwise", 1, GLUT_STROKE_ROMAN, colorList[3], 1);
	printText(-3.2, -4.6, "Z : Top clockwise", 1, GLUT_STROKE_ROMAN, colorList[4], 1);
	printText(-3.2, -4.8, "X : Bottom clockwise", 1, GLUT_STROKE_ROMAN, colorList[5], 1);
	printText(-3.2, -5.0, "SHIFT : Anti-clockwise Turns", 1, GLUT_STROKE_ROMAN, colorList[3], 1);

	printText(-3.5, -5.5, "2. MOUSE", 1.5, GLUT_STROKE_ROMAN, color(0.8, 0.8, 0.8), 2.5);
	printText(-3.2, -5.8, "Drag Vertically : Rotate along X-Axis", 1, GLUT_STROKE_ROMAN, colorList[5], 1);
	printText(-3.2, -6.0, "Drag Horizontally : Rotate along Y-Axis", 1, GLUT_STROKE_ROMAN, colorList[0], 1);
	printText(-3.2, -6.5, "DEVELOPED BY:", 1.1, GLUT_STROKE_MONO_ROMAN, colorList[2], 2);
	printText(-2.5, -6.8, "SIDDHANTH B N & SUPREETH B S", 1, GLUT_STROKE_MONO_ROMAN, colorList[4], 2);


}


//Prints solution when solve is clicked if empty prints nothing
void displaySolution() {

	REP(i, moveList.size()) {
		string s = "";
		s += moveList[i];
		printText(11, 2 - i * 0.3, s, 1.5, GLUT_STROKE_ROMAN, colorList[3], 2);
	}

}

void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	animateButtons();
	doTransition(fromXScreen, toXScreen, fromYScreen, toYScreen);

	if ((ceil(xscreen) == 0 || floor(xscreen) == 0) &&
		(ceil(yscreen) == 0 || floor(yscreen) == 0)) {
		bstart.draw();
		bhelp.draw();
		bexit.draw();
		displayTitle();
	}


	if ((ceil(xscreen) == 0 || floor(xscreen) == 0) &&
		(ceil(yscreen) == -1 || floor(yscreen) == -1)) {
		bback0_1.draw();
		displayHelp();
	}
	if ((ceil(xscreen) == 1 || floor(xscreen) == 1) &&
		(ceil(yscreen) == 0 || floor(yscreen) == 0)) {

		bback10.draw();
		bsolve.draw();

		displaySolution();

		glTranslatef(8, 0, 0); // used to place the cube
		glMultMatrixd(getRotationMatrix(camera));  // handles viewing of the cube
		glTranslatef(-8, 0, 0);
		buildRubiksCube(); //builds the cube

		if (rotationType) {
			updateRotation();
		}
	}

	glutSwapBuffers();
	glutPostRedisplay();

}

void init() {


	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	REP(i, CUBE_SIZE) {
		REP(j, CUBE_SIZE) {
			REP(k, CUBE_SIZE) {
				cubesRotation[i][j][k] = Quaterniond{ AngleAxisd{1,Vector3d{0,0,0}} };
			}
		}
	}

}

//Entry point
int main(int argc, char *argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Rubics Cube");

	init();

	glEnable(GL_DEPTH_TEST);
	glLoadIdentity();
	glutMainLoop();
	return 0;

}
