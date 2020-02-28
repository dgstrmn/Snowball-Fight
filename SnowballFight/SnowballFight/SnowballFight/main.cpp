#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>


#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"

#include "Model.h"

#include "Skybox.h"
#include <irrKlang.h>



const float width = 1920.0f;
const float height = 1080.0f;

const float v_snow = 150.0f;
const float a_grav = 1.5f;

Window mainWindow;

struct Snowball
{
	Mesh* mesh;
	glm::vec3 front;
	GLfloat cur_grav = 0;
	glm::vec3 pos;
	glm::vec3 scale = glm::vec3(0.3f, 0.3f, 0.3f);
};

struct envObjects
{
	glm::vec3 pos;
};

std::vector<Shader> shaderList;
Shader shader2D;

std::vector<std::vector<glm::vec3>> heightMap_new;

std::vector<Snowball> snowballList;
std::vector<envObjects*> envList;
std::vector<envObjects*> enemyList;

std::vector<Mesh*> lifeList;

GLfloat *sph_vertices;
unsigned int *sph_indices;
GLfloat *ter_vertices;
unsigned int *ter_indices;

size_t verCount;
size_t indCount;

size_t ter_verCount;
size_t ter_indCount;

Camera camera;

Texture snowTerTexture;
Texture crossTex;
Texture heartTex;
Texture startScreen;
Texture deathScreen;

Material shine;

Model tree;
Model snowman;

Light mainLight;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

GLfloat deltaProjTime = 0.0f;
GLfloat lastProjTime = 0.0f;

GLfloat deltaEnmTime = 0.0f;
GLfloat lastEnmTime = 0.0f;

Skybox skybox;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";
static const char* vShader2D = "Shaders/shader2D.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";
static const char* fShader2D = "Shaders/shader2D.frag";

void setupSphere()
{
	std::vector<GLfloat> verticesVector;
	std::vector<unsigned int> indicesVector;

	double latitudeBands = 5;
	double longitudeBands = 5;
	double radius = 2;

	for (double latNumber = 0; latNumber <= latitudeBands; latNumber++) {
		double theta = latNumber * M_PI / latitudeBands;
		double sinTheta = sin(theta);
		double cosTheta = cos(theta);

		for (double longNumber = 0; longNumber <= longitudeBands; longNumber++) {
			double phi = longNumber * 2 * M_PI / longitudeBands;
			double sinPhi = sin(phi);
			double cosPhi = cos(phi);

			GLfloat temp[8] = { 0.0f };
			temp[5] = cosPhi * sinTheta;   // x
			temp[6] = cosTheta;            // y
			temp[7] = sinPhi * sinTheta;   // z
			temp[3] = 1 - (longNumber / longitudeBands); // u
			temp[4] = 1 - (latNumber / latitudeBands);   // v
			temp[0] = radius * temp[5];
			temp[1] = radius * temp[6];
			temp[2] = radius * temp[7];

			for (size_t i = 0; i < 8; i++)
			{
				verticesVector.push_back(temp[i]);
			}
			
		}

		for (int latNumber = 0; latNumber < latitudeBands; latNumber++) {
			for (int longNumber = 0; longNumber < longitudeBands; longNumber++) {
				int first = (latNumber * (longitudeBands + 1)) + longNumber;
				int second = first + longitudeBands + 1;

				indicesVector.push_back(first);
				indicesVector.push_back(second);
				indicesVector.push_back(first + 1);

				indicesVector.push_back(second);
				indicesVector.push_back(second + 1);
				indicesVector.push_back(first + 1);

			}
		}

		verCount = verticesVector.size();
		sph_vertices = new GLfloat[verCount];
		indCount = indicesVector.size();
		sph_indices = new unsigned int[indCount];

		copy(verticesVector.begin(), verticesVector.end(), sph_vertices);
		copy(indicesVector.begin(), indicesVector.end(), sph_indices);
	}
}

