/*
 * Proyecto Final - Escenario Deportivo 3D Interactivo
 * Arena de Hockey sobre Hielo
 * Laboratorio de Computación Gráfica e Interacción Humano-Computadora, FI UNAM
 *
 * Zonas del escenario:
 *   1. Pista de juego: hielo con sus marcas, vallas, cristal protector y porterías.
 *   2. Zona de bancas: bancas de los equipos (atrás de la pista) y cajas de castigo
 *      con la mesa de oficiales (al frente).
 *   3. Zona de espectadores: gradas en los cuatro lados con público y el marcador central.
 *
 * Medidas: 1 unidad = 1 metro. La pista mide 61 x 26 m (reglamento NHL).
 *
 * Controles:
 *   W A S D  -> moverse         Mouse -> mirar
 *   L        -> encender/apagar los reflectores
 *   1 / 2    -> gol del equipo local / visitante     0 -> reiniciar marcador
 *   ESC      -> salir
 */

#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Otras librerías
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Prototipos
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();
GLuint CargarTextura(const char *ruta);
void DibujarCubo(Shader &shader, glm::vec3 posicion, glm::vec3 escala, glm::vec3 color, GLuint textura, float rotacionY = 0.0f);

// Dimensiones de la ventana
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Cámara (empieza como espectador en las gradas)
Camera camera(glm::vec3(0.0f, 3.1f, 18.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -12.0f);
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Luces (reflectores del techo)
glm::vec3 lightPos(-15.0f, 10.0f, 0.0f);
glm::vec3 lightPos2(15.0f, 10.0f, 0.0f);
bool reflectores = true;

// Marcador
int golesLocal = 0;
int golesVisita = 0;

// Tiempo entre cuadros
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Colores
glm::vec3 blanco(1.0f, 1.0f, 1.0f);
glm::vec3 rojo(0.75f, 0.10f, 0.12f);
glm::vec3 azul(0.10f, 0.25f, 0.65f);
glm::vec3 gris(0.45f, 0.45f, 0.48f);
glm::vec3 grisOscuro(0.15f, 0.15f, 0.18f);
glm::vec3 madera(0.55f, 0.38f, 0.22f);

int main()
{
	// Inicializar GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto Final - Arena de Hockey", nullptr, nullptr);
	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Funciones de teclado y mouse
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	// Transparencia para el cristal protector
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

	// Modelos
	Model aficionadoLocal((char *)"Models/aficionado_local.obj");
	Model aficionadoVisita((char *)"Models/aficionado_visita.obj");
	Model palo((char *)"Models/palo.obj");
	Model botella((char *)"Models/botella.obj");
	Model disco((char *)"Models/disco.obj");

	// Cubo: posición, normal y coordenada de textura
	GLfloat vertices[] =
	{
		// Atrás
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		// Frente
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		// Izquierda
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		// Derecha
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		// Abajo
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		// Arriba
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	// Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Coordenadas de textura
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// Texturas
	GLuint texHielo = CargarTextura("Textures/pista_hielo.png");
	GLuint texValla1 = CargarTextura("Textures/valla1.png");
	GLuint texValla2 = CargarTextura("Textures/valla2.png");
	GLuint texRed = CargarTextura("Textures/red.png");
	GLuint texCristal = CargarTextura("Textures/cristal.png");
	GLuint texBlanco = CargarTextura("Textures/blanco.png");
	GLuint texMarcador = CargarTextura("Textures/marcador.png");
	GLuint texNumeros[10];
	for (int i = 0; i < 10; i++)
	{
		std::string ruta = "Textures/numeros/" + std::to_string(i) + ".png";
		texNumeros[i] = CargarTextura(ruta.c_str());
	}

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 200.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();

		glClearColor(0.05f, 0.06f, 0.09f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightingShader.Use();
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// Propiedades de las luces: si los reflectores están apagados, solo queda una luz tenue
		glm::vec3 ambiente = reflectores ? glm::vec3(0.2f) : glm::vec3(0.05f);
		glm::vec3 difusa = reflectores ? glm::vec3(0.45f) : glm::vec3(0.03f);
		glm::vec3 especular = reflectores ? glm::vec3(0.3f) : glm::vec3(0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "light.ambient"), 1, glm::value_ptr(ambiente));
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "light.diffuse"), 1, glm::value_ptr(difusa));
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "light.specular"), 1, glm::value_ptr(especular));
		glUniform3f(glGetUniformLocation(lightingShader.Program, "light2.position"), lightPos2.x, lightPos2.y, lightPos2.z);
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "light2.ambient"), 1, glm::value_ptr(ambiente));
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "light2.diffuse"), 1, glm::value_ptr(difusa));
		glUniform3fv(glGetUniformLocation(lightingShader.Program, "light2.specular"), 1, glm::value_ptr(especular));

		// Material (el color de cada objeto se cambia en DibujarCubo)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.3f, 0.3f, 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		// Matrices de vista y proyección
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		// ======================= ZONA 1: PISTA DE JUEGO =======================

		// Piso de concreto
		DibujarCubo(lightingShader, glm::vec3(0.0f, -0.05f, 0.0f), glm::vec3(90.0f, 0.1f, 50.0f), gris, texBlanco);

		// Hielo (la textura ya trae las líneas, círculos y áreas de portero)
		DibujarCubo(lightingShader, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(60.96f, 0.02f, 25.91f), blanco, texHielo);

		// Vallas de los lados largos: 15 tramos de 4.064 m con publicidad
		for (int i = 0; i < 15; i++)
		{
			float x = -30.48f + 4.064f * (i + 0.5f);
			GLuint tex = (i % 2 == 0) ? texValla1 : texValla2;
			DibujarCubo(lightingShader, glm::vec3(x, 0.535f, -13.03f), glm::vec3(4.064f, 1.07f, 0.15f), blanco, tex, 0.0f);
			DibujarCubo(lightingShader, glm::vec3(x, 0.535f, 13.03f), glm::vec3(4.064f, 1.07f, 0.15f), blanco, tex, 180.0f);
		}
		// Vallas de los fondos: 6 tramos de 4.318 m (se giran para que la publicidad mire a la pista)
		for (int i = 0; i < 6; i++)
		{
			float z = -12.955f + 4.318f * (i + 0.5f);
			GLuint tex = (i % 2 == 0) ? texValla2 : texValla1;
			DibujarCubo(lightingShader, glm::vec3(30.555f, 0.535f, z), glm::vec3(4.318f, 1.07f, 0.15f), blanco, tex, -90.0f);
			DibujarCubo(lightingShader, glm::vec3(-30.555f, 0.535f, z), glm::vec3(4.318f, 1.07f, 0.15f), blanco, tex, 90.0f);
		}

		// Porterías: una en cada línea de gol (a 3.35 m del fondo)
		for (int lado = -1; lado <= 1; lado += 2)
		{
			float gx = 27.13f * lado;      // línea de gol
			float atras = 1.0f * lado;     // la red va hacia el fondo de la pista
			// Postes y travesaño (rojos)
			DibujarCubo(lightingShader, glm::vec3(gx, 0.61f, -0.915f), glm::vec3(0.06f, 1.22f, 0.06f), rojo, texBlanco);
			DibujarCubo(lightingShader, glm::vec3(gx, 0.61f, 0.915f), glm::vec3(0.06f, 1.22f, 0.06f), rojo, texBlanco);
			DibujarCubo(lightingShader, glm::vec3(gx, 1.22f, 0.0f), glm::vec3(0.06f, 0.06f, 1.89f), rojo, texBlanco);
			// Red: atrás, arriba y a los lados
			DibujarCubo(lightingShader, glm::vec3(gx + atras, 0.61f, 0.0f), glm::vec3(0.02f, 1.22f, 1.83f), blanco, texRed);
			DibujarCubo(lightingShader, glm::vec3(gx + atras / 2, 1.22f, 0.0f), glm::vec3(1.0f, 0.02f, 1.83f), blanco, texRed);
			DibujarCubo(lightingShader, glm::vec3(gx + atras / 2, 0.61f, -0.915f), glm::vec3(1.0f, 1.22f, 0.02f), blanco, texRed);
			DibujarCubo(lightingShader, glm::vec3(gx + atras / 2, 0.61f, 0.915f), glm::vec3(1.0f, 1.22f, 0.02f), blanco, texRed);
		}

		// ================= ZONA 2: BANCAS, CASTIGO Y OFICIALES =================

		// Bancas de los equipos (detrás de la valla de atrás, z negativo)
		DibujarCubo(lightingShader, glm::vec3(0.0f, 0.03f, -14.35f), glm::vec3(24.0f, 0.06f, 2.5f), grisOscuro, texBlanco);  // piso de hule
		for (int equipo = -1; equipo <= 1; equipo += 2)
		{
			float xc = 7.0f * equipo;
			glm::vec3 color = (equipo < 0) ? rojo : azul;
			DibujarCubo(lightingShader, glm::vec3(xc, 0.7f, -15.6f), glm::vec3(9.4f, 1.4f, 0.1f), color, texBlanco);         // muro de atrás
			DibujarCubo(lightingShader, glm::vec3(xc - 4.7f, 0.6f, -14.35f), glm::vec3(0.1f, 1.2f, 2.5f), color, texBlanco);  // muros de los lados
			DibujarCubo(lightingShader, glm::vec3(xc + 4.7f, 0.6f, -14.35f), glm::vec3(0.1f, 1.2f, 2.5f), color, texBlanco);
			DibujarCubo(lightingShader, glm::vec3(xc, 0.42f, -15.05f), glm::vec3(9.0f, 0.08f, 0.45f), madera, texBlanco);    // asiento
			DibujarCubo(lightingShader, glm::vec3(xc, 0.2f, -15.05f), glm::vec3(8.6f, 0.4f, 0.3f), grisOscuro, texBlanco);   // base
			// Toallas
			for (int i = -3; i <= 3; i += 3)
				DibujarCubo(lightingShader, glm::vec3(xc + i + 0.6f, 0.47f, -15.05f), glm::vec3(0.4f, 0.02f, 0.3f), blanco, texBlanco);
		}

		// Cajas de castigo y mesa de oficiales (al frente, z positivo)
		DibujarCubo(lightingShader, glm::vec3(0.0f, 0.03f, 14.35f), glm::vec3(12.0f, 0.06f, 2.5f), grisOscuro, texBlanco);   // piso
		DibujarCubo(lightingShader, glm::vec3(0.0f, 0.7f, 15.6f), glm::vec3(12.0f, 1.4f, 0.1f), gris, texBlanco);            // muro de atrás
		DibujarCubo(lightingShader, glm::vec3(-4.2f, 0.42f, 15.05f), glm::vec3(3.2f, 0.08f, 0.45f), rojo, texBlanco);        // banca castigo local
		DibujarCubo(lightingShader, glm::vec3(4.2f, 0.42f, 15.05f), glm::vec3(3.2f, 0.08f, 0.45f), azul, texBlanco);         // banca castigo visitante
		DibujarCubo(lightingShader, glm::vec3(0.0f, 0.75f, 13.8f), glm::vec3(3.6f, 0.06f, 0.8f), blanco, texBlanco);         // mesa de oficiales
		DibujarCubo(lightingShader, glm::vec3(0.0f, 0.37f, 13.45f), glm::vec3(3.6f, 0.74f, 0.05f), grisOscuro, texBlanco);  // frente de la mesa
		for (int i = -1; i <= 1; i += 2)
		{
			DibujarCubo(lightingShader, glm::vec3(0.8f * i, 0.45f, 14.6f), glm::vec3(0.45f, 0.08f, 0.45f), grisOscuro, texBlanco);  // silla
			DibujarCubo(lightingShader, glm::vec3(0.8f * i, 0.75f, 14.82f), glm::vec3(0.45f, 0.55f, 0.05f), grisOscuro, texBlanco); // respaldo
		}

		// ===================== ZONA 3: ESPECTADORES =====================

		// Gradas: 5 escalones en cada lado; cada escalón es un cubo más alto que el anterior
		for (int fila = 0; fila < 5; fila++)
		{
			float alto = 0.5f * (fila + 1);
			glm::vec3 color = (fila % 2 == 0) ? gris : glm::vec3(0.38f, 0.38f, 0.42f);
			DibujarCubo(lightingShader, glm::vec3(0.0f, alto / 2, 16.5f + fila), glm::vec3(64.0f, alto, 1.0f), color, texBlanco);    // frente
			DibujarCubo(lightingShader, glm::vec3(0.0f, alto / 2, -16.5f - fila), glm::vec3(64.0f, alto, 1.0f), color, texBlanco);   // atrás
			DibujarCubo(lightingShader, glm::vec3(34.5f + fila, alto / 2, 0.0f), glm::vec3(1.0f, alto, 34.0f), color, texBlanco);    // fondo derecho
			DibujarCubo(lightingShader, glm::vec3(-34.5f - fila, alto / 2, 0.0f), glm::vec3(1.0f, alto, 34.0f), color, texBlanco);   // fondo izquierdo
		}

		// Marcador central colgante
		DibujarCubo(lightingShader, glm::vec3(0.0f, 13.0f, 0.0f), glm::vec3(5.0f, 2.6f, 2.0f), grisOscuro, texBlanco);     // cuerpo
		DibujarCubo(lightingShader, glm::vec3(-2.0f, 19.0f, 0.0f), glm::vec3(0.05f, 9.4f, 0.05f), gris, texBlanco);        // cables
		DibujarCubo(lightingShader, glm::vec3(2.0f, 19.0f, 0.0f), glm::vec3(0.05f, 9.4f, 0.05f), gris, texBlanco);
		// Pantallas (una de cada lado); se les pone un color mayor a 1 para que se vean brillantes
		glm::vec3 brillo(2.0f, 2.0f, 2.0f);
		DibujarCubo(lightingShader, glm::vec3(0.0f, 13.0f, 1.01f), glm::vec3(4.6f, 2.4f, 0.02f), brillo, texMarcador, 0.0f);
		DibujarCubo(lightingShader, glm::vec3(0.0f, 13.0f, -1.01f), glm::vec3(4.6f, 2.4f, 0.02f), brillo, texMarcador, 180.0f);
		// Números de los goles (cada número es una textura distinta)
		DibujarCubo(lightingShader, glm::vec3(-1.4f, 12.8f, 1.03f), glm::vec3(0.8f, 1.1f, 0.01f), brillo, texNumeros[golesLocal], 0.0f);
		DibujarCubo(lightingShader, glm::vec3(1.4f, 12.8f, 1.03f), glm::vec3(0.8f, 1.1f, 0.01f), brillo, texNumeros[golesVisita], 0.0f);
		DibujarCubo(lightingShader, glm::vec3(1.4f, 12.8f, -1.03f), glm::vec3(0.8f, 1.1f, 0.01f), brillo, texNumeros[golesLocal], 180.0f);
		DibujarCubo(lightingShader, glm::vec3(-1.4f, 12.8f, -1.03f), glm::vec3(0.8f, 1.1f, 0.01f), brillo, texNumeros[golesVisita], 180.0f);

		// ======================= MODELOS CARGADOS =======================
		glm::mat4 model(1);
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 1.0f, 1.0f, 1.0f);

		// Disco en el centro de la pista
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.012f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		disco.Draw(lightingShader);

		// Botellas y palos en las bancas
		for (int equipo = -1; equipo <= 1; equipo += 2)
		{
			float xc = 7.0f * equipo;
			for (int i = -4; i <= 4; i += 2)
			{
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(xc + i, 0.46f, -14.95f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				botella.Draw(lightingShader);
			}
			for (int i = 0; i < 4; i++)
			{
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(xc - 1.5f + i, 0.06f, -15.4f));
				model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // recargado en el muro
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				palo.Draw(lightingShader);
			}
		}

		// Público en las filas 1, 3 y 5 de las gradas (afición local a la izquierda, visitante a la derecha)
		for (int fila = 0; fila < 5; fila += 2)
		{
			float y = 0.5f * (fila + 1);
			// Lados largos
			for (float x = -27.5f; x <= 27.5f; x += 5.0f)
			{
				Model &aficionado = (x < 0) ? aficionadoLocal : aficionadoVisita;
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(x, y, 16.5f + fila));
				model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // mira hacia la pista
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				aficionado.Draw(lightingShader);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(x, y, -16.5f - fila));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				aficionado.Draw(lightingShader);
			}
			// Fondos
			for (float z = -12.5f; z <= 12.5f; z += 5.0f)
			{
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(34.5f + fila, y, z));
				model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				aficionadoVisita.Draw(lightingShader);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(-34.5f - fila, y, z));
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				aficionadoLocal.Draw(lightingShader);
			}
		}

		// ======================= LÁMPARAS (reflectores) =======================
		lampShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		if (reflectores)
		{
			glBindVertexArray(VAO);
			model = glm::mat4(1);
			model = glm::translate(model, lightPos);
			model = glm::scale(model, glm::vec3(1.0f));
			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);

			model = glm::mat4(1);
			model = glm::translate(model, lightPos2);
			model = glm::scale(model, glm::vec3(1.0f));
			glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// ============ CRISTAL PROTECTOR (al final porque es transparente) ============
		lightingShader.Use();
		glBindVertexArray(VAO);
		DibujarCubo(lightingShader, glm::vec3(0.0f, 1.97f, -13.03f), glm::vec3(60.96f, 1.8f, 0.02f), blanco, texCristal);
		DibujarCubo(lightingShader, glm::vec3(0.0f, 1.97f, 13.03f), glm::vec3(60.96f, 1.8f, 0.02f), blanco, texCristal);
		DibujarCubo(lightingShader, glm::vec3(30.555f, 1.97f, 0.0f), glm::vec3(0.02f, 1.8f, 25.91f), blanco, texCristal);
		DibujarCubo(lightingShader, glm::vec3(-30.555f, 1.97f, 0.0f), glm::vec3(0.02f, 1.8f, 25.91f), blanco, texCristal);
		// Separadores de cristal de las cajas de castigo
		DibujarCubo(lightingShader, glm::vec3(-2.45f, 1.1f, 14.35f), glm::vec3(0.03f, 2.2f, 2.5f), blanco, texCristal);
		DibujarCubo(lightingShader, glm::vec3(2.45f, 1.1f, 14.35f), glm::vec3(0.03f, 2.2f, 2.5f), blanco, texCristal);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}

