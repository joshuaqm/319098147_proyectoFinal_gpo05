#include <iostream>
#include <cmath>
// GLEW
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// Other Libs
#include "stb_image.h"
#include <random>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//Load Models
#include "SOIL2/SOIL2.h"
// Audio includes
#include <AL/al.h>
#include <AL/alc.h>
// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

// Function prototypes
// Funciones de interaccion
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 135.0f));
//Camera  camera(glm::vec3(-8.0f, 10.0f, 0.0f));

// Funciones de animacion
void animateCircularDrift(float deltaTime);
void UpdateDayNightTransition(float deltaTime);
;
template<typename T>
T lerp(const T& a, const T& b, float t) {
	return a + t * (b - a);
}
float smoothstep(float edge0, float edge1, float x) {
	x = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return x * x * (3.0f - 2.0f * x);
}
void updateTrampolineJump(float deltaTime);
void actualizarCaidaHojas(float deltaTime);

//Funciones de audio 
bool loadWavFile(const char* filename, ALuint buffer);
bool initAudio(const char* wavFile);
void updateListener(const glm::vec3& listenerPos, const glm::vec3& listenerDir);
void toggleAudioPlayback();
void cleanupAudio();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
//const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Variables globales
bool active;

//Animacion de luces
bool isNight = false;
float timeOfDay = 0.0f;
float transitionSpeed = 0.25f;
bool transitionActive = false;
bool lightsOff = true; // Estado de las luces
bool keyPressed2 = false; // Registra el estado de la tecla 2

// Animacion carrito de golf
float driftAngle = 0.0f;
float circleRadius = 7.0f;  // Radio del ci­rculo de drift
float rotationSpeed = 2.0f; // Velocidad de rotacion (radianes/segundo)
glm::vec3 pivotPoint = glm::vec3(35.0f, -6.52f, 46.0f); // Punto de pivote (ruedas delanteras)

// Animacion saltos
glm::vec3 trampolinPos = glm::vec3(19.0f, 27.2f, -14.5f);  // Posición del trampolín
glm::vec3 rigbyBasePos = glm::vec3(19.0f, 27.3f, -14.55f); // Posición base de Rigby
float rigbyScale = 0.03f;                                  // Escala de Rigby
float jumpHeight = 1.5f;    // Altura del salto
float jumpSpeed = 1.2f;     // Velocidad del rebotea
float jumpProgress = 0.0f;  // Progreso del salto (0 a 1)
bool isAscending = true;    // Dirección del salto
float squashFactor = 0.0f;  // Controla la compresión (0 = normal, 1 = máximo squash)
float squashIntensity = 0.1f; // Qué tanto se comprime (20% de su escala original)

// Animacion viento
float windTime = 0.0f;       // Acumulador de tiempo
float windSpeed = 1.4f;      // Velocidad del viento (1.0 = suave, 3.0 = rápido)
float windIntensity = 0.1f; // Fuerza del viento (0.1 = sutil, 0.5 = fuerte)
glm::mat4 applyTreeWind(glm::mat4 model, float offset) {
	// Rotación en Z (inclinación del árbol)
	float tilt = sin(windTime * windSpeed + offset) * windIntensity;
	model = glm::rotate(model, tilt, glm::vec3(0.0f, 0.0f, 1.0f));

	// Escalado en X/Y para simular hojas moviéndose
	float scaleX = 1.0f + sin(windTime * windSpeed * 2.0f + offset) * windIntensity * 0.1f;
	float scaleY = 1.0f - sin(windTime * windSpeed * 2.0f + offset) * windIntensity * 0.05f;
	model = glm::scale(model, glm::vec3(scaleX, scaleY, 1.0f));

	return model;
}

// Animacion hojas
float hojasYOffset[3] = { 0.0f, 0.0f, 0.0f };
float hojasVelocidadCaida = 13.0f;
float hojasLimiteInferiorBase = -20.0f; // Límite base fijo que vamos a modificar
float hojasLimiteInferiorMaxFactor = 2.0f; // Factor máximo para multiplicar el límite base
float hojasLimiteSuperior = 22.0f;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> disFactor(1.0f, hojasLimiteInferiorMaxFactor);


