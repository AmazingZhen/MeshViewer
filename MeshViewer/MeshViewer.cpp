// MeshViewer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "FileReading.h"
#include <iostream>
#include <vector>

//// ------------------- OpenMesh  
#include <OpenMesh\Core\IO\MeshIO.hh>  
#include <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>  

//// --------------------OpenGL  
#include <GL\glut.h>  

#define FILE_FOLDER "dataset\\"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500

using namespace std;
typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

vector<MyMesh> mesh;
vector<GLuint> flatList, wireframeList, flatLinesList;

vector<GLfloat> xRotate;
vector<GLfloat> yRotate;
vector<GLfloat> oldXPos, oldYPos;

vector<GLfloat> xOffset;
vector<GLfloat> yOffset;

vector<GLfloat> scale;

int mode = 1;

int chosenIndex = 0;
int indexSize;

//   
void initGL() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	// Lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	for (int i = 0; i < mesh.size(); i++) {
		// Initial display list  
		flatList[i] = glGenLists(1);
		wireframeList[i] = glGenLists(1);
		flatLinesList[i] = glGenLists(1);

		// Show wire frame  
		glNewList(wireframeList[i], GL_COMPILE);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		glColor3f(0.2f, 0.2f, 0.2f);
		glBegin(GL_LINES);
		for (MyMesh::HalfedgeIter he_it = mesh[i].halfedges_begin(); he_it != mesh[i].halfedges_end(); ++he_it) {
			glVertex3fv(mesh[i].point(mesh[i].from_vertex_handle(*he_it)).data());
			glVertex3fv(mesh[i].point(mesh[i].to_vertex_handle(*he_it)).data());
		}
		glEnd();
		glEnable(GL_LIGHTING);
		glEndList();

		// Show flat  
		glNewList(flatList[i], GL_COMPILE);
		for (MyMesh::FaceIter f_it = mesh[i].faces_begin(); f_it != mesh[i].faces_end(); ++f_it) {
			glBegin(GL_TRIANGLES);
			for (MyMesh::FaceVertexIter fv_it = mesh[i].fv_iter(*f_it); fv_it.is_valid(); ++fv_it) {
				// Using vertex normal.
				glNormal3fv(mesh[i].normal(*fv_it).data());
				glVertex3fv(mesh[i].point(*fv_it).data());
			}
			glEnd();
		}
		glEndList();

		// Show flat lines
		glNewList(flatLinesList[i], GL_COMPILE);
		glCallList(wireframeList[i]);

		for (MyMesh::FaceIter f_it = mesh[i].faces_begin(); f_it != mesh[i].faces_end(); ++f_it) {
			glBegin(GL_TRIANGLES);
			for (MyMesh::FaceVertexIter fv_it = mesh[i].fv_iter(*f_it); fv_it.is_valid(); ++fv_it) {
				// Using face normal.
				glNormal3fv(mesh[i].normal(*f_it).data());
				glVertex3fv(mesh[i].point(*fv_it).data());
			}
			glEnd();
		}
		glEndList();

	}

}
//  
void myReshape(GLint w, GLint h)
{
	glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (w > h)
		glOrtho(-static_cast<GLdouble>(w) / h, static_cast<GLdouble>(w) / h, -1.0, 1.0, -50.0f, 50.0f);
	else
		glOrtho(-1.0, 1.0, -static_cast<GLdouble>(h) / w, static_cast<GLdouble>(h) / w, -50.0f, 50.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}
//   
void mySpecial(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_RIGHT:
		chosenIndex++;

		if (chosenIndex == mesh.size()) {
			chosenIndex = 0;
		}
		break;
	case GLUT_KEY_LEFT:
		chosenIndex--;

		if (chosenIndex == -1) {
			chosenIndex = mesh.size() - 1;
		}
		break;
	case GLUT_KEY_DOWN:
		scale[chosenIndex] *= 0.9;
		break;
	case GLUT_KEY_UP:
		scale[chosenIndex] /= 0.9;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
//  
void myKeyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		mode = 1;
		break;
	case '2':
		mode  = 2;
		break;
	case '3':
		mode = 3;
		break;
	case 'W':
		yOffset[chosenIndex] += 0.1f;
		break;
	case 'S':
		yOffset[chosenIndex] -= 0.1f;
		break;
	case 'A':
		xOffset[chosenIndex] -= 0.1f;
		break;
	case 'D':
		xOffset[chosenIndex] += 0.1f;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		oldXPos[chosenIndex] = x;
		oldYPos[chosenIndex] = y;
	}
}

