#include <iostream>
#include <cmath>
// GLEW
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// Other Libs
#include "stb_image.h"
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//Load Models
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Function prototypes
// Funciones de interaccion
void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Funciones de animacion
void animateCircularDrift(float deltaTime);
void UpdateDayNightTransition(bool& isNight, float& timeOfDay, float deltaTime, float speed = 1.0f);
template<typename T>
T lerp(const T& a, const T& b, float t) {
	return a + t * (b - a);
}
float smoothstep(float edge0, float edge1, float x) {
	x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return x * x * (3.0f - 2.0f * x);
}

// Window dimensions
//const GLuint WIDTH = 800, HEIGHT = 600;
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
//Camera  camera(glm::vec3(0.0f, 0.0f, 135.0f));
Camera  camera(glm::vec3(0.0f, 35.0f, 0.0f));

GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Variables globales
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

//Animacion de luces
bool isNight = false;
float timeOfDay = 0.0f; // 0.0 = di­a, 1.0 = noche
float transitionSpeed = 1.0f;
bool keyPressed = false;
bool lightsOff = true;
bool keyPressed2 = false;

// Animacion carrito de golf
float driftAngle = 0.0f;
float circleRadius = 7.0f;  // Radio del ci­rculo de drift
float rotationSpeed = 2.0f; // Velocidad de rotacion (radianes/segundo)
glm::vec3 pivotPoint = glm::vec3(35.0f, -6.52f, 46.0f); // Punto de pivote (ruedas delanteras)


glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

glm::vec3 Light1 = glm::vec3(1.0f, 1.0f, 0.0f);

