#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include<irrKlang.h>
#include <GL/glew.h>

#define GLM_FORCE_CTOR_INIT 
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector> 

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Camera.h"
#include "Shader.h"
#include "MoveableObject.h"
#include "Terrain.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "irrKlang.lib")

using namespace irrklang;
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundEngine* TrainSoundEngine = createIrrKlangDevice();
bool isInside = false;
void SetOutsideSound(bool day)
{
	SoundEngine->removeAllSoundSources();
	if (day)
	{
		SoundEngine->play2D("Assets\\Audio\\Day.wav", true);
		SoundEngine->setSoundVolume(0.8);
	}
	else
	{
		SoundEngine->play2D("Resources/audio/night.mp3", true);
		SoundEngine->setSoundVolume(0.4);
	}
}

void setTrainSound(bool move) {
	TrainSoundEngine->removeAllSoundSources();
	if (move) {
		TrainSoundEngine->play2D("Assets\\Audio\\train.wav", true);
		TrainSoundEngine->setSoundVolume(0.9);
	}
	else {
		TrainSoundEngine->removeAllSoundSources();
		TrainSoundEngine->setAllSoundsPaused();
	}
}

const unsigned int SCR_WIDTH = 1800;
const unsigned int SCR_HEIGHT = 1080;
bool isDayTime = false;

const float maxVisibilityDistance = 50.0f;

Camera* pCamera = nullptr;
bool freeCameraView = true;

MoveableObject* currentObject;
Terrain terrain;

unsigned int CreateTexture(const std::string& strTexturePath)
{
	unsigned int textureId = -1;

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format{};
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load texture: " << strTexturePath << std::endl;
	}
	stbi_image_free(data);

	return textureId;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


void renderScene(const Shader& shader);
void renderModel(Shader& ourShader, Model& ourModel, const glm::vec3& position, float rotationAngle, const glm::vec3& scale);


double deltaTime = 0.0f;
double lastFrame = 0.0f;

float skyboxVertices[] = {
	-1.0f,  -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	 1.0f, -1.0f, -1.0f,
	 -1.0f, -1.0f, -1.0f,
	 -1.0f, 1.0f, 1.0f,
	 1.0f,  1.0f, 1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	1,2,6,
	6,5,1,
	0,4,7,
	7,3,0,
	4,5,6,
	6,7,4,
	0,3,2,
	2,1,0,
	0,1,5,
	5,4,0,
	3,7,6,
	6,2,3
};

std::vector<glm::vec3> railPositions =
{

	glm::vec3(0.0f, -1.5f, 0.0f),
	glm::vec3(0.0f, -1.5f, 60.0f),
	glm::vec3(0.0f, -1.5f, 120.0f),
	glm::vec3(0.0f, -1.5f, 180.0f),
	glm::vec3(0.0f, -1.5f, -60.0f),
	glm::vec3(0.0f, -1.5f, -120.0f),
	glm::vec3(0.0f, -1.5f, -180.0f),
};

std::vector<glm::vec3> forestTreePositions =
{
	glm::vec3(-40.0f, -1.5f, 50.0f),
	glm::vec3(40.0f, -1.5f, 50.0f),
	glm::vec3(40.0f, -1.5f, -10.0f),
	glm::vec3(-40.0f, -1.5f, -10.0f),

	glm::vec3(-40.0f, -1.5f, -70.0f),
	glm::vec3(40.0f, -1.5f, -70.0f),
	glm::vec3(-40.0f, -1.5f, -140.0f),
	glm::vec3(40.0f, -1.5f, -140.0f),
};

std::vector<glm::vec3> decorPositions =
{
	glm::vec3(-35.0f, -1.5f, -110.0f),
	glm::vec3(55.0f, -1.5f, -110.0f),
	glm::vec3(-20.0f, -1.5f, -140.0f),
	glm::vec3(20.0f, -1.5f, -140.0f),

	glm::vec3(-20.0f, -1.5f, 50.0f),
	glm::vec3(50.0f, -1.5f, 50.0f),
	glm::vec3(-30.0f, -1.5f, -70.0f),
	glm::vec3(90.0f, -1.5f, -70.0f),
};

