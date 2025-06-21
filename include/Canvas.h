#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Shader.h"

namespace Canvas {
	glm::vec3 c_center();

	extern std::unique_ptr<Shader> shader;

	extern float x, y, w, h, screenW, screenH;

	extern int currentW, currentH;

	extern float padding;

	extern ImVec2 cm, sz;
	extern ImVec2 fboDrawPos;
	extern float fboDrawW, fboDrawH;

	extern glm::mat4 view;
	extern glm::vec3 cameraEye;

	extern glm::vec2 getClickWorld(const glm::vec2& screenPos);
	extern glm::vec2 worldToScreen(glm::vec3 worldPos, const ImVec2& screenOrigin, const ImVec2& screenSize);

	extern glm::mat4 projFullScreen;

	void init(int width, int height);
	extern GLuint fbo, colorTex, depthRbo;
	void render();

	extern std::shared_ptr<GraphicObject> selectedObject;

	inline void setSelected(std::shared_ptr<GraphicObject> obj) {
		selectedObject = obj;
	}

	inline void clearSelected() {
		selectedObject.reset();
	}

	extern bool isDraggingObject;

	extern bool clicked;

	void shutdown();
	void recreate(int newW, int newH);
	void onResize(int canvasW, int canvasH);
}