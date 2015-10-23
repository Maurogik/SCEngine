# SCEngine

*A 3D engine I am currently working on.*

*This is still a Work In Progress and will be fleshed out and refined as I go.*

How to build (requires CMake) :
-----------------------------

#### Linux - Command Line
    
* **clone the repository**

   *git clone https://github.com/Maurogik/SCEngine.git*

* **go to the SCEngine folder and run the following commands**

	*mkdir build && cd build*
	
	*cmake ..*

	*make*

* **Run the executable**

    *./playground*

#### Windows - CMake and Visual Studio

* **clone or download the project**

* **Open CMake and browse to where you extracted the project**

* **Configure and Generate the build files with CMake (you will have to pick a build directory first)**

* **Open the generated SCEngine.sln solution with visual studio**

* **Build and run**


### Dependencies

* GLEW : loading OpenGL extensions
* GLFW : window creation OpenGL window context initialization and keyboard inputs
* GLM : matrix and vector math
* stb_perlin.h : perlin noise function
* stb_truetype.h and stb_rect_pack.h : load .ttf files into a font atlas
* stb_image.h : read images files from disk