std::vector<glm::vec3> mountainsPositions =
{
	glm::vec3(85.0f, -1.55f, -35.0f),
	glm::vec3(-70.0f, -1.55f, -100.0f),
};

std::vector<glm::vec3> mountainsScales =
{
	glm::vec3(0.7f),
	glm::vec3(0.6f),
};

float scaleFactor = 2.0f;

Model railModel, trainModel, haystackModel, mountainModel, stationModel, benchModel, humanModel, treesModel, forestModel, BucurestiModel, PloiestiModel, SinaiaModel, BrasovModel;
MoveableObject trainVehicle, railVehicle;

bool night = false;

std::vector<std::string> facesDay
{
	"skybox_images\\px.png",
	"skybox_images\\nx.png",
	"skybox_images\\py.png",
	"skybox_images\\ny.png",
	"skybox_images\\pz.png",
	"skybox_images\\nz.png",
};

std::vector<std::string>facesNight
{

	"skybox_images_night\\px_2.png",
	"skybox_images_night\\nx_2.png",
	"skybox_images_night\\py_2.png",
	"skybox_images_night\\ny_2.png",
	"skybox_images_night\\pz_2.png",
	"skybox_images_night\\nz_2.png"
};


float blendFactor = 0;
float ambientFactor = 0.9;

float trainX = 0.0f;
float trainZ = 0.0f;

int stationCount = 0;

unsigned int cubemapTexture;
unsigned int cubemapTextureNight;

