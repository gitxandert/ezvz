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

	extern glm::vec2 getClickWorld(const ImGuiIO&);

	extern glm::mat4 projFullScreen;

	void init(int width, int height);
	extern GLuint fbo, colorTex, depthRbo;
	void render();

	inline std::shared_ptr<GraphicObject> selectedObject;

	inline void setSelected(std::shared_ptr<GraphicObject> obj) {
		selectedObject = obj;
	}

	inline void clearSelected() {
		selectedObject = nullptr;
	}

	inline bool isDraggingObject = false;

	extern bool clicked;

	void shutdown();
	void recreate(int newW, int newH);
	void onResize(int canvasW, int canvasH);
}