// Luces puntuales
glm::vec3 pointLightPositions[] = {
	glm::vec3(-14.0f, 12.5f, -11.0f),
	glm::vec3(-17.0f, 17.0f, 80.0f),
	glm::vec3(12.45f, 30.3f, -16.4f),
};

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
	glfwInit();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto Final - Computacion Grafica", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

	//Modelos exterior
	//Model Casa((char*)"Models/casa.obj");
	//Model Arbusto((char*)"Models/arbusto.fbx");
	//Model Arbol((char*)"Models/arbol.obj");
	//Model Farol((char*)"Models/faro.obj");
	//Model Cesped((char*)"Models/cesped.obj");
	//Model Cielo((char*)"Models/cieloo.obj");
	//Model Cart((char*)"Models/cart.obj");

	//// Modelos interior cocina
	//Model Pared_Cocina((char*)"Models/pared_cocina.obj");
	//Model Pared_Cocina_Anterior((char*)"Models/pared_cocina_anterior.obj");
	Model Piso((char*)"Models/piso_cocina.obj");
	//Model Puerta_Cocina((char*)"Models/puerta_cocina.obj");
	//Model Ventana_Cocina((char*)"Models/ventana_cocina.obj");
	//Model Pared_Madera((char*)"Models/pared_madera.obj");
	//Model Alacena((char*)"Models/alacena.obj");
	//Model Alacena_Superior((char*)"Models/alacena_superior.obj");
	//Model Alacena_Grande((char*)"Models/alacena_grande.obj");
	//Model Campana((char*)"Models/campana.obj");
	//Model Estufa((char*)"Models/estufa.obj");
	//Model Mesa((char*)"Models/mesa.obj");
	//Model Refrigerador((char*)"Models/refrigerador.obj");
	//Model Silla((char*)"Models/silla.obj");
	//Model Tostadora((char*)"Models/tostadora.obj");

	//// Modelos interior habitacion
	//Model Archivero((char*)"Models/archivero.obj");
	//Model Buro((char*)"Models/buro.obj");
	//Model Cajones((char*)"Models/cajones.obj");
	//Model Cama((char*)"Models/cama.obj");
	//Model Pared_Habitacion((char*)"Models/pared_habitacion.obj");
	//Model Television_Habitacion((char*)"Models/television_habitacion.obj");
	Model Trampolin((char*)"Models/trampolin.obj");
	//Model Ventana_Habitacion((char*)"Models/ventana_habitacion.obj");
	Model Rigby((char*)"Models/rigby.fbx");

	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 350.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		animateCircularDrift(deltaTime);
		// Funciona con o sin el cuarto argumento (por el valor predeterminado)
		UpdateDayNightTransition(isNight, timeOfDay, deltaTime, 0.3);

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL options
		glEnable(GL_DEPTH_TEST);

		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// Luz direccional (sol/luna)
		glm::vec3 dayLightDir(-0.2f, -1.0f, -0.3f);
		glm::vec3 nightLightDir(0.2f, -1.0f, 0.3f); // Direccion diferente para la luna
		glm::vec3 currentLightDir = lerp(dayLightDir, nightLightDir, timeOfDay);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"),
			currentLightDir.x, currentLightDir.y, currentLightDir.z);

		// Colores interpolados
		float ambientFactor = smoothstep(0.2f, 0.8f, timeOfDay);
		glm::vec3 currentAmbient = lerp(glm::vec3(0.5f), glm::vec3(0.15f), ambientFactor);
		glm::vec3 currentDiffuse = lerp(glm::vec3(0.8f, 0.8f, 0.7f), glm::vec3(0.2f, 0.2f, 0.4f), timeOfDay);
		glm::vec3 currentSpecular = lerp(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.1f, 0.1f, 0.2f), timeOfDay);
		float currentShininess = lerp(32.0f, 16.0f, timeOfDay);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"),
			currentAmbient.x, currentAmbient.y, currentAmbient.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"),
			currentDiffuse.x, currentDiffuse.y, currentDiffuse.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"),
			currentSpecular.x, currentSpecular.y, currentSpecular.z);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), lerp(32.0f, 64.0f, timeOfDay));


		// Configuracion de la luz puntual 1
		float lightsOffFactor = lightsOff ? 0.0f : 1.0f; // Factor de luz apagada
		float lerpFactor2 = lightsOff * 1.0f; // Factor de interpolacion

		// Luz puntual 1
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"),
			pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"),
			0.2f * lightsOffFactor, 0.2f * lightsOffFactor, 0.2f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"),
			1.0f * lightsOffFactor, 1.0f * lightsOffFactor, 1.0f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"),
			1.0f * lightsOffFactor, 1.0f * lightsOffFactor, 1.0f * lightsOffFactor);

		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.032f);

		// Configuracion de la luz puntual 2
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"),
			pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);

		// Color amarillento (mas intenso en el componente rojo y verde, menos azul)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"),
			0.5f * lightsOffFactor, 0.5f * lightsOffFactor, 0.1f * lightsOffFactor);  // Amarillo suave ambiental

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"),
			4.0f * lightsOffFactor, 3.5f * lightsOffFactor, 2.0f * lightsOffFactor);  // Amarillo brillante difuso

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"),
			2.5f * lightsOffFactor, 2.2f * lightsOffFactor, 1.3f * lightsOffFactor);  // Amarillo especular

		// Parametros de atenuacion para mayor alcance (valores mas bajos = mayor alcance)
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.05f);    // Reducido para mayor alcance
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.01f); // Reducido para mayor alcance

		// Configuracion de la luz puntual 3
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"),
			pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		// Color amarillento (mas intenso en el componente rojo y verde, menos azul)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"),
			0.5f * lightsOffFactor, 0.5f * lightsOffFactor, 0.1f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"),
			1.0f * lightsOffFactor, 0.8f * lightsOffFactor, 0.3f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"),
			0.5f * lightsOffFactor, 0.4f * lightsOffFactor, 0.1f * lightsOffFactor);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.032f);


		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 model(1);

		//Carga de modelos
		view = camera.GetViewMatrix();

		/////////////////////////-Ambientacion-////////////////////////////////
		//Arbusto central
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(58.0f, -10.0f, 20.0f));
		//model = glm::scale(model, glm::vec3(18.0f, 20.0f, 18.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbusto.Draw(lightingShader);
		////Arbusto izquierdo
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(45.0f, -10.0f, 22.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbusto.Draw(lightingShader);
		////Arbusto derecho
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(75.0f, -10.0f, 21.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbusto.Draw(lightingShader);
		////Arbusto derecho 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(85.0f, -10.0f, 21.0f));
		//model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbusto.Draw(lightingShader);

		////Arbol derecha
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(90.0f, -10.0f, -30.0f));
		//model = glm::scale(model, glm::vec3(12.0f, 12.0f, 12.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbol.Draw(lightingShader);
		////Arbol izquierda
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-90.0f, -10.0f, 30.0f));
		//model = glm::scale(model, glm::vec3(12.0f, 12.0f, 12.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbol.Draw(lightingShader);
		////Arbol 1
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-80.0f, -10.0f, -30.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbol.Draw(lightingShader);
		////Arbol 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-150.0f, -10.0f, -70.0f));
		//model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbol.Draw(lightingShader);
		////Arbol 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-230.0f, -10.0f, -70.0f));
		//model = glm::scale(model, glm::vec3(8.0f, 8.0f, 8.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Arbol.Draw(lightingShader);
		//
		////Faro
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-17.0f, -10.0f, 80.0f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Farol.Draw(lightingShader);

		////Cesped
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.0f, -13.0f, 20.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Cesped.Draw(lightingShader);

		////Cielo
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.0f, -14.0f, 20.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Cielo.Draw(lightingShader);

		////Carrito de golf
		//// Calcular posicion en el ci­rculo
		//float carPosX = pivotPoint.x + circleRadius * sin(driftAngle);
		//float carPosZ = pivotPoint.z + circleRadius * cos(driftAngle);

		//// Orientacion del carrito (apuntando tangente al ci­rculo)
		//float carRotation = driftAngle + glm::pi<float>() / 2; // 90Â° adicionales para orientacion correcta

		//// Aplicar transformaciones
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(carPosX, pivotPoint.y, carPosZ));
		//model = glm::rotate(model, carRotation, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotacion en Y
		//model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Cart.Draw(lightingShader);

		/////////////////////////////-Fachada-////////////////////////////////
		////Casa
		//model = glm::mat4(1);
		//model = glm::scale(model, glm::vec3(6.0f, 6.0f, 6.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Casa.Draw(lightingShader);
		////Piso general (provisional)
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-3.0f, 5.55f, 3.0f));
		//model = glm::scale(model, glm::vec3(2.7f, 1.0f, 1.9f));
		//model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Piso.Draw(lightingShader);
		////Techo
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(3.75f, 13.65f, 2.65f));
		//model = glm::scale(model, glm::vec3(2.55f, 1.0f, 1.75));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Piso.Draw(lightingShader);

		//////////////////////////-Cocina-////////////////////////////////
		//////////////////////////-Estructura-////////////////////////////////
		////Piso
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.2f, 4.1f, -12.7f));
		//model = glm::scale(model, glm::vec3(1.28, 1.0f, 0.84f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Piso.Draw(lightingShader);
		////Techo
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.2f, 13.0f, -12.7f));
		//model = glm::scale(model, glm::vec3(1.28, 1.0f, 0.84f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Piso.Draw(lightingShader);
		////Pared izquierda
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-15.57f, 3.3f, -9.6f));
		//model = glm::scale(model, glm::vec3(1.0f, 2.06f, 0.85f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Cocina.Draw(lightingShader);
		////Pared derecha
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(12.0f, 3.3f, -9.6f));
		//model = glm::scale(model, glm::vec3(1.0f, 2.06f, 0.85f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Cocina.Draw(lightingShader);
		////Pared posterior
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.2f, 3.3f, -32.3f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
		//model = glm::scale(model, glm::vec3(1.0f, 2.06f, 1.28f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Cocina.Draw(lightingShader);
		////Pared anterior
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.2f, 3.3f, -14.2f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 2.06f, 1.28f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Cocina_Anterior.Draw(lightingShader);
		//////Ventana
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-21.2f, 10.2f, -20.37f));
		//model = glm::scale(model, glm::vec3(3.35f, 3.35f, 1.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Ventana_Cocina.Draw(lightingShader);
		////Puerta interior
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-4.8f, 7.2f, -20.15f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Puerta_Cocina.Draw(lightingShader);
		////Puerta exterior
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-4.8f, 7.2f, -21.35f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Puerta_Cocina.Draw(lightingShader);
		////Pared Madera 1
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-7.9f, 6.2f, -20.3f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Madera.Draw(lightingShader);
		////Pared Madera 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-1.6f, 6.2f, -20.3f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Madera.Draw(lightingShader);

		////////////////////////////-Objetos-////////////////////////////////
		//////Alacena 1
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-19.0f, 4.74f, -17.57f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena.Draw(lightingShader);	
		//////Alacena 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-21.1f, 4.74f, -14.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena.Draw(lightingShader);
		//////Alacena 3
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-19.0f, 4.74f, -10.5f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena.Draw(lightingShader);
		//////Alacena 4
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-17.1f, 4.74f, -14.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena.Draw(lightingShader);
		//////Alacena 5
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-13.1f, 4.74f, -14.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena.Draw(lightingShader);
		////Alacena 6
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-15.0f, 11.5f, -19.1f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena_Superior.Draw(lightingShader);
		////Alacena 7
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-25.5f, 9.8f, -17.0f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Alacena_Grande.Draw(lightingShader);

		//////Estufa
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-21.15f, 5.91f, -23.85f));	
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Estufa.Draw(lightingShader);
		//////Campana
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-19.2f, 6.1f, -23.4f));
		//glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 200.0f);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Campana.Draw(lightingShader);
		////Refrigerador
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-16.8f, 4.7f, -19.1f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Refrigerador.Draw(lightingShader);
		////Mesa
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-11.3, 6.82f, -11.75));
		//model = glm::scale(model, glm::vec3(0.85f, 0.9f, 0.85f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Mesa.Draw(lightingShader);
		////Silla 1
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-15.3, 6.4f, -5.0f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		//model = glm::rotate(model, glm::radians(315.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Silla.Draw(lightingShader);
		////Silla 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-24.3, 6.4f, -9.6f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		//model = glm::rotate(model, glm::radians(225.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Silla.Draw(lightingShader);
		////Tostadora
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-11.5f, 4.74f, -16.5f));
		//model = glm::rotate(model, glm::radians(315.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Tostadora.Draw(lightingShader);

		////////////////////////-Habitacion-////////////////////////////////
		////////////////////////-Estructura-////////////////////////////////
		//Piso
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(3.0f, 25.4f, 0.2f));
		model = glm::scale(model, glm::vec3(2.15f, 1.0f, 1.35f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Piso.Draw(lightingShader);
		//Techo
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(3.0f, 35.3f, 0.19f));
		//model = glm::scale(model, glm::vec3(2.155f, 1.0f, 1.36f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Piso.Draw(lightingShader);
		////Pared posterior
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(16.5f, 28.5f, -19.7f));
		//model = glm::scale(model, glm::vec3(3.2f, 3.2f, 1.5f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Habitacion.Draw(lightingShader);
		////Pared derecha
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(25.3f, 28.5f, -6.6f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(3.2f, 3.2f, 1.5f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Habitacion.Draw(lightingShader);
		////Pared anterior
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(16.5f, 28.5f, -1.5f));
		//model = glm::scale(model, glm::vec3(3.2f, 3.2f, 1.5f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Habitacion.Draw(lightingShader);
		////Pared izquierda
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(7.1f, 28.5f, -6.6f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(3.2f, 3.2f, 1.5f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Pared_Habitacion.Draw(lightingShader);
		////Ventana Habitacion 1
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(10.0f, 32.0f, -17.6f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Ventana_Habitacion.Draw(lightingShader);
		////Ventana Habitacion 2
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(19.0f, 32.0f, -17.6f));
		//model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Ventana_Habitacion.Draw(lightingShader);


		//////////////////////////-Objetos-////////////////////////////////
		////Cama
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(8.0f, 27.2f, -10.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Cama.Draw(lightingShader);
		////Buro 
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(11.8f, 27.5f, -15.9f));
		//model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Buro.Draw(lightingShader);
		//Trampolin
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(19.0f, 27.2f, -14.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Trampolin.Draw(lightingShader);
		//Rigby
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(17.0f, 27.2f, -14.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Rigby.Draw(lightingShader);
		////Archivero
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(22.2f, 28.4f, -10.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Archivero.Draw(lightingShader);
		////Cajones
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(22.2f, 28.0f, -7.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Cajones.Draw(lightingShader);
		////Television
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(22.0f, 30.55f, -7.0f));
		//model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Television_Habitacion.Draw(lightingShader);

		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Dibujar ejes de coordenadas
		lampShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		for (int i = 0; i < 3; i++) {
			glm::mat4 model(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniform3f(glGetUniformLocation(lampShader.Program, "lampColor"), 1.0f, 1.0f, 1.0f);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		glfwSwapBuffers(window);
	}


	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	float multiplier = 1.0f;
	if (keys[GLFW_KEY_LEFT_CONTROL])
	{
		multiplier = 6.0f;
	}

	if (keys[GLFW_KEY_W]) {
		camera.ProcessKeyboard(FORWARD, deltaTime * multiplier);
	}
	if (keys[GLFW_KEY_S]) {
		camera.ProcessKeyboard(BACKWARD, deltaTime * multiplier);
	}
	if (keys[GLFW_KEY_A]) {
		camera.ProcessKeyboard(LEFT, deltaTime * multiplier);
	}
	if (keys[GLFW_KEY_D]) {
		camera.ProcessKeyboard(RIGHT, deltaTime * multiplier);
	}

	if (keys[GLFW_KEY_SPACE]) {
		camera.ProcessKeyboard(UP, deltaTime * multiplier);
	}
	if (keys[GLFW_KEY_LEFT_SHIFT]) {
		camera.ProcessKeyboard(DOWN, deltaTime * multiplier);
	}

	if (keys[GLFW_KEY_K])
	{
		pointLightPositions[0].x += 0.3f;
	}
	if (keys[GLFW_KEY_H])
	{
		pointLightPositions[0].x -= 0.3f;
	}

	if (keys[GLFW_KEY_UP])
	{
		pointLightPositions[0].y += 0.3f;
	}

	if (keys[GLFW_KEY_DOWN])
	{
		pointLightPositions[0].y -= 0.3f;
	}
	if (keys[GLFW_KEY_U])
	{
		pointLightPositions[0].z -= 0.3f;
	}
	if (keys[GLFW_KEY_J])
	{
		pointLightPositions[0].z += 0.3f;
	}

}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
	if (keys[GLFW_KEY_N] && !keyPressed) {
		isNight = !isNight; // Alterna entre noche y di­a
		keyPressed = true; // Marca que la tecla ha sido presionada
	}
	else if (!keys[GLFW_KEY_N]) {
		keyPressed = false; // Si la tecla se ha soltado, resetea el estado
	}

	if (keys[GLFW_KEY_L] && !keyPressed) {
		lightsOff = !lightsOff; // Alterna entre noche y di­a
		keyPressed2 = true; // Marca que la tecla ha sido presionada
	}
	else if (!keys[GLFW_KEY_L]) {
		keyPressed2 = false; // Si la tecla se ha soltado, resetea el estado
	}
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void animateCircularDrift(float deltaTime) {
	driftAngle += rotationSpeed * deltaTime; // Aumenta el angulo continuamente

	// Mantener el angulo entre 0 y 2Ï€ para evitar overflow
	if (driftAngle > 2 * glm::pi<float>()) {
		driftAngle -= 2 * glm::pi<float>();
	}
}

void UpdateDayNightTransition(bool& isNight, float& timeOfDay, float deltaTime, float speed) {
	float target = isNight ? 1.0f : 0.0f;
	float direction = (target > timeOfDay) ? 1.0f : -1.0f;
	timeOfDay += direction * deltaTime * speed;

	// Suavizado adicional cerca de los extremos
	if ((direction > 0 && timeOfDay > 0.95f) || (direction < 0 && timeOfDay < 0.05f)) {
		timeOfDay = target; // Fuerza el valor final sin oscilaciones
	}
}