void setupTerrain(float size_xz, float delta, float max_height)
{

	float inv_delta = 1 / delta;
	float point = -size_xz;

	

	unsigned int dist = 2 * size_xz * inv_delta + 1;

	std::vector<std::vector<glm::vec3>> heightMap_old(dist + 2, std::vector<glm::vec3>(dist + 2, glm::vec3(0.0f, max_height/2, 0.0f)));

	heightMap_new = std::vector<std::vector<glm::vec3>>(dist, std::vector<glm::vec3>(dist, glm::vec3(0.0f, max_height / 2, 0.0f)));

	unsigned int vertex_count = std::pow(dist, 2);
	unsigned int square_count = std::pow(dist - 1, 2);

	std::vector<GLfloat> verticesVec;
	std::vector<unsigned int> indicesVec;

	unsigned int cur_index = 0;
	float tex_u = 0.0f;
	float tex_v = dist - 1.0f;
	float x = point, z = point, y = 0.0f;

	unsigned int cur_row = 0, cur_col = 0;

	for (unsigned int i = 0; i < vertex_count; i++)
	{
		y = static_cast <float> (rand()) / ((static_cast <float> (RAND_MAX)) / max_height);

		GLfloat temp[8] = { x, y, z, tex_u, tex_v, 0.0f, 0.0f, 0.0f };
		heightMap_old[cur_row + 1][cur_col + 1].y = y;
		heightMap_old[cur_row + 1][cur_col + 1].x = x;
		heightMap_old[cur_row + 1][cur_col + 1].z = z;
		for (size_t a = 0; a < 8; a++)
		{
			verticesVec.push_back(temp[a]);
		}
		x += delta;
		tex_u += 0.5f;
		if (cur_col == dist - 1)
		{
			x = point;
			z += delta;
			tex_u = 0.0f;
			tex_v -= 0.5f;
			cur_col = 0;
			cur_row++;
		}
		else
		{
			cur_col++;
		}
	}
	unsigned int cur_vertex = 1;
	for (unsigned int i = 1; i < dist + 1; i++)
	{
		for (unsigned int j = 1; j < dist + 1; j++)
		{
			heightMap_new[i-1][j-1].y = (12 * heightMap_old[i][j].y + 
				6 * (heightMap_old[i - 1][j].y + heightMap_old[i][j - 1].y + heightMap_old[i + 1][j].y + heightMap_old[i][j + 1].y) +
				5 * (heightMap_old[i - 1][j - 1].y + heightMap_old[i + 1][j + 1].y + heightMap_old[i + 1][j - 1].y + heightMap_old[i - 1][j + 1].y)) / 23;
			heightMap_new[i - 1][j - 1].x = heightMap_old[i][j].x;
			heightMap_new[i - 1][j - 1].z = heightMap_old[i][j].z;
			verticesVec[cur_vertex] = heightMap_new[i-1][j-1].y;
			cur_vertex += 8;
		}
	}
	cur_vertex = 16 * dist + 21;
	for (unsigned int i = 2; i < dist - 2; i++)
	{
		for (unsigned int j = 2; j < dist - 2; j++)
		{
			glm::vec3 v1 = heightMap_new[i + 1][j] - heightMap_new[i][j];
			glm::vec3 v2 = heightMap_new[i][j + 1] - heightMap_new[i][j];
			glm::vec3 v3 = heightMap_new[i - 1][j] - heightMap_new[i][j];
			glm::vec3 v4 = heightMap_new[i][j - 1] - heightMap_new[i][j];

			glm::vec3 v12 = normalize(glm::cross(v1, v2));
			glm::vec3 v23 = normalize(glm::cross(v2, v3));
			glm::vec3 v34 = normalize(glm::cross(v3, v4));
			glm::vec3 v41 = normalize(glm::cross(v4, v1));

			glm::vec3 realNormal = glm::normalize(v12 + v23 + v34 + v41);
			verticesVec[cur_vertex] = realNormal.x;
			verticesVec[cur_vertex + 1] = realNormal.y;
			verticesVec[cur_vertex + 2] = realNormal.z;
			cur_vertex += 8;
		}
		cur_vertex += 32;
	}
	cur_index = 0;
	
	for (unsigned int i = 0; i < vertex_count - dist; i++)
	{
		

			unsigned int temp[6] = { i, i + dist, i + dist + 1, i, i + 1, i + dist + 1 };
			for (size_t a = 0; a < 6; a++)
			{
				indicesVec.push_back(temp[a]);
			}
			cur_index++;

			if (cur_index == dist - 1)
			{
				cur_index = 0;
				i++;
			}
		
	}

	ter_verCount = verticesVec.size();
	ter_vertices = new GLfloat[ter_verCount];
	ter_indCount = indicesVec.size();
	ter_indices = new unsigned int[ter_indCount];


	copy(verticesVec.begin(), verticesVec.end(), ter_vertices);
	copy(indicesVec.begin(), indicesVec.end(), ter_indices);

}

