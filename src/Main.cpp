//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury (2018)
//
//  Uses the Assimp and DevIL libraries.
//  ========================================================================

#include <iostream>
#include <map>
#include <GL/freeglut.h>
#include <IL/il.h>
using namespace std;

#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_extras.h"

#include "Utils.hpp"
#include "Task1.cpp"
#include "Task2.cpp"
#include "Task3.cpp"

enum Task
{
	T1,
	T2,
	T3,
};

struct Main
{
	Task1 task1;
	Task2 task2;
	Task3 task3;
	Task currentTask = T2;
};

Main state;

void init()
{
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Model Loader");
	glutInitContextVersion(4, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	//--------------------OpenGL initialization------------------------
	glClearColor(0.7f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1.0, 100000.0);

	state.task1.init();
	state.task2.init();
	state.task3.init();
}

void update(int millisSinceStart)
{
	switch (state.currentTask)
	{
	case T1:
		state.task1.update(millisSinceStart);
		break;
	case T2:
		state.task2.update(millisSinceStart);
		break;
	case T3:
		state.task3.update(millisSinceStart);
		break;
	}
	glutPostRedisplay();
	glutTimerFunc(10, update, millisSinceStart + 12);
}

//----Keyboard callback to toggle initial model orientation---
void keyboard(unsigned char key, int x, int y)
{
	if (key == '1')
	{
		state.currentTask = T1;
	}
	if (key == '2')
	{
		state.currentTask = T2;
	}
	if (key == '3')
	{
		state.currentTask = T3;
	}

	state.task2.keyboard(key);
	state.task3.keyboard(key);

	glutPostRedisplay();
}

void cleanup()
{
	state.task1.cleanup();
	state.task2.cleanup();
	state.task3.cleanup();
}

//------The main display function---------
//----The model is first drawn using a display list so that all GL commands are
//    stored for subsequent display updates.
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	switch (state.currentTask)
	{
	case T1:
		state.task1.display();
		break;
	case T2:
		state.task2.display();
		break;
	case T3:
		state.task3.display();
		break;
	}

	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	init();

	glutDisplayFunc(display);
	glutTimerFunc(50, update, 50);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	cleanup();
}