// Linterna/Spotlight
bool flashlightOn = true;
bool keyPressed3 = false; // Para controlar el estado de la tecla "1"

// Parámetros del spotlight
struct Spotlight {
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float cutOff;
	float outerCutOff;
	float constant;
	float linear;
	float quadratic;
};

Spotlight flashlight = {
	glm::vec3(0.0f),
	glm::vec3(0.0f),
	glm::vec3(0.2f),
	glm::vec3(1.5f, 1.5f, 1.3f),
	glm::vec3(2.0f),
	glm::cos(glm::radians(16.0f)),
	glm::cos(glm::radians(30.0f)),
	1.0f,
	0.05f,
	0.008f
};

void UpdateFlashlight(Camera& camera, Spotlight& flashlight);

//Variables de audio 
ALCdevice* audioDevice = nullptr;
ALCcontext* audioContext = nullptr;
ALuint audioBuffer = 0;
ALuint audioSource = 0;

// Posición estática del audio en el mundo
glm::vec3 audioSourcePos(11.8f, 27.8f, -15.9f);

// Archivo de audio
const char* audioFile = "./audio/sintetizador_mono.wav";

// Control de reproducción
bool audioPlaying = false;
bool keyPressedM = false;

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
	Model Casa((char*)"Models/casa.obj");
	Model Arbusto((char*)"Models/arbusto.fbx");
	Model Arbol((char*)"Models/arbol.obj");
	Model Farol((char*)"Models/faro.obj");
	Model Cesped((char*)"Models/cesped.obj");
	Model Cielo((char*)"Models/cieloo.obj");
	Model Cart((char*)"Models/cart.obj");
	Model Hojas((char*)"Models/hojas.obj");

	// Modelos segundo piso
	Model Escaleras_Interior((char*)"Models/escaleras_interior.obj");

	// Modelos interior cocina
	Model Pared_Cocina((char*)"Models/pared_cocina.obj");
	Model Pared_Cocina_Anterior((char*)"Models/pared_cocina_anterior.obj");
	Model Piso((char*)"Models/piso_cocina.obj");
	Model Puerta_Cocina((char*)"Models/puerta_cocina.obj");
	Model Ventana_Cocina((char*)"Models/ventana_cocina.obj");
	Model Pared_Madera((char*)"Models/pared_madera.obj");
	Model Alacena((char*)"Models/alacena.obj");
	Model Alacena_Superior((char*)"Models/alacena_superior.obj");
	Model Alacena_Grande((char*)"Models/alacena_grande.obj");
	Model Campana((char*)"Models/campana.obj");
	Model Estufa((char*)"Models/estufa.obj");
	Model Mesa((char*)"Models/mesa.obj");
	Model Refrigerador((char*)"Models/refrigerador.obj");
	Model Silla((char*)"Models/silla.obj");
	Model Tostadora((char*)"Models/tostadora.obj");

	// Modelos interior habitacion
	Model Archivero((char*)"Models/archivero.obj");
	Model Buro((char*)"Models/buro.obj");
	Model Cajones((char*)"Models/cajones.obj");
	Model Cama((char*)"Models/cama.obj");
	Model Pared_Habitacion((char*)"Models/pared_habitacion.obj");
	Model Pared_Habitacion_Puerta((char*)"Models/pared_habitacion_puerta.obj");
	Model Television_Habitacion((char*)"Models/television_habitacion.obj");
	Model Trampolin((char*)"Models/trampolin.obj");
	Model Ventana_Habitacion((char*)"Models/ventana_habitacion.obj");
	Model Rigby((char*)"Models/rigby.obj");

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

	if (!initAudio(audioFile)) {
		std::cerr << "Error inicializando audio\n";
		return -1;
	}

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Llamada a funciones de animacion
		animateCircularDrift(deltaTime);
		UpdateDayNightTransition(deltaTime);
		windTime += deltaTime;
		actualizarCaidaHojas(deltaTime);

		updateListener(camera.GetPosition(), camera.GetFront());

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

		// Interpolación progresiva basada en timeOfDay
		float ambientFactor = glm::smoothstep(0.0f, 1.0f, timeOfDay);

		glm::vec3 dayLightDir(-0.2f, -1.0f, -0.3f);
		glm::vec3 nightLightDir(0.2f, -1.0f, 0.3f);
		glm::vec3 currentLightDir = glm::normalize(glm::mix(dayLightDir, nightLightDir, ambientFactor));

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"),
			currentLightDir.x, currentLightDir.y, currentLightDir.z);

		glm::vec3 currentAmbient = glm::mix(glm::vec3(0.5f), glm::vec3(0.05f, 0.05f, 0.15f), ambientFactor);
		glm::vec3 currentDiffuse = glm::mix(glm::vec3(0.8f, 0.8f, 0.7f), glm::vec3(0.1f, 0.1f, 0.25f), ambientFactor);
		glm::vec3 currentSpecular = glm::mix(glm::vec3(0.5f), glm::vec3(0.1f, 0.1f, 0.2f), ambientFactor);

		float currentShininess = glm::mix(32.0f, 64.0f, ambientFactor);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), currentAmbient.x, currentAmbient.y, currentAmbient.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), currentDiffuse.x, currentDiffuse.y, currentDiffuse.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), currentSpecular.x, currentSpecular.y, currentSpecular.z);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), currentShininess);

		// Luces puntuales: se mantienen, activadas o no según lightsOff
		float lightsOffFactor = lightsOff ? 0.0f : 1.0f;

		for (int i = 0; i < 3; ++i) {
			glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + std::to_string(i) + "].position").c_str()),
				pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
		}

		// Luz puntual 1
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.2f * lightsOffFactor, 0.2f * lightsOffFactor, 0.2f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 1.0f * lightsOffFactor, 1.0f * lightsOffFactor, 1.0f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f * lightsOffFactor, 0.5f * lightsOffFactor, 0.5f * lightsOffFactor);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.032f);

		// Luz puntual 2
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.5f * lightsOffFactor, 0.5f * lightsOffFactor, 0.1f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), 5.0f * lightsOffFactor, 5.0f * lightsOffFactor, 5.0f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 2.5f * lightsOffFactor, 2.2f * lightsOffFactor, 1.3f * lightsOffFactor);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.5f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.02f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.01f);

		// Luz puntual 3
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), 0.5f * lightsOffFactor, 0.5f * lightsOffFactor, 0.1f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), 0.3f * lightsOffFactor, 0.3f * lightsOffFactor, 0.3f * lightsOffFactor);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), 0.2f * lightsOffFactor, 0.2f * lightsOffFactor, 0.2f * lightsOffFactor);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.1f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.04f);

		// Spotlight
		// Actualiza la posición de la linterna
		UpdateFlashlight(camera, flashlight);

		// Configura el spotlight en el shader
		if (flashlightOn) {
			std::string base = "spotLight.";
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "position").c_str()),
				flashlight.position.x, flashlight.position.y, flashlight.position.z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "direction").c_str()),
				flashlight.direction.x, flashlight.direction.y, flashlight.direction.z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()),
				flashlight.ambient.x, flashlight.ambient.y, flashlight.ambient.z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()),
				flashlight.diffuse.x, flashlight.diffuse.y, flashlight.diffuse.z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()),
				flashlight.specular.x, flashlight.specular.y, flashlight.specular.z);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "cutOff").c_str()),
				flashlight.cutOff);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "outerCutOff").c_str()),
				flashlight.outerCutOff);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()),
				flashlight.constant);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()),
				flashlight.linear);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()),
				flashlight.quadratic);
		}
		else {
			// Apaga completamente la linterna: todos los componentes a cero
			std::string base = "spotLight.";
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "position").c_str()), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "direction").c_str()), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()), 0.0f, 0.0f, 0.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "cutOff").c_str()), 0.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "outerCutOff").c_str()), 0.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()), 1.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()), 0.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()), 0.0f);
		}

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
		//Arbustos
		glm::vec3 arbustoPosiciones[] = { glm::vec3(58.0f, -10.0f, 20.0f), glm::vec3(45.0f, -10.0f, 22.0f), glm::vec3(75.0f, -10.0f, 21.0f), glm::vec3(85.0f, -10.0f, 21.0f) };
		glm::vec3 arbustoEscalas[] = {
			glm::vec3(18.0f, 20.0f, 18.0f),
			glm::vec3(10.0f, 10.0f, 10.0f),
			glm::vec3(10.0f, 10.0f, 10.0f),
			glm::vec3(8.0f, 8.0f, 8.0f)
		};
		for (int i = 0; i < sizeof(arbustoPosiciones) / sizeof(arbustoPosiciones[0]); ++i) {
			model = glm::mat4(1);
			model = glm::translate(model, arbustoPosiciones[i]);
			model = glm::scale(model, arbustoEscalas[i]);
			model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = applyTreeWind(model, i * 0.5f); // Aplicar viento con un offset diferente para cada arbusto
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Arbusto.Draw(lightingShader);
		}
		//Arboles
		glm::vec3 arbolPosiciones[] = { glm::vec3(80.0f, -10.0f, -30.0f), glm::vec3(-90.0f, -10.0f, 30.0f), glm::vec3(-80.0f, -10.0f, -30.0f) };
		glm::vec3 arbolEscalas[] = { glm::vec3(12.0f, 12.0f, 12.0f), glm::vec3(12.0f, 12.0f, 12.0f), glm::vec3(10.0f, 10.0f, 10.0f) };
		for (int i = 0; i < sizeof(arbolPosiciones) / sizeof(arbolPosiciones[0]); ++i) {
			model = glm::mat4(1);
			model = glm::translate(model, arbolPosiciones[i]);
			model = glm::scale(model, arbolEscalas[i]);
			model = applyTreeWind(model, i * 0.5f); // Aplicar viento con un offset diferente para cada árbol
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Arbol.Draw(lightingShader);
		}
		//Hojas
		glm::vec3 hojasPosiciones[] = { glm::vec3(80.0f, -10.0f, -30.0f), glm::vec3(-90.0f, -10.0f, 30.0f), glm::vec3(-80.0f, -10.0f, -30.0f) };
		glm::vec3 hojasEscalas[] = { glm::vec3(12.0f, 12.0f, 12.0f), glm::vec3(12.0f, 12.0f, 12.0f), glm::vec3(10.0f, 10.0f, 10.0f) };
		for (int i = 0; i < sizeof(hojasPosiciones) / sizeof(hojasPosiciones[0]); ++i) {
			model = glm::mat4(1);
			// Aplica el desplazamiento vertical para la animación de caída
			glm::vec3 posConCaida = hojasPosiciones[i] + glm::vec3(0.0f, hojasYOffset[i], 0.0f);
			model = glm::translate(model, posConCaida);
			model = glm::scale(model, hojasEscalas[i]);
			model = applyTreeWind(model, i * 0.5f); // Aplica viento como antes
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Hojas.Draw(lightingShader);
		}

		////Faro
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-17.0f, -10.0f, 80.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Farol.Draw(lightingShader);

		//Cesped
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-10.0f, -13.0f, 20.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cesped.Draw(lightingShader);

		//Cielo
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-10.0f, -14.0f, 20.0f));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cielo.Draw(lightingShader);

		//Carrito de golf
		// Calcular posicion en el ci­rculo
		float carPosX = pivotPoint.x + circleRadius * sin(driftAngle);
		float carPosZ = pivotPoint.z + circleRadius * cos(driftAngle);
		// Orientacion del carrito (apuntando tangente al ci­rculo)
		float carRotation = driftAngle + glm::pi<float>() / 2; // 90 adicionales para orientacion correcta
		// Aplicar transformaciones
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(carPosX, pivotPoint.y, carPosZ));
		model = glm::rotate(model, carRotation, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotacion en Y
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cart.Draw(lightingShader);

		/////////////////////////////-Estructura-////////////////////////////////
		//Casa
		model = glm::mat4(1);
		model = glm::scale(model, glm::vec3(6.0f, 6.0f, 6.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Casa.Draw(lightingShader);
		//Pisos y techos
		//Piso, techo, piso cocina, techo cocina, piso habitacion, techo habitacion
		glm::vec3 pisosPosicion[] = { glm::vec3(-3.0f, 5.55f, 3.0f), glm::vec3(-1.8f, 11.2f, 2.65f), glm::vec3(-10.2f, 4.1f, -12.7f),
			glm::vec3(-10.2f, 13.0f, -12.7f), glm::vec3(6.8f, 25.4f, 0.2f), glm::vec3(3.0f, 35.3f, 0.19f) };
		glm::vec3 pisosEscala[] = { glm::vec3(2.7f, 1.0f, 1.9f), glm::vec3(2.1f, 5.5f, 1.75f), glm::vec3(1.28f, 1.0f, 0.84f),
			glm::vec3(1.28f, 1.0f, 0.84f), glm::vec3(1.75f, 1.0f, 1.35f), glm::vec3(2.155f, 1.0f, 1.36f) };
		float radianes[] = { 180.0f, 0.0f, 90.0f, 90.0f, 0.0f, 0.0f };
		glm::vec3 pisosRotacion[] = { glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
		for (int i = 0; i < sizeof(pisosPosicion) / sizeof(pisosPosicion[0]); ++i) {
			model = glm::mat4(1);
			model = glm::translate(model, pisosPosicion[i]);
			model = glm::scale(model, pisosEscala[i]);
			model = glm::rotate(model, glm::radians(radianes[i]), pisosRotacion[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Piso.Draw(lightingShader);
		}
		//Escaleras inferior
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(5.9f, 3.5f, -5.1f));
		model = glm::scale(model, glm::vec3(1.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Escaleras_Interior.Draw(lightingShader);

		// Escaleras superior
		model = glm::mat4(1);
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.2f));
		model = glm::translate(model, glm::vec3(2.0f, 12.5f, 2.7f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Escaleras_Interior.Draw(lightingShader);

		////////////////////////-Cocina-////////////////////////////////
		////////////////////////-Estructura-////////////////////////////////
		//Paredes
		// Pared izquierda, derecha, posterior
		glm::vec3 paredesCocinaPosicion[] = { glm::vec3(-15.57f, 3.1f, -9.6f), glm::vec3(12.0f, 3.1f, -9.6f), glm::vec3(-10.2f, 3.1f, -32.3f) };
		glm::vec3 paredesCocinaEscala[] = { glm::vec3(1.0f, 1.86f, 0.85f), glm::vec3(1.0f, 1.86f, 0.85f), glm::vec3(1.0f, 1.86f, 1.28f) };
		float paredesCocinaRadianes[] = { 0.0f, 0.0f, 90.0f, 90.0f };
		glm::vec3 paredesCocinaRotacion[] = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
		for (int i = 0; i < sizeof(paredesCocinaPosicion) / sizeof(paredesCocinaPosicion[0]); ++i) {
			model = glm::mat4(1);
			model = glm::translate(model, paredesCocinaPosicion[i]);
			model = glm::rotate(model, glm::radians(paredesCocinaRadianes[i]), paredesCocinaRotacion[i]);
			model = glm::scale(model, paredesCocinaEscala[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Pared_Cocina.Draw(lightingShader);
		}
		//Pared anterior
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-10.2f, 3.1f, -14.2f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.86f, 1.28f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Pared_Cocina_Anterior.Draw(lightingShader);
		////Ventana
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-21.2f, 10.2f, -20.37f));
		model = glm::scale(model, glm::vec3(3.35f, 3.35f, 1.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Ventana_Cocina.Draw(lightingShader);
		//Puerta interior
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-4.8f, 7.2f, -20.15f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Puerta_Cocina.Draw(lightingShader);
		//Puerta exterior
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-4.8f, 7.2f, -21.35f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Puerta_Cocina.Draw(lightingShader);
		//Pared Madera 1
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-7.9f, 6.2f, -20.3f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Pared_Madera.Draw(lightingShader);
		//Pared Madera 2
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-1.6f, 6.2f, -20.3f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Pared_Madera.Draw(lightingShader);

		//////////////////////////-Objetos-////////////////////////////////
		//Alacenas
		glm::vec3 alacenasPosiciones[] = { glm::vec3(-19.0f, 4.74f, -17.57f), glm::vec3(-21.1f, 4.74f, -14.0f), glm::vec3(-19.0f, 4.74f, -10.5f), glm::vec3(-17.1f, 4.74f, -14.0f), glm::vec3(-13.1f, 4.74f, -14.0f) };
		glm::vec3 alacenasEscalas[] = { glm::vec3(1.2f, 1.0f, 1.0f), glm::vec3(1.2f, 1.0f, 1.0f), glm::vec3(1.2f, 1.0f, 1.0f), glm::vec3(1.2f, 1.0f, 1.0f), glm::vec3(1.2f, 1.0f, 1.0f) };
		glm::vec3 alacenasRotaciones[] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 270.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 270.0f, 0.0f), glm::vec3(0.0f, 270.0f, 0.0f) };

		for (int i = 0; i < sizeof(alacenasPosiciones) / sizeof(alacenasPosiciones[0]); ++i) {
			model = glm::mat4(1);
			model = glm::translate(model, alacenasPosiciones[i]);
			model = glm::scale(model, alacenasEscalas[i]);
			model = glm::rotate(model, glm::radians(alacenasRotaciones[i].y), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Alacena.Draw(lightingShader);
		}

		//Alacena Superior
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-15.0f, 11.5f, -19.1f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Alacena_Superior.Draw(lightingShader);
		//Alacena Grande
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-25.5f, 9.8f, -17.0f));
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Alacena_Grande.Draw(lightingShader);

		////Estufa
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-21.15f, 5.91f, -23.85f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Estufa.Draw(lightingShader);
		////Campana
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-19.2f, 6.1f, -23.4f));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 200.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Campana.Draw(lightingShader);
		//Refrigerador
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-16.8f, 4.7f, -19.1f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Refrigerador.Draw(lightingShader);
		//Mesa
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-11.3, 6.82f, -11.75));
		model = glm::scale(model, glm::vec3(0.85f, 0.9f, 0.85f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Mesa.Draw(lightingShader);
		//Silla 1
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-15.3, 6.4f, -5.0f));
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		model = glm::rotate(model, glm::radians(315.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Silla.Draw(lightingShader);
		//Silla 2
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-24.3, 6.4f, -9.6f));
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
		model = glm::rotate(model, glm::radians(225.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Silla.Draw(lightingShader);
		//Tostadora
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-11.5f, 4.74f, -16.5f));
		model = glm::rotate(model, glm::radians(315.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Tostadora.Draw(lightingShader);

		// Segundo piso


		//////////////////////-Habitacion-////////////////////////////////
		//////////////////////-Estructura-////////////////////////////////
		//Paredes 
		// Posterior, derecha, anterior
		glm::vec3 paredesHabitacionPosicion[] = { glm::vec3(16.5f, 28.5f, -19.7f), glm::vec3(25.3f, 28.5f, -6.6f), glm::vec3(16.5f, 28.5f, -1.5f) };
		glm::vec3 paredesHabitacionEscala[] = { glm::vec3(3.2f, 3.2f, 1.5f),glm::vec3(3.2f, 3.2f, 1.5f), glm::vec3(3.2f, 3.2f, 1.5f) };
		float paredesHabitacionRadianes[] = { 0.0f, 270.0f, 0.0f };
		glm::vec3 paredesHabitacionRotacion[] = { glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f) };
		for (int i = 0; i < sizeof(paredesHabitacionPosicion) / sizeof(paredesHabitacionPosicion[0]); ++i) {
			model = glm::mat4(1);
			model = glm::translate(model, paredesHabitacionPosicion[i]);
			model = glm::rotate(model, glm::radians(paredesHabitacionRadianes[i]), paredesHabitacionRotacion[i]);
			model = glm::scale(model, paredesHabitacionEscala[i]);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			Pared_Habitacion.Draw(lightingShader);
		}
		//Con puerta
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(3.9f, 30.4f, -9.3f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Pared_Habitacion_Puerta.Draw(lightingShader);

		//Ventana Habitacion 1
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(10.0f, 32.0f, -17.6f));
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Ventana_Habitacion.Draw(lightingShader);
		//Ventana Habitacion 2
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(19.0f, 32.0f, -17.6f));
		model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Ventana_Habitacion.Draw(lightingShader);


		////////////////////////-Objetos-////////////////////////////////
		//Cama
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(8.0f, 27.2f, -10.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cama.Draw(lightingShader);
		//Buro 
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(11.8f, 27.5f, -15.9f));
		model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Buro.Draw(lightingShader);
		//Trampolin
		model = glm::mat4(1);
		model = glm::translate(model, trampolinPos);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Trampolin.Draw(lightingShader);
		// Trampolin Jump
		updateTrampolineJump(deltaTime);
		float jumpYOffset = jumpHeight * sin(jumpProgress * glm::pi<float>());
		glm::vec3 rigbyPos = rigbyBasePos + glm::vec3(0.0f, jumpYOffset, 0.0f);
		// Rigby
		float squashY = 1.0f - (squashFactor * squashIntensity);
		float stretchXZ = 1.0f + (squashFactor * squashIntensity * 0.5f); // Efecto secundario en X/Z
		model = glm::mat4(1);
		model = glm::translate(model, rigbyPos);
		//model = glm::rotate(model, jumpProgress * glm::radians(360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(rigbyScale * stretchXZ, rigbyScale * squashY, rigbyScale * stretchXZ));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Rigby.Draw(lightingShader);
		//Archivero
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(22.2f, 28.4f, -10.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Archivero.Draw(lightingShader);
		//Cajones
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(22.2f, 28.0f, -7.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cajones.Draw(lightingShader);
		//Television
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(22.0f, 30.55f, -7.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Television_Habitacion.Draw(lightingShader);

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

	cleanupAudio();
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
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		isNight = !isNight;
		transitionActive = true; // inicia la animación
	}


	if (keys[GLFW_KEY_L] && !keyPressed2) {
		lightsOff = !lightsOff;
		keyPressed2 = true;
	}
	else if (!keys[GLFW_KEY_L]) {
		keyPressed2 = false;
	}

	if (keys[GLFW_KEY_F] && !keyPressed3) {
		flashlightOn = !flashlightOn;
		keyPressed3 = true;
	}
	else if (!keys[GLFW_KEY_F]) {
		keyPressed3 = false;
	}

	if (key == GLFW_KEY_M && action == GLFW_PRESS && !keyPressedM) {
		toggleAudioPlayback();
		keyPressedM = true;
	}
	if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
		keyPressedM = false;
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

void UpdateDayNightTransition(float deltaTime) {
	if (!transitionActive) return;

	float target = isNight ? 1.0f : 0.0f;
	float direction = (target > timeOfDay) ? 1.0f : -1.0f;

	timeOfDay += direction * deltaTime * transitionSpeed;

	// Clamp y finalización
	if ((direction > 0.0f && timeOfDay >= target) || (direction < 0.0f && timeOfDay <= target)) {
		timeOfDay = target;
		transitionActive = false;  // transición terminada
	}
}


void updateTrampolineJump(float deltaTime) {
	if (isAscending) {
		jumpProgress += jumpSpeed * deltaTime;
		if (jumpProgress >= 1.0f) {
			jumpProgress = 1.0f;
			isAscending = false;
		}
		// Squash disminuye durante el ascenso
		squashFactor = 1.0f - jumpProgress;
	}
	else {
		jumpProgress -= jumpSpeed * deltaTime;
		if (jumpProgress <= 0.0f) {
			jumpProgress = 0.0f;
			isAscending = true;
		}
		// Squash aumenta durante el descenso
		squashFactor = jumpProgress;
	}
}

void UpdateFlashlight(Camera& camera, Spotlight& flashlight) {
	flashlight.position = camera.GetPosition();
	flashlight.direction = camera.GetFront();
}

void actualizarCaidaHojas(float deltaTime) {
	for (int i = 0; i < 3; ++i) {
		hojasYOffset[i] -= hojasVelocidadCaida * deltaTime;

		// Multiplica el límite base por un factor aleatorio en cada reinicio
		float limiteInferiorAleatorio = hojasLimiteInferiorBase * disFactor(gen);

		if (hojasYOffset[i] < limiteInferiorAleatorio) {
			hojasYOffset[i] = hojasLimiteSuperior;
			// No necesitas almacenar el límite, lo recalculas cada vez
		}
	}
}

bool loadWavFile(const char* filename, ALuint buffer) {
	drwav wav;
	if (!drwav_init_file(&wav, filename, NULL)) {
		std::cerr << "Error al cargar el archivo WAV: " << filename << std::endl;
		return false;
	}

	float* pSampleData = (float*)malloc(wav.totalPCMFrameCount * wav.channels * sizeof(float));
	drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);

	short* pcmData = (short*)malloc(wav.totalPCMFrameCount * wav.channels * sizeof(short));
	for (drwav_uint64 i = 0; i < wav.totalPCMFrameCount * wav.channels; ++i) {
		pcmData[i] = (short)(pSampleData[i] * 32767.0f);
	}

	ALenum format;
	if (wav.channels == 1)
		format = AL_FORMAT_MONO16;
	else if (wav.channels == 2)
		format = AL_FORMAT_STEREO16;
	else {
		std::cerr << "Formato no soportado: " << wav.channels << " canales" << std::endl;
		drwav_uninit(&wav);
		free(pSampleData);
		free(pcmData);
		return false;
	}

	alBufferData(buffer, format, pcmData, wav.totalPCMFrameCount * wav.channels * sizeof(short), wav.sampleRate);

	drwav_uninit(&wav);
	free(pSampleData);
	free(pcmData);
	return true;
}

bool initAudio(const char* wavFile) {
	audioDevice = alcOpenDevice(nullptr);
	if (!audioDevice) return false;

	audioContext = alcCreateContext(audioDevice, nullptr);
	if (!audioContext || !alcMakeContextCurrent(audioContext)) {
		if (audioContext) alcDestroyContext(audioContext);
		alcCloseDevice(audioDevice);
		return false;
	}

	alGenBuffers(1, &audioBuffer);
	alGenSources(1, &audioSource);

	if (!loadWavFile(wavFile, audioBuffer)) return false;

	alSourcei(audioSource, AL_BUFFER, audioBuffer);
	alSourcei(audioSource, AL_LOOPING, AL_TRUE);

	// Configurar atenuación 3D
	alSourcef(audioSource, AL_REFERENCE_DISTANCE, 7.0f); //Distancia a la que el volumen es máximo (sin atenuación).
	alSourcef(audioSource, AL_ROLLOFF_FACTOR, 2.0f); //Qué tan rápido cae el volumen con la distancia.
	alSourcef(audioSource, AL_MAX_DISTANCE, 25.0f); //Distancia máxima donde la fuente puede oírse (después de esta distancia el volumen es cero).

	// Posición fija
	alSource3f(audioSource, AL_POSITION, audioSourcePos.x, audioSourcePos.y, audioSourcePos.z);

	return true;
}


void updateListener(const glm::vec3& listenerPos, const glm::vec3& listenerDir) {
	// Calcula la distancia entre cámara y fuente de audio
	float distance = glm::distance(listenerPos, audioSourcePos);

	ALint state;
	alGetSourcei(audioSource, AL_SOURCE_STATE, &state);

	if (distance > 40.0f) {
		// Pausa la fuente solo si está sonando y el usuario NO la ha pausado manualmente
		if (state == AL_PLAYING && audioPlaying) {
			alSourcePause(audioSource);
			//std::cout << "Audio pausado automáticamente por distancia\n";
		}
	}
	else {
		// Reproduce la fuente solo si está pausada y el usuario quiere que suene (audioPlaying == true)
		if (state != AL_PLAYING && audioPlaying) {
			alSourcePlay(audioSource);
			//std::cout << "Audio reanudado automáticamente por proximidad\n";
		}
	}

	// Actualiza posición y orientación del listener (cámara)
	alListener3f(AL_POSITION, listenerPos.x, listenerPos.y, listenerPos.z);

	float orientation[6] = {
		listenerDir.x, listenerDir.y, listenerDir.z,
		0.0f, 1.0f, 0.0f
	};
	alListenerfv(AL_ORIENTATION, orientation);

	// Debug (opcional)
	//std::cout << "Distancia: " << distance << ", Estado fuente: " << state << ", audioPlaying: " << audioPlaying << std::endl;
}


void toggleAudioPlayback() {
	ALint state;
	alGetSourcei(audioSource, AL_SOURCE_STATE, &state);

	if (state == AL_PLAYING) {
		alSourcePause(audioSource);
		audioPlaying = false;
	}
	else {
		alSourcePlay(audioSource);
		audioPlaying = true;
	}
}


void cleanupAudio() {
	alSourceStop(audioSource);
	alDeleteSources(1, &audioSource);
	alDeleteBuffers(1, &audioBuffer);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(audioContext);
	alcCloseDevice(audioDevice);
}
