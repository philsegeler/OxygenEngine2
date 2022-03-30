#ifndef OE_LIBS_H_INCLUDED
#define OE_LIBS_H_INCLUDED

// ALL platform specific includes must go here

#ifdef __gnu_linux__

#define OE_PLATFORM_PC
#define OE_PLATFORM_LINUX
#define OE_RENDERER_OPENGL

#include <OE/Renderer/glad.h>
#include <SDL2/SDL.h>


#elif defined(_WIN32)

#define OE_PLATFORM_PC
#define OE_PLATFORM_WINDOWS
#define OE_RENDERER_OPENGL
#define SDL_MAIN_HANDLED

#include <OE/Renderer/glad.h>
#include <SDL.h>

#elif defined(__EMSCRIPTEN__)

#define OE_PLATFORM_WEB
#define OE_RENDERER_GLES

#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <SDL.h>
#include <SDL_opengles2.h>

#else

#define OE_PLATFORM_PC
#define OE_PLATFORM_UNKNOWN
#define OE_RENDERER_OPENGL

//#include <Renderer/glad.h>

#include <SDL2/SDL.h>

#endif


#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <cmath>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#endif // OE_LIBS_H_INCLUDED