int main(int argc, char** argv)
{
	SetOutsideSound(true);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	std::string strFullExeFileName = argv[0];
	std::string strExePath;
	const size_t last_slash_idx = strFullExeFileName.rfind('\\');
	if (std::string::npos != last_slash_idx)
	{
		strExePath = strFullExeFileName.substr(0, last_slash_idx);
	}

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CFR_Calatori", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glewInit();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 7.0f, 25.0f));
	glEnable(GL_DEPTH_TEST);


	Shader shadowMappingShader("ShadowMapping.vs", "ShadowMapping.fs");
	Shader shadowMappingDepthShader("ShadowMappingDepth.vs", "ShadowMappingDepth.fs");
	Shader ModelShader("ModelShader.vs", "ModelShader.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");

	unsigned int floorTexture = CreateTexture("skybox_images\\grass.jpg");
	unsigned int fieldTexture = CreateTexture("Assets\\Field\\field_texture.jpg");

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::vec3 fogColor(0.5f, 0.5f, 0.5f); // Example fog color, adjust as needed 
	// shader configuration
	shadowMappingShader.Use();
	shadowMappingShader.SetInt("diffuseTexture", 0);
	shadowMappingShader.SetInt("shadowMap", 1);
	shadowMappingShader.SetFloat("ambientFactor", 0.1f); // Adjust as needed
	shadowMappingShader.SetVec3("fogColor", fogColor);
	shadowMappingShader.SetFloat("fogStart", 75.0f);    // Distance at which fog starts
	shadowMappingShader.SetFloat("fogEnd", 150.0f);      // Distance at which fog completely obscures objects
	shadowMappingShader.SetInt("diffuseTexture", 0);
	shadowMappingShader.SetInt("shadowMap", 1);

	//lightPos over the train to follow it
	glm::vec3 lightPos(trainVehicle.GetPosition().x, 100.0f, trainVehicle.GetPosition().z);

	glEnable(GL_CULL_FACE);

	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < facesDay.size(); i++)
	{
		unsigned char* data = stbi_load(facesDay[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			glGetError();
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << facesDay[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glGenTextures(1, &cubemapTextureNight);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//night skybox
	for (unsigned int i = 0; i < facesNight.size(); i++) {
		unsigned char* data = stbi_load(facesNight[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			glGetError();
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << facesNight[i] << std::endl;
		}
	}

	//Terrain loading
	terrain.initialize(floorTexture, fieldTexture);
	float chunkBorder = -100.0f;
	bool hasStation = true;

	//Train model loading
	trainModel = Model("Assets\\Train\\electrictrain.obj");
	trainVehicle = MoveableObject(trainModel, SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainX, -1.25f, trainZ));

	//Rail model loading
	railModel = Model("Assets\\Rail\\rail.obj");

	//Decorations loading
	benchModel = Model("Assets\\Bench\\Bench_HighRes.obj");
	humanModel = Model("Assets\\Human\\Humano_01Business_01_30K.obj");
	treesModel = Model("Assets\\Forest\\station_trees.obj");

	forestModel = Model("Assets\\Forest\\mountain_forest.obj");
	mountainModel = Model("Assets\\Mountain\\mountain.obj");
	haystackModel = Model("Assets\\Field\\haystack.obj");


	//Station model loading
	BucurestiModel = Model("Assets\\TrainStations\\bucuresti_station.obj");
	PloiestiModel = Model("Assets\\TrainStations\\ploiesti_station.obj");
	SinaiaModel = Model("Assets\\TrainStations\\sinaia_station.obj");
	BrasovModel = Model("Assets\\TrainStations\\brasov_station.obj");

	std::vector<Model> stationModels = { BucurestiModel, PloiestiModel, SinaiaModel, BrasovModel };
	stationCount = 0;


	while (!glfwWindowShouldClose(window))
	{
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		double currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// Terrain update
		if (trainZ <= chunkBorder && trainZ >= chunkBorder - 1.0f)
		{
			terrain.addChunk();
			chunkBorder -= 200.0f;
			if (stationCount == 0)
				terrain.setTexture(floorTexture);

			switch (hasStation)
			{
			case true:
				hasStation = false;
				stationCount++;
				break;
			case false:
				hasStation = true;
				break;
			}
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;

		glm::mat4 lightRotationMatrix = glm::mat4(
			glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(sin(lastFrame), 0.0f, cos(lastFrame), 1.0f)
		);

		glm::vec4 rotatedLightPos = lightRotationMatrix * glm::vec4(lightPos, 1.0f);

		float near_plane = 1.0f, far_plane = 10.5f;
		lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		shadowMappingDepthShader.Use();
		shadowMappingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		renderScene(shadowMappingDepthShader);

		//---TRAIN VARIABLES---
		float trainRotation = -10.0f;
		glm::vec3 trainScale = glm::vec3(0.5f);

		//---RAILS VARIABLES---
		float railRotation = 0.0f;
		glm::vec3 railScale = glm::vec3(0.001f, 0.001f, 0.002f);

		//---DECOR VARIABLES---
		float decorRotation = 0.0f;
		glm::vec3 forestScale = glm::vec3(1);



		float mountainRotation = 0.0f;
		glm::vec3 mountainScale = glm::vec3(0.1f);
		for (int i = 0; i < mountainsPositions.size(); i++)
		{
			renderModel(shadowMappingDepthShader, mountainModel, mountainsPositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), mountainRotation, mountainsScales[i]);
		}

		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowMappingShader.Use();
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix(currentObject);
		shadowMappingShader.SetMat4("projection", projection);
		shadowMappingShader.SetMat4("view", view);
		shadowMappingShader.SetFloat("ambientFactor", ambientFactor);
		// set light uniforms
		shadowMappingShader.SetVec3("viewPos", pCamera->GetPosition());
		shadowMappingShader.SetVec3("lightPos", glm::vec3(trainVehicle.GetPosition().x, 100.f, trainVehicle.GetPosition().z));
		shadowMappingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDisable(GL_CULL_FACE);
		renderScene(shadowMappingShader);
		terrain.render();

		//-----TRAIN AND RAILS RENDERING------
		renderModel(ModelShader, trainVehicle.GetVehicleModel(), trainVehicle.GetPosition(), trainVehicle.GetRotation() - 1, trainScale);
		for (int i = 0; i < railPositions.size(); i++)
		{
			renderModel(ModelShader, railModel, railPositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 50.0f), railRotation, railScale);
		}

		//-----GENERATIVE RENDERING-----
		switch (hasStation)
		{
		case true:
			//Station
			if (stationCount < stationModels.size())
				renderModel(ModelShader, stationModels[stationCount], glm::vec3(5.0f, -1.5f, 10.0f) + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), 0.0f, glm::vec3(1.0f));

			//Bench
			renderModel(ModelShader, benchModel, glm::vec3(10.0f, -1.5f, 18.0f) + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), -90.0f, glm::vec3(0.01f));

			//Human
			renderModel(ModelShader, humanModel, glm::vec3(8.0f, -1.5f, 18.0f) + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), -90.0f, glm::vec3(0.01f));

			//Decor
			if (stationCount == 0)
			{
				for (int i = 0; i < decorPositions.size()/2; i++)
				{
					renderModel(ModelShader, haystackModel, decorPositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), decorRotation, glm::vec3(5.f));

					if (decorRotation <= 360.0f)
						decorRotation += 90.0f;
					else
						decorRotation = 0.0f;
				}

				for (int i = 0; i < forestTreePositions.size()/2; i++)
				{
					renderModel(ModelShader, treesModel, forestTreePositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), 0.0f, glm::vec3(2.5f));
				}
			}
			else
			{
				for (int i = 0; i < forestTreePositions.size(); i++)
				{
					renderModel(ModelShader, treesModel, forestTreePositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), decorRotation, glm::vec3(2.5f));

					if (decorRotation <= 360.0f)
						decorRotation += 90.0f;
					else
						decorRotation = 0.0f;
				}
			}

			break;

		case false:
			//Decor
			if (stationCount == 1)
			{
				//Haystack
				for (int i = 0; i < decorPositions.size(); i++)
				{
					renderModel(ModelShader, haystackModel, decorPositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), decorRotation, glm::vec3(5.f));

					if (decorRotation <= 360.0f)
						decorRotation += 90.0f;
					else
						decorRotation = 0.0f;
				}
			}
			else
			{
				//Forest
				for (int i = 0; i < forestTreePositions.size(); i++)
				{
					renderModel(ModelShader, forestModel, forestTreePositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), decorRotation, forestScale);

					if (decorRotation <= 360.0f)
						decorRotation += 90.0f;
					else
						decorRotation = 0.0f;
				}
				
				//Mountains
				for (int i = 0; i < mountainsPositions.size(); i++)
				{
					renderModel(ModelShader, mountainModel, mountainsPositions[i] + glm::vec3(0.0f, 0.0f, chunkBorder + 100.0f), mountainRotation, mountainsScales[i]);
				}
			}


			//Station from far
			if (stationCount < stationModels.size())
				renderModel(ModelShader, stationModels[stationCount], glm::vec3(5.0f, -1.5f, 10.0f) + glm::vec3(0.0f, 0.0f, chunkBorder - 100.0f), 0.0f, glm::vec3(1.0f));

			break;
		}

		glm::vec3 light = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 lightDir = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
		glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.31f);

		ModelShader.Use();
		ModelShader.SetVec3("lightColor", glm::vec3(1.0, 1.0, 1.0));
		ModelShader.SetVec3("lightDir", lightDir);
		ModelShader.SetVec3("objectColor", objectColor);

		glm::vec3 trainPosition = trainVehicle.GetPosition();
		ModelShader.SetVec3("trainPos", trainPosition);

		glm::vec3 fogColor(0.5f, 0.5f, 0.5f);
		ModelShader.SetVec3("fogColor", fogColor);
		ModelShader.SetFloat("fogStart", trainVehicle.GetPosition().x + 50.0f);    // Increased distance at which fog starts
		ModelShader.SetFloat("fogEnd", trainVehicle.GetPosition().x + 200.0f);
		ModelShader.SetFloat("exclusionRadius", 100.0f);
		ModelShader.SetInt("texture_diffuse1", 0);

		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		skyboxShader.Use();

		skyboxShader.SetInt("skybox1", 0);
		skyboxShader.SetInt("skybox2", 1);
		skyboxShader.SetFloat("blendFactor", blendFactor);

		glm::mat4 viewSB = glm::mat4(glm::mat3(pCamera->GetViewMatrix(currentObject)));

		glm::mat4 projectionSB = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		skyboxShader.SetMat4("view", viewSB);
		skyboxShader.SetMat4("projection", projectionSB);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);

		glBindVertexArray(skyboxVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete pCamera;
	glfwTerminate();
	return 0;
}