GLfloat box_vertices[] = {
	-10.0f , -10.0f, 0.0f, 0.0f, 1.0f,
	-10.0f , 10.0f, 0.0f, 0.0f, 0.0f,
   10.0f , 10.0f, 0.0f, 1.0f, 0.0f,
   10.0f , -10.0f, 0.0f, 1.0f, 1.0f
};

unsigned int box_indices[] = {
	0, 1, 2,
	2, 0, 3
};

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	shader2D.CreateFromFiles("Shaders/shader2D.vert", "Shaders/shader2D.frag");
}

float getCurHeight(float leftTopSize, float delta, glm::vec3 objpos)
{
	
	glm::vec3 cur_tri[3];

	unsigned int cur_col = int((objpos.x + leftTopSize) / delta + 0.5);
	unsigned int cur_row = int((objpos.z + leftTopSize) / delta + 0.5);

	if (objpos.x > heightMap_new[cur_row][cur_col].x)
	{
		if (objpos.z < heightMap_new[cur_row][cur_col].z)
		{
			cur_tri[0] = heightMap_new[cur_row - 1][cur_col];
			cur_tri[1] = heightMap_new[cur_row][cur_col];
			cur_tri[2] = heightMap_new[cur_row][cur_col + 1];
		}
		else
		{
			if (glm::length(objpos - heightMap_new[cur_row][cur_col + 1]) <= glm::length(objpos - heightMap_new[cur_row + 1][cur_col]))
			{
				cur_tri[0] = heightMap_new[cur_row][cur_col];
				cur_tri[1] = heightMap_new[cur_row][cur_col + 1];
				cur_tri[2] = heightMap_new[cur_row + 1][cur_col + 1];
			}
			else
			{
				cur_tri[0] = heightMap_new[cur_row][cur_col];
				cur_tri[1] = heightMap_new[cur_row + 1][cur_col];
				cur_tri[2] = heightMap_new[cur_row + 1][cur_col + 1];
			}
		}
	}
	else
	{
		if (objpos.z > heightMap_new[cur_row][cur_col].z)
		{
			cur_tri[0] = heightMap_new[cur_row][cur_col];
			cur_tri[1] = heightMap_new[cur_row][cur_col - 1];
			cur_tri[2] = heightMap_new[cur_row + 1][cur_col];
		}
		else
		{
			if (glm::length(objpos - heightMap_new[cur_row][cur_col - 1]) < glm::length(objpos - heightMap_new[cur_row - 1][cur_col]))
			{
				cur_tri[0] = heightMap_new[cur_row][cur_col];
				cur_tri[1] = heightMap_new[cur_row][cur_col - 1];
				cur_tri[2] = heightMap_new[cur_row - 1][cur_col - 1];
			}
			else
			{
				cur_tri[0] = heightMap_new[cur_row][cur_col];
				cur_tri[1] = heightMap_new[cur_row - 1][cur_col];
				cur_tri[2] = heightMap_new[cur_row - 1][cur_col - 1];
			}
		}
	}

	float cur_height = 0.0f;

	float det = (cur_tri[1].z - cur_tri[2].z) * (cur_tri[0].x - cur_tri[2].x) + (cur_tri[2].x - cur_tri[1].x) * (cur_tri[0].z - cur_tri[2].z);
	float l1 = ((cur_tri[1].z - cur_tri[2].z) * (objpos.x - cur_tri[2].x) + (cur_tri[2].x - cur_tri[1].x) * (objpos.z - cur_tri[2].z)) / det;
	float l2 = ((cur_tri[2].z - cur_tri[0].z) * (objpos.x - cur_tri[2].x) + (cur_tri[0].x - cur_tri[2].x) * (objpos.z - cur_tri[2].z)) / det;
	float l3 = 1.0f - l1 - l2;
	cur_height = l1 * cur_tri[0].y + l2 * cur_tri[1].y + l3 * cur_tri[2].y;

	return cur_height;
}

