#include "PCH.h"
#include "Shader.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "DataTable.h"
#include "UIRenderer.h"
#include "TextRenderer.h"

void main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_W, WINDOW_H);
	glutInitWindowPosition(WINDOW_W,200);
	glutCreateWindow("MiniFarm");

	glewInit();
	glEnable(GL_DEPTH_TEST);

	DataTable::Init();
	TextRenderer::Init();
	UIRenderer::Init();
	InputManager::Init();

	SceneManager::Init();

	glutKeyboardFunc(InputManager::KeyDown);
	glutKeyboardUpFunc(InputManager::KeyUp);

	glutMouseFunc(InputManager::MouseButton);
	glutPassiveMotionFunc(InputManager::MouseMove);
	glutDisplayFunc(SceneManager::Draw);
	glutReshapeFunc(SceneManager::Reshape);

	glutTimerFunc(FRAME_TIME_MS, SceneManager::Update, 0);

	glutMainLoop();
}
