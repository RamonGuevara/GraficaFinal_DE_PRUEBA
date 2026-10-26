# Arena de Hockey sobre Hielo

Proyecto Final de Computación Gráfica e Interacción Humano-Computadora (FI UNAM).

## Qué tiene hasta ahora

- **Zona 1, pista de juego:** hielo de 61 × 26 m con sus marcas, vallas con publicidad, cristal protector y dos porterías con red.
- **Zona 2, bancas:** bancas de los dos equipos con toallas, botellas y palos; cajas de castigo y mesa de oficiales.
- **Zona 3, espectadores:** gradas de 5 escalones en los cuatro lados, público y marcador central colgante.
- **Iluminación:** dos luces (como en la práctica de iluminación) que funcionan como reflectores.

## Controles

| Tecla | Acción |
| --- | --- |
| W A S D | Moverse |
| Mouse | Mirar |
| L | Encender y apagar los reflectores |
| 1 / 2 | Gol del local / visitante |
| 0 | Reiniciar el marcador |
| ESC | Salir |

## Archivos

- `Main.cpp`: todo el programa.
- `Shader.h`, `Camera.h`, `Mesh.h`, `Model.h`, `stb_image.h`: clases de las prácticas.
- `Shader/`: `lighting.vs`, `lighting.frag`, `lamp.vs`, `lamp.frag`.
- `Textures/`: hielo, vallas, red, cristal, marcador y números del 0 al 9.
- `Models/`: aficionados, palo, botella y disco (`.obj`).

## Cómo abrirlo en Visual Studio 2022

Igual que la práctica de carga de modelos:

1. Proyecto vacío de C++; copiar todo esto junto al `.vcxproj`.
2. Agregar `Main.cpp` y los `.h` al proyecto.
3. Propiedades: rutas de GLEW, GLFW, GLM y Assimp, y en Dependencias adicionales `opengl32.lib; glew32.lib; glfw3.lib;` más el `.lib` de Assimp.
4. Plataforma x86.
5. Copiar `glew32.dll` y la `.dll` de Assimp junto al `.exe`.
6. F5.