bool isColliding(glm::vec3 firstObj, glm::vec3 secondObj) 
{
	//this collider
	float r1 = firstObj.x + 2.0f;
	float l1 = firstObj.x - 2.0f;
	float t1 = firstObj.z + 2.0f;
	float b1 = firstObj.z - 2.0f;

	//the otherder
	float r2 = secondObj.x + 2.0f;
	float l2 = secondObj.x - 2.0f;
	float t2 = secondObj.z + 2.0f;
	float b2 = secondObj.z - 2.0f;

	return !(r1 < l2 || l1 > r2 || t1 < b2 || b1 > t2);
}

int main() 
{
	int life = 10;
	mainWindow = Window(1920, 1080);
	mainWindow.Initialise();

	tree = Model();
	tree.LoadModel("Models/tree.obj");
	snowman = Model();
	snowman.LoadModel("Models/snowman.obj");

	irrklang::ISoundEngine *SoundEngine = irrklang::createIrrKlangDevice();

	SoundEngine->play2D("Audio/breakout.mp3", GL_FALSE);
	
	
	setupSphere();

	srand(static_cast <unsigned> (time(0)));
	float leftTopSize = 500.0f, delta = 25.0f, maxHeight = 17.0f;
	setupTerrain(leftTopSize, delta, maxHeight);

	std::vector<std::vector<bool>> block_check(leftTopSize*2/delta, std::vector<bool>(leftTopSize * 2 / delta, false));

	for (size_t i = 0; i < 250; i++)
	{
		unsigned int pos_tree_x = static_cast <unsigned int> (rand()) / ((static_cast <unsigned int> (RAND_MAX)) / ((leftTopSize * 2) / delta));
		unsigned int pos_tree_z = static_cast <unsigned int> (rand()) / ((static_cast <unsigned int> (RAND_MAX)) / ((leftTopSize * 2) / delta));


		if ((heightMap_new[pos_tree_x][pos_tree_z].x == 0.0f && heightMap_new[pos_tree_x][pos_tree_z].z == 0.0f) || block_check[pos_tree_x][pos_tree_z])
		{
			i--;
			continue;
		}
		else
		{
			block_check[pos_tree_x][pos_tree_z] = true;
			envObjects* env;
			env = new envObjects();
			env->pos = heightMap_new[pos_tree_x][pos_tree_z];
			envList.push_back(env);
		}
	}

	CreateShaders();

	Mesh terrain = Mesh();
	terrain.CreateMesh(ter_vertices, ter_indices, ter_verCount, ter_indCount);

	Mesh crosshair = Mesh();
	crosshair.CreateHUDMesh(box_vertices, box_indices, 20, 6);
	Mesh startScr = Mesh();
	startScr.CreateHUDMesh(box_vertices, box_indices, 20, 6);
	Mesh deathScr = Mesh();
	deathScr.CreateHUDMesh(box_vertices, box_indices, 20, 6);

	for (size_t i = 0; i < 10; i++)
	{
		Mesh* heart = new Mesh();
		heart->CreateHUDMesh(box_vertices, box_indices, 20, 6);
		lifeList.push_back(heart);
	}

	bool left_click = false;
	bool space_press = false;

	camera = Camera(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 50.0f, 0.2f);

	snowTerTexture = Texture("Textures/snow_terrain.jpg");
	snowTerTexture.LoadTexture();
	crossTex = Texture("Textures/crosshair.png");
	crossTex.LoadTextureA();
	heartTex = Texture("Textures/life.png");
	heartTex.LoadTextureA();
	startScreen = Texture("Textures/play.png");
	startScreen.LoadTextureA();
	deathScreen = Texture("Textures/dead.jpg");
	deathScreen.LoadTexture();

	shine = Material(0.5f, 16);

	mainLight = Light(1.0f, 1.0f, 1.0f, 0.5f, 
					-0.5f, -0.5f, 2.0f, 0.8f);

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/snow_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/snow_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/snow_up.tga");
	skyboxFaces.push_back("Textures/Skybox/snow_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/snow_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/snow_ft.tga");

	skybox = Skybox(skyboxFaces);


	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformAmbientIntensity = 0, uniformAmbientColour = 0, uniformDirection = 0, uniformDiffuseIntensity = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 300.0f);
	glm::mat4 orthogonal = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

	bool game_started = false;
	bool player_alive = true;
	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0);

		if (life == 0)
		{
			game_started = false;
			player_alive = false;
		}
			

		if (!game_started)
		{
			if (!player_alive)
			{
				glfwPollEvents();
				glDisable(GL_DEPTH_TEST);

				shader2D.UseShader();
				uniformProjection = shader2D.GetProjectionLocation();
				uniformModel = shader2D.GetModelLocation();

				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(orthogonal));

				model = glm::mat4(1.0);
				model = glm::translate(model, glm::vec3(width / 2, height / 2, 0.0f));
				model = glm::scale(model, glm::vec3(width / 20, height / 20, 1.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				deathScreen.UseTexture();
				deathScr.RenderMesh();

				glEnable(GL_DEPTH_TEST);

				glUseProgram(0);
				mainWindow.swapBuffers();
			}
			else
			{
				glfwPollEvents();
				if (mainWindow.getsBoardKeys()[GLFW_KEY_SPACE])
				{
					game_started = true;
					SoundEngine->setAllSoundsPaused();
				}
				else
				{
					glDisable(GL_DEPTH_TEST);

					shader2D.UseShader();
					uniformProjection = shader2D.GetProjectionLocation();
					uniformModel = shader2D.GetModelLocation();

					glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(orthogonal));

					model = glm::mat4(1.0);
					model = glm::translate(model, glm::vec3(width / 2, height / 2, 0.0f));
					model = glm::scale(model, glm::vec3(width / 20, height / 20, 1.0f));
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
					startScreen.UseTexture();
					startScr.RenderMesh();

					glEnable(GL_DEPTH_TEST);

					glUseProgram(0);
					mainWindow.swapBuffers();
				}
			}
		}
		else
		{
			skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

			GLfloat now = glfwGetTime();
			deltaTime = now - lastTime;
			lastTime = now;

			glm::vec3 cam_pos = camera.getCameraPosition();

			// Get + Handle User Input
			glfwPollEvents();
			camera.keyControl(mainWindow.getsBoardKeys(), deltaTime, getCurHeight(leftTopSize, delta, cam_pos) + 12.5f, leftTopSize - delta);
			camera.mouseControl(mainWindow.getsMouseKeys(), mainWindow.getXChange(), mainWindow.getYChange(), left_click);

			if (left_click)
			{
				GLfloat now_proj = glfwGetTime();
				deltaProjTime = now_proj - lastProjTime;

				if (deltaProjTime > 0.4f)
				{
					lastProjTime = now_proj;

					Snowball snowball;
					snowball.mesh = new Mesh();
					snowball.mesh->CreateMesh(sph_vertices, sph_indices, verCount, indCount);

					snowball.front = camera.getCameraFront();

					glm::mat4 model(1.0);
					glm::vec3 temp = camera.getCameraPosition() + snowball.front;
					model = glm::translate(model, temp);
					snowball.pos = temp;

					snowballList.push_back(snowball);

					if (snowballList.size() > 100)
					{
						snowballList.erase(snowballList.begin());
					}
				}
			}

			GLfloat now_enm = glfwGetTime();
			deltaEnmTime = now_enm - lastEnmTime;

			if (deltaEnmTime > 1.0f)
			{
				lastEnmTime = now_enm;
				unsigned int pos_enemy_x = static_cast <unsigned int> (rand()) / ((static_cast <unsigned int> (RAND_MAX)) / ((leftTopSize * 2) / delta));
				unsigned int pos_enemy_z = static_cast <unsigned int> (rand()) / ((static_cast <unsigned int> (RAND_MAX)) / ((leftTopSize * 2) / delta));


				if ((heightMap_new[pos_enemy_x][pos_enemy_z].x >= camera.getCameraPosition().x + 5 * delta || heightMap_new[pos_enemy_x][pos_enemy_z].x <= camera.getCameraPosition().x - 5 * delta ||
					heightMap_new[pos_enemy_x][pos_enemy_z].z >= camera.getCameraPosition().z + 5 * delta || heightMap_new[pos_enemy_x][pos_enemy_z].z <= camera.getCameraPosition().z - 5 * delta) &&
					!block_check[pos_enemy_x][pos_enemy_z] && enemyList.size() <= 100)
				{
					envObjects* env;
					env = new envObjects();
					env->pos = heightMap_new[pos_enemy_x][pos_enemy_z];
					env->pos.y += 2.0f;
					enemyList.push_back(env);
				}
			}
			// Clear the window

			shaderList[0].UseShader();
			uniformModel = shaderList[0].GetModelLocation();
			uniformProjection = shaderList[0].GetProjectionLocation();
			uniformView = shaderList[0].GetViewLocation();
			uniformAmbientColour = shaderList[0].GetAmbientColourLocation();
			uniformAmbientIntensity = shaderList[0].GetAmbientIntensityLocation();
			uniformDirection = shaderList[0].GetDirectionLocation();
			uniformDiffuseIntensity = shaderList[0].GetDiffuseIntensityLocation();
			uniformEyePosition = shaderList[0].GetEyePositionLocation();
			uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
			uniformShininess = shaderList[0].GetShininessLocation();

			mainLight.UseLight(uniformAmbientIntensity, uniformAmbientColour,
				uniformDiffuseIntensity, uniformDirection);

			glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
			glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

			model = glm::mat4(1.0);
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			snowTerTexture.UseTexture();
			shine.UseMaterial(uniformSpecularIntensity, uniformShininess);
			terrain.RenderMesh();


			for (size_t i = 0; i < 250; i++)
			{
				model = glm::mat4(1.0);
				model = glm::translate(model, glm::vec3(envList[i]->pos.x, envList[i]->pos.y - 2.0f, envList[i]->pos.z));
				model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				shine.UseMaterial(uniformSpecularIntensity, uniformShininess);
				tree.RenderModel();
			}

			for (size_t i = 0; i < enemyList.size(); i++)
			{
				bool enemy_deleted = false;
				model = glm::mat4(1.0);
				float cam_x = camera.getCameraPosition().x;
				float cam_z = camera.getCameraPosition().z;
				float enm_x = enemyList[i]->pos.x;
				float enm_z = enemyList[i]->pos.z;
				float enm_y = enemyList[i]->pos.y;
				model = glm::translate(model, glm::vec3(enm_x, enm_y, enm_z));

				float angle = std::atan2f(enm_z - cam_z, enm_x - cam_x) - M_PI;

				model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f)); // rotate around center
				model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
				enemyList[i]->pos -= glm::normalize(glm::vec3(enm_x - cam_x, 0.0f, enm_z - cam_z)) * 0.3f;
				glm::vec3 enm_pos = glm::vec3(enemyList[i]->pos.x, 0.0f, enemyList[i]->pos.z);
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				shine.UseMaterial(uniformSpecularIntensity, uniformShininess);
				snowman.RenderModel();
				enemyList[i]->pos.y = getCurHeight(leftTopSize, delta, enm_pos) + 2.0f;
				if (isColliding(camera.getCameraPosition(), enemyList[i]->pos))
				{
					SoundEngine->play2D("Audio/damage.mp3", GL_FALSE);
					enemyList.erase(enemyList.begin() + i);
					life--;
					break;
				}
				for (size_t j = 0; j < snowballList.size(); j++)
				{
					if (isColliding(enemyList[i]->pos, snowballList[j].pos))
					{
						if (snowballList[j].pos.y - 1.0f <= enemyList[i]->pos.y + 13.0f && snowballList[j].pos.y + 1.0f >= enemyList[i]->pos.y)
						{
							SoundEngine->play2D("Audio/hit.wav", GL_FALSE);
							enemyList.erase(enemyList.begin() + i);
							snowballList.erase(snowballList.begin() + j);
							enemy_deleted = true;
							break;
						}
					}
				}
				if (enemy_deleted)
					break;
			}

			for (size_t i = 0; i < snowballList.size(); i++)
			{
				model = glm::mat4(1.0);
				model = glm::translate(model, snowballList[i].pos);
				glm::vec3 trans_temp = snowballList[i].front * v_snow * deltaTime;
				model = glm::translate(model, trans_temp);
				snowballList[i].pos += trans_temp;
				snowballList[i].cur_grav = snowballList[i].cur_grav - a_grav * deltaTime;
				trans_temp = glm::vec3(0.0f, snowballList[i].cur_grav, 0.0f);
				model = glm::translate(model, trans_temp);
				snowballList[i].pos += trans_temp;
				model = glm::scale(model, snowballList[i].scale);

				if (snowballList[i].pos.y < 0.0f) {
					snowballList.erase(snowballList.begin() + i);
					break;
				}

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				snowTerTexture.UseTexture();
				shine.UseMaterial(uniformSpecularIntensity, uniformShininess);
				snowballList[i].mesh->RenderMesh();
			}

			glDisable(GL_DEPTH_TEST);

			shader2D.UseShader();
			uniformProjection = shader2D.GetProjectionLocation();
			uniformModel = shader2D.GetModelLocation();

			glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(orthogonal));

			model = glm::mat4(1.0);
			model = glm::translate(model, glm::vec3(width / 2, height / 2, 0.0f));
			model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			crossTex.UseTexture();
			crosshair.RenderMesh();

			for (size_t i = 0; i < life; i++)
			{
				model = glm::mat4(1.0);
				model = glm::scale(model, glm::vec3(2.0f, 2.0f, 1.0f));
				model = glm::translate(model, glm::vec3(i * 20.0f + 10.0f, 10.0f, 0.0f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				heartTex.UseTexture();
				lifeList[i]->RenderMesh();
			}

			glEnable(GL_DEPTH_TEST);

			glUseProgram(0);
			mainWindow.swapBuffers();

		}
	}
	if (game_started)
	{
		delete[] sph_vertices;
		delete[] sph_indices;
		delete[] ter_vertices;
		delete[] ter_indices;
	}
	

	return 0;
}