// Dibuja el cubo con una posición, tamaño, color, textura y giro en Y
void DibujarCubo(Shader &shader, glm::vec3 posicion, glm::vec3 escala, glm::vec3 color, GLuint textura, float rotacionY)
{
	glm::mat4 model(1);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, escala);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(glGetUniformLocation(shader.Program, "material.diffuse"), color.x, color.y, color.z);
	glBindTexture(GL_TEXTURE_2D, textura);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

// Carga una imagen con stb_image y crea la textura
GLuint CargarTextura(const char *ruta)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int textureWidth, textureHeight, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *image = stbi_load(ruta, &textureWidth, &textureHeight, &nrChannels, 4);
	if (image)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "No se pudo cargar la textura: " << ruta << std::endl;
	}
	stbi_image_free(image);
	return texture;
}

// Movimiento de la cámara con el teclado
void DoMovement()
{
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}

	// Encender y apagar los reflectores
	if (GLFW_KEY_L == key && GLFW_PRESS == action)
		reflectores = !reflectores;

	// Marcador (del 0 al 9)
	if (GLFW_KEY_1 == key && GLFW_PRESS == action && golesLocal < 9)
		golesLocal++;
	if (GLFW_KEY_2 == key && GLFW_PRESS == action && golesVisita < 9)
		golesVisita++;
	if (GLFW_KEY_0 == key && GLFW_PRESS == action)
	{
		golesLocal = 0;
		golesVisita = 0;
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;
	lastX = xPos;
	lastY = yPos;
	camera.ProcessMouseMovement(xOffset, yOffset);
}
