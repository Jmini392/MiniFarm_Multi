#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <chrono>
#include <array>
#include <memory>
#include <unordered_map>
#include <functional>
#include <format>

#include "FileUtils.h"
#include "MathUtils.h"
#include "Logger.h"

constexpr GLint WINDOW_W = 500;
constexpr GLint WINDOW_H = 500;

constexpr int FPS = 60;
constexpr int FRAME_TIME_MS = 1000 / FPS;

constexpr int GOAL_MONEY = 10000;