#include "header/VM.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/freeglut.h>


/*
*/

extern "C"{

enum KeyState{
	PRESS,
	RELEASE,
	HOLD,
	OFF
};

struct Input{

	KeyState keys[255];

	KeyState mouseLeft;
	KeyState mouseMiddle;
	KeyState mouseRight;

	int mouseX;
	int mouseY;

	void PushKey(unsigned char key);
	void ReleaseKey(unsigned char key);

	void PushMouse(int button);
	void ReleaseMouse(int button);

	void EndFrame();

	Input();

	bool GetKey(unsigned char key) const;
	bool GetKeyUp(unsigned char key) const;
	bool GetKeyDown(unsigned char key) const;

	bool GetMouse(int button) const;
	bool GetMouseUp(int button) const;
	bool GetMouseDown(int button) const;

	int inline GetMouseX() const{ return mouseX; }
	int inline GetMouseY() const{ return mouseY; }
};

Input::Input(){
	for(int i = 0; i < 256; i++){
		keys[i] = KeyState::OFF;
	}
}

void Input::PushKey(unsigned char key){
	keys[key] = KeyState::PRESS;
}

void Input::ReleaseKey(unsigned char key){
	keys[key] = KeyState::RELEASE;
}

void Input::PushMouse(int button){
	if(button == GLUT_LEFT_BUTTON)  { mouseLeft   = KeyState::PRESS;}
	if(button == GLUT_RIGHT_BUTTON) { mouseRight  = KeyState::PRESS;}
	if(button == GLUT_MIDDLE_BUTTON){ mouseMiddle = KeyState::PRESS;}
}

void Input::ReleaseMouse(int button){
	if(button == GLUT_LEFT_BUTTON)  { mouseLeft   = KeyState::RELEASE;}
	if(button == GLUT_RIGHT_BUTTON) { mouseRight  = KeyState::RELEASE;}
	if(button == GLUT_MIDDLE_BUTTON){ mouseMiddle = KeyState::RELEASE;}
}

bool Input::GetKey(unsigned char key) const{
	return keys[key] == KeyState::PRESS || keys[key] == KeyState::HOLD;
}

bool Input::GetKeyUp(unsigned char key) const{
	return keys[key] == KeyState::RELEASE;
}

bool Input::GetKeyDown(unsigned char key) const{
	return keys[key] == KeyState::PRESS;
}

bool Input::GetMouse(int button) const{
	if(button == GLUT_LEFT_BUTTON)  { return mouseLeft   == KeyState::PRESS || mouseLeft   == KeyState::HOLD;}
	if(button == GLUT_RIGHT_BUTTON) { return mouseRight  == KeyState::PRESS || mouseRight  == KeyState::HOLD;}
	if(button == GLUT_MIDDLE_BUTTON){ return mouseMiddle == KeyState::PRESS || mouseMiddle == KeyState::HOLD;}
	return false;
}

bool Input::GetMouseUp(int button) const{
	if(button == GLUT_LEFT_BUTTON)  { return mouseLeft   == KeyState::RELEASE;}
	if(button == GLUT_RIGHT_BUTTON) { return mouseRight  == KeyState::RELEASE;}
	if(button == GLUT_MIDDLE_BUTTON){ return mouseMiddle == KeyState::RELEASE;}
	return false;
}

bool Input::GetMouseDown(int button) const{
	if(button == GLUT_LEFT_BUTTON)  { return mouseLeft   == KeyState::PRESS;}
	if(button == GLUT_RIGHT_BUTTON) { return mouseRight  == KeyState::PRESS;}
	if(button == GLUT_MIDDLE_BUTTON){ return mouseMiddle == KeyState::PRESS;}
	return false;
}

void Input::EndFrame(){
	for(int i = 0; i < 256; i++){
		if(keys[i] == KeyState::PRESS){
			keys[i] = KeyState::HOLD;
		}
		if(keys[i] == KeyState::RELEASE){
			keys[i] = KeyState::OFF;
		}
	}

	if(mouseLeft == KeyState::PRESS)  { mouseLeft = KeyState::HOLD;}
	if(mouseLeft == KeyState::RELEASE){ mouseLeft = KeyState::OFF; }

	if(mouseRight == KeyState::PRESS)  { mouseRight = KeyState::HOLD;}
	if(mouseRight == KeyState::RELEASE){ mouseRight = KeyState::OFF; }

	if(mouseMiddle == KeyState::PRESS)  { mouseMiddle = KeyState::HOLD;}
	if(mouseMiddle == KeyState::RELEASE){ mouseMiddle = KeyState::OFF; }
}

Input input;

void OnKeyFunc(unsigned char key, int x, int y){
	input.PushKey(key);
}

void OnKeyUpFunc(unsigned char key, int x, int y){
	input.ReleaseKey(key);
}


VMValue Init(VMValue init){
	input = Input();

	int x = 0;
	char* argv[3] = {"a", "b", "v"};
	glutInit(&x, argv);

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_ALPHA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(300, 100);
    glutCreateWindow("svm-GL");

	glutKeyboardFunc(OnKeyFunc);
	glutKeyboardUpFunc(OnKeyUpFunc);
	
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	VMValue ret;
	ret.intValue = 0;
	ret.type = ValueType::INT;
	return ret;
}

VMValue FrameStart(VMValue a){
	glutMainLoopEvent();
	glClear(GL_COLOR_BUFFER_BIT);

	VMValue ret;
	ret.intValue = 0;
	ret.type = ValueType::INT;
	return ret;
}

VMValue FrameEnd(VMValue a){
	glutPostRedisplay();
	glutSwapBuffers();
	input.EndFrame();

	VMValue ret;
	ret.intValue = 0;
	ret.type = ValueType::INT;
	return ret;
}

VMValue Draw(VMValue a){
	//glBegin(GL_POINT);
	//glVertex2f(a.intValue / 500, a.intValue % 500);
	//glEnd();

	glClearColor(1.0f, 1.0f, a.floatValue, 1.0f);

	VMValue ret;
	ret.intValue = 0;
	ret.type = ValueType::INT;
	return ret;
}

VMValue GetInput(VMValue a){
	VMValue ret;
	ret.intValue = (input.GetKeyUp(a.intValue) ? 1 : 0);
	ret.type = ValueType::INT;
	return ret;
}


}