void myMotion(int x, int y)
{
	GLint deltax = oldXPos[chosenIndex] - x;
	GLint deltay = oldYPos[chosenIndex] - y;
	yRotate[chosenIndex] -= 360 * (GLfloat)deltax / (GLfloat)WINDOW_WIDTH;
	xRotate[chosenIndex] -= 360 * (GLfloat)deltay / (GLfloat)WINDOW_HEIGHT;
	oldXPos[chosenIndex] = x;
	oldYPos[chosenIndex] = y;
	glutPostRedisplay();
}

//   
void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(xOffset[chosenIndex], yOffset[chosenIndex], 0);
	glRotatef(xRotate[chosenIndex], 1.0f, 0.0f, 0.0f);
	glRotatef(yRotate[chosenIndex], 0.0f, 1.0f, 0.0f);
	glScalef(scale[chosenIndex], scale[chosenIndex], scale[chosenIndex]);

	if (mode == 1) {
		glCallList(flatList[chosenIndex]);
	}
	else if (mode == 2) {
		glCallList(wireframeList[chosenIndex]);
	}
	else if (mode == 3) {
		glCallList(flatLinesList[chosenIndex]);
	}

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	string file_folder(FILE_FOLDER);
	vector<string> mesh_files;

	getAllFiles(file_folder, mesh_files);
	mesh = vector<MyMesh>(mesh_files.size());

	flatList = vector<GLuint>(mesh_files.size());
	wireframeList = vector<GLuint>(mesh_files.size());
	flatLinesList = vector<GLuint>(mesh_files.size());

	xRotate = vector<GLfloat>(mesh_files.size(), 0.0f);
	yRotate = vector<GLfloat>(mesh_files.size(), 0.0f);
	oldXPos = vector<GLfloat>(mesh_files.size());
	oldYPos = vector<GLfloat>(mesh_files.size());;

	xOffset = vector<GLfloat>(mesh_files.size(), 0.0f);
	yOffset = vector<GLfloat>(mesh_files.size(), 0.0f);
	scale = vector<GLfloat>(mesh_files.size(), 1.0f);

	for (int i = 0; i < mesh_files.size(); i++) {
		// request vertex normals, so the mesh reader can use normal information  
		// if available  
		mesh[i].request_vertex_normals();
		// assure we have vertex normals  
		if (!mesh[i].has_vertex_normals())
		{
			std::cerr << "ERROR: Standard vertex property 'Normals' not available!\n";
			return 1;
		}

		// Read mesh 
		OpenMesh::IO::Options opt;
		if (!OpenMesh::IO::read_mesh(mesh[i], mesh_files[i], opt))
		{
			cout << "Error: Cannot read mesh from " << mesh_files[i] << endl;
			return 1;
		}

		// If the file did not provide vertex normals, then calculate them  
		if (!opt.check(OpenMesh::IO::Options::VertexNormal))
		{
			// we need face normals to update the vertex normals  
			mesh[i].request_face_normals();

			// let the mesh update the normals  
			mesh[i].update_normals();

			// dispose the face normals, as we don't need them anymore  
			// mesh[i].release_face_normals();
		}

	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Mesh Viewer");
	initGL();
	glutKeyboardFunc(&myKeyboard);
	glutSpecialFunc(&mySpecial);
	glutReshapeFunc(&myReshape);
	glutDisplayFunc(&myDisplay);
	glutMouseFunc(&myMouse);
	glutMotionFunc(&myMotion);
	glutMainLoop();
	return 0;
}