void renderScene(const Shader& shader)
{
	glm::mat4 model;
	float deltaY = -1.0f;
	model = glm::translate(model, glm::vec3(0.0f, deltaY, 0.0f));
	shader.SetMat4("model", model);
}

unsigned int modelVAO = 0;
unsigned int modelVBO = 0;
unsigned int modelEBO;

void renderModel(Shader& ourShader, Model& ourModel, const glm::vec3& position, float rotationAngle, const glm::vec3& scale)
{
	ourShader.Use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, scale);

	glm::mat4 viewMatrix = pCamera->GetViewMatrix(currentObject);
	glm::mat4 projectionMatrix = pCamera->GetProjectionMatrix();

	ourShader.SetMat4("model", model);
	ourShader.SetMat4("view", viewMatrix);
	ourShader.SetMat4("projection", projectionMatrix);

	ourModel.Draw(ourShader);
}

bool isTrainSoundPlaying = false;

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//-----------------NIGHT MODE-----------------
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		blendFactor = std::min(blendFactor + 0.01, 0.85);
		ambientFactor = std::max(ambientFactor - 0.01, 0.0);
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		blendFactor = std::max(blendFactor - 0.01, 0.0);
		ambientFactor = std::min(ambientFactor + 0.01, 1.0);
	}

	//-----------------TRAIN MOVEMENT-----------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			trainVehicle.setSpeed(20.5);
		trainVehicle.ProcessKeyboard(V_FORWARD, (float)deltaTime);
		if (!isTrainSoundPlaying)
		{
			setTrainSound(true);
			isTrainSoundPlaying = true;
		}
		trainZ = trainVehicle.GetPosition().z;
		if (freeCameraView == false)
			pCamera->set(SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainVehicle.GetPosition().x, trainVehicle.GetPosition().y + 6, trainVehicle.GetPosition().z));
		trainVehicle.setSpeed(12.5);
		if (freeCameraView == false && isInside == true)
			pCamera->set(SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainVehicle.GetPosition().x, trainVehicle.GetPosition().y + 2.4, trainVehicle.GetPosition().z - 15));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			trainVehicle.setSpeed(20.5);
		trainVehicle.ProcessKeyboard(V_BACKWARD, (float)deltaTime);
		if (!isTrainSoundPlaying)
		{
			setTrainSound(true);
			isTrainSoundPlaying = true;
		}
		trainZ = trainVehicle.GetPosition().z;
		if (freeCameraView == false)
			pCamera->set(SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainVehicle.GetPosition().x, trainVehicle.GetPosition().y + 6, trainVehicle.GetPosition().z));
		trainVehicle.setSpeed(12.5);
		if (freeCameraView == false && isInside == true)
			pCamera->set(SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainVehicle.GetPosition().x, trainVehicle.GetPosition().y + 2.4, trainVehicle.GetPosition().z - 15));
	}

	//-----------------CAMERA MODE---------------------
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		freeCameraView = false;
		pCamera->set(SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainVehicle.GetPosition().x, trainVehicle.GetPosition().y + 6, trainVehicle.GetPosition().z));
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		freeCameraView = false;
		isInside = true;
		pCamera->set(SCR_WIDTH, SCR_HEIGHT, glm::vec3(trainVehicle.GetPosition().x, trainVehicle.GetPosition().y + 2.4, trainVehicle.GetPosition().z - 15));
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		freeCameraView = true;
		pCamera->SetFreeCamera(true);
		isInside = false;
	}

	//-----------------FREE CAMERA MOVEMENT-----------------
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && freeCameraView == true)
		pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && freeCameraView == true)
		pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && freeCameraView == true)
		pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && freeCameraView == true)
		pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS && freeCameraView)
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS && freeCameraView)
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}