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

#### Windows - CMake




### Dependencies

* GLEW : loading OpenGL extensions
* GLFW : window creation OpenGL window context initialization and keyboard inputs
* GLM : matrix and vector math
* stb_perlin.h : perlin noise function
* stb_truetype.h and stb_rect_pack.h : loaf .ttf file into a font atlas
* stb_image.h : read images files from disk

