/*
**
**                      -+- lgi -+-
**
**                 Mundane Graphics API.
**
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**  Copyright(C) Dayan Rodriguez, 2022, All Rights Reserved
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**
**        NOT PRODUCTION READY/NOT PRODUCTION QUALITY
**
**
**               HONI SOIT QUI MAL Y PENSE
**
**                 github.com/MicroRJ/lgi
**
*/

/*
** Libraries that you should use instead:
** sokol or tigr or raylib or virtually any other library that isn't this one.
**
** ** panic notes **
** - I DON'T SEE ANYTHING:
**   - Ensure that the pixel shader is not the problem first, make it return a solid color, like white,
**     if it does show, ensure the alpha channel is set.
**   - Ensure the render stack is configured properly, see note below to ensure how.
**   - Ensure your constant, vertex and index buffers contain valid data, you can check all this quite
**     trivially in the amazing 'Render Doc' for free!
**     - Select the 'Event Browser' window, (You can toggle windows under the 'Window' menu)
**         - First look at your draw calls, ensure you have a 'ClearRenderTarget()', 'Present()' and
**           at least a draw instruction, such as 'DrawIndexed(6)', if you don't it means you forgot
**           missed some code.
**         - For further inspection, select a draw event, such as 'DrawIndexed(..)'
**           - First, go to the 'Pipeline Stage' window, click the vertex and then the pixel shader stages to ensure
**             that all required resources are properly bound in their corresponding slots.
**              - You can click on the texture or resource to view its contents.
**           - Then, to verify that the per vertex data is proper, click on the 'Mesh Viewer' window,
**             you can see the projected inputs and outputs of the vertex shader, use this window to ensure
**             everything is properly set.
**           - If applicable, ensure the 'w' component of your vectors is properly set, if not sure, edit
**              the vertex shader and manually set to .5.
**   - If you're sure the problem is not the data nor the shaders, then it will most likely be a pipeline configuration
**     mistake, check the blend state first, then the rasterizer state.
**      - Ensure there's at least one blend state properly configured, if you're not sure disable blending
**        altogether to use d3d's default blend state.
**      - If clipping enabled, ensure you've got a clip set big enough for your scene, if not sure, disable
**        clipping altogether by not setting the rasterizer.
**      - Ensure your view-port is properly set!
**   - If you still don't see anything, try changing the clear color to something different, if you don't
**     see that color, then it could be a driver error, in which case, run Visual Studio's Graphic Debugger.
*/
// todo: bind different textures to the shader to avoid having to switch them multiple times per frame?
#ifndef _lgi_
#define _lgi_

// TODO:
// Implement lgi_IMLEMENTATION

// TODO:
// Allocators!

//
// Basic Setup Macros:
//

// Define:
// lgi_CLOSE_ON_ESCAPE: To Terminate the Application

//
// NOTE: These Options can Also be Changed at Runtime!
//

#if !defined(lgi_DEFAULT_WINDOW_WIDTH)
	#define lgi_DEFAULT_WINDOW_WIDTH CW_USEDEFAULT
#endif

#if !defined(lgi_DEFAULT_WINDOW_HEIGHT)
	#define lgi_DEFAULT_WINDOW_HEIGHT CW_USEDEFAULT
#endif

#if !defined(lgi_DEFAULT_INDEX_BUFFER_LENGTH)
	#define lgi_DEFAULT_INDEX_BUFFER_LENGTH 0x1000
#endif
#if !defined(lgi_DEFAULT_VERTEX_BUFFER_LENGTH)
	#define lgi_DEFAULT_VERTEX_BUFFER_LENGTH 0x1000
#endif


//
// NOTE: Multi-Sampling Anti-Aliasing!
// 1) Disabled, Single-Sampling!
//
#if !defined(lgi_MSAA)
	#define lgi_MSAA 1
#endif

#if !defined(lgi_REFRESH_RATE)
	#define lgi_REFRESH_RATE 60
#endif

//
// Device Setup Macros:
//

#if defined(_DEBUG)
	#define lgi_DEBUGGABLE_DEVICE
#endif

#if !defined(lgi_COMMON_DEVICE_SETUP_FLAGS)
	#define lgi_COMMON_DEVICE_SETUP_FLAGS D3D11_CREATE_DEVICE_BGRA_SUPPORT
#endif

#if !defined(lgi_DEFAULT_DEVICE_SETUP_FLAGS)
	#if defined(lgi_DEBUGGABLE_DEVICE)
		#define lgi_DEFAULT_DEVICE_SETUP_FLAGS lgi_COMMON_DEVICE_SETUP_FLAGS|D3D11_CREATE_DEVICE_DEBUG|D3D11_CREATE_DEVICE_SINGLETHREADED
	#else
		#define lgi_DEFAULT_DEVICE_SETUP_FLAGS lgi_COMMON_DEVICE_SETUP_FLAGS
	#endif
#endif

//
// Shader Compilation Setup Macros:
//

#if !defined(lgi_COMMON_SHADER_BUILD_FLAGS)
	#define lgi_COMMON_SHADER_BUILD_FLAGS D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR|D3DCOMPILE_WARNINGS_ARE_ERRORS|D3DCOMPILE_ENABLE_STRICTNESS
#endif

#if !defined(lgi_DEFAULT_SHADER_BUILD_FLAGS)
	#if defined(lgi_DEBUGGABLE_SHADERS)
		#define lgi_DEFAULT_SHADER_BUILD_FLAGS lgi_COMMON_SHADER_BUILD_FLAGS|D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION
	#else
		#define lgi_DEFAULT_SHADER_BUILD_FLAGS lgi_COMMON_SHADER_BUILD_FLAGS|D3DCOMPILE_OPTIMIZATION_LEVEL3
	#endif
#endif

//
// Memory Macros:
//

#if !defined(lgi__deallocate_memory)
	#define lgi__deallocate_memory(memory,user) ((void)(user),free(memory))
#endif
#if !defined(lgi__allocate_memory)
	#define lgi__allocate_memory(length,user) ((void)(user),malloc(length))
#endif
#if !defined(lgi__reallocate_memory)
	#define lgi__reallocate_memory(length,memory,user) ((void)(user),realloc(length,memory))
#endif
#if !defined(lgi__clear_memory)
	#define lgi__clear_memory(memory,length) (memset(memory,0,length))
#endif

#define lgi__allocate_typeof(T) ((T*)lgi__allocate_memory(sizeof(T),NULL))
#define lgi__clear_typeof(T) (lgi__clear_memory(T,sizeof(*(T))))

//
// String Formatting Utility:
//

static char *_fmt(char const *format, ...);

//
// Logging:
//


//
// NOTE: This is so that it works with d3d out of the box!
//

#define lgi_CORRUPTION  D3D11_MESSAGE_SEVERITY_CORRUPTION
#define lgi_ERROR       D3D11_MESSAGE_SEVERITY_ERROR
#define lgi_WARNING     D3D11_MESSAGE_SEVERITY_WARNING
#define lgi_INFO        D3D11_MESSAGE_SEVERITY_INFO
#define lgi_MESSAGE     D3D11_MESSAGE_SEVERITY_MESSAGE
static void lgi_logFunction(int severity, char const *message);

static void lgi_appLogFunction(int severity, char const *message, char const *file, char const *func, int line);

#if !defined(lgi_logSomething)
	#define lgi_logSomething(TAG,FMT,...) (lgi_appLogFunction(TAG,_fmt(FMT, ## __VA_ARGS__),__FILE__,__func__,__LINE__),0)
#endif
#if !defined(lgi_logInfo)
	#define lgi_logInfo(FMT,...) lgi_logSomething(lgi_INFO,FMT, ## __VA_ARGS__)
#endif
#if !defined(lgi_logError)
	#define lgi_logError(FMT,...) lgi_logSomething(lgi_ERROR,FMT, ## __VA_ARGS__)
#endif

//
// Assert Macros:
//

#ifndef lgi_ASSERT
# ifdef _DEBUG
#  define lgi_ASSERT(is_true,...) do{ if(!(is_true)) { lgi_logError("assertion triggered"); __debugbreak(); } } while(0)
# else
#  define lgi_ASSERT(is_true,...)
# endif
#endif

//
// NOTE: To Make Struct Literals In C or CPP Mode!
//

#if !defined(lgi_T)
	#if !defined(__cplusplus)
		#define lgi_T(T) (T)
	#else
		#define lgi_T(T)
	#endif
#endif


//
// Attribute Macros:
//

#if !defined(lgi_API)
	#define lgi_API static
#endif

#if !defined(lgi_Global)
	#define lgi_Global static
#endif

//
// Constant Definitions:
//

#define lgi_PI  3.14159
#define lgi_TAU 6.28318

//
// Weird Macros
//
#define isNeitherOf3(xx,y0,y1,y2) (((xx) != (y0)) && ((xx) != (y1)) && ((xx) != (y2)))

//
// [[INCLUSIONS]]
//

//
// WINDOWS
//


#if !defined(lgi_NO_WINDOWS)
	#pragma comment(lib,"user32")
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#define _NO_CRT_STDIO_INLINE
	#include <windows.h>
	#include <Windowsx.h>
# endif

//
// D3D
//

#pragma comment(lib,      "Gdi32")
#pragma comment(lib,     "dxguid")
#pragma comment(lib,      "d3d11")
#pragma comment(lib,"d3dcompiler")

#define CINTERFACE
#define COBJMACROS
#define D3D11_NO_HELPERS

// NOTE: Suppress some warnings generated by d3dcompiler.h

#pragma warning(push)
#pragma warning(disable:4115)

#include <d3dcompiler.h>

#pragma warning(pop)

// NOTE: Suppress some warnings generated by d3d11.h
# pragma warning(push)
# pragma warning(disable:4201)

// #include       <d3d11.h>
#include <d3d11_3.h>

#pragma warning(pop)


#include <dxgidebug.h>
#include <dxgi.h>
#include <dxgi1_3.h>

//
// STB:
//

#if !defined(STB_SPRINTF_IMPLEMENTATION)
	#define STB_SPRINTF_IMPLEMENTATION
	#include <stb/stb_sprintf.h>
#endif

#if !defined(STB_IMAGE_IMPLEMENTATION)
	#define STB_IMAGE_IMPLEMENTATION
	#define STBI_MALLOC(size) lgi__allocate_memory(size,NULL)
	#define STBI_REALLOC(size,memory) lgi__reallocate_memory(size,memory,NULL)
	#define STBI_FREE(memory) lgi__deallocate_memory(memory,NULL)
	#include <stb/stb_image.h>
#endif

#if !defined(STB_IMAGE_WRITE_IMPLEMENTATION)
	#define STB_IMAGE_WRITE_IMPLEMENTATION
	#define STBIW_MALLOC(size) lgi__allocate_memory(size,NULL)
	#define STBIW_REALLOC(size,memory) lgi__reallocate_memory(size,memory,NULL)
	#define STBIW_FREE(memory) lgi__deallocate_memory(memory,NULL)
	#include <stb/stb_image_write.h>
#endif


//
// Weird Macros:
//

#if !defined(isWithin)
	#define isWithin(x,l,r) (((x)>=(l))&&((x)<=(r)))
#endif

#if !defined(isWithin3)
	#define isWithin3(x,a0,a1,b0,b1,c0,c1) (isWithin(x,a0,a1) || isWithin(x,b0,b1) || isWithin(x,c0,c1))
#endif


/* todo: this is to be embedded eventually */
#include <src\hlsl\lgi.ps.h>
#include <src\hlsl\lgi.vs.h>
#include <src\hlsl\lgi_sdf.vs.h>
#include <src\hlsl\lgi_txt.ps.h>
#include <src\hlsl\lgi_txt_sdf.ps.h>
#include <src\hlsl\lgi_sdf_cir.ps.h>
#include <src\hlsl\lgi_sdf_box.ps.h>


//
// Warning Suppression:
//

/* disabled warnings */
#pragma warning(push)
/* unreferenced stuff */
#pragma warning(disable:4100)
/* nameless structs and unions */
#pragma warning(disable:4201)
/* int to float float to int and truncation warnings */
#pragma warning(disable:4244)
#pragma warning(disable:4305)


//
// Basic Types:
//

typedef signed long long int lgi_longInt;

#define lgi_Null NULL

typedef signed int lgi_Bool;
#define lgi_True  ((lgi_Bool) 1)
#define lgi_False ((lgi_Bool) 0)

/* remove this please! */
#if !defined(isTrue)
	#define isTrue(xx) ((xx) != lgi_False)
#endif
#if !defined(isFalse)
	#define isFalse(xx) ((xx) == lgi_False)
#endif

/* remove this please! */
#if !defined(isNull)
	#define isNull(xx) ((xx) == lgi_Null)
#endif
#if !defined(isNotNull)
	#define isNotNull(xx) ((xx) != lgi_Null)
#endif
#if !defined(isNotNullAnd)
	#define isNotNullAnd(xx,yy) (isNotNull(xx) && (yy))
#endif



typedef enum {
	lgi_Error_NONE = 0,
	lgi_Error_UNKNOWN,
	lgi_Error_CREATE_TEXTURE,
	lgi_Error_SHADER_COMPILATION,
} lgi_Error;

#if !defined(lgi_Success)
	#define lgi_Success(xx) ((xx) == lgi_Error_NONE)
#endif
#if !defined(lgi_Failed)
	#define lgi_Failed(xx) ((xx) != lgi_Error_NONE)
#endif

typedef enum {
	lgi_Format_R8_UNORM 		  = DXGI_FORMAT_R8_UNORM,
	lgi_Format_R8G8B8A8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM
} lgi_Format;


#include <src/lgi_dlb.c>

//
// MATH STUFF:
//

lgi_API float lgi_mix(float ratio, float min, float max);
lgi_API float lgi_unmix(float val, float min, float max);
lgi_API float lgi_remix(float val, float val_min, float val_max, float min, float max);
lgi_API float lgi_clamp(float val, float min, float max);



#include <src/lgi_vec.c>

/*
**
**  NOLI SE TANGERE
**
*/

typedef vec4 lgi_Color;

#if !defined(lgi_RGBA)
	#define lgi_RGBA(R,G,B,A) lgi_T(lgi_Color){R,G,B,A}
#endif

#if !defined(lgi_RGBA_U)
	#define lgi_RGBA_U(R,G,B,A) lgi_RGBA((R)/255.f,(G)/255.f,(B)/255.f,(A)/255.f)
#endif

// CHANGE:
// -- lgi_Color__XXX -> lgi_XXX

#define lgi_CLEAR       lgi_RGBA_U(0xFF, 0xFF, 0xFF, 0x00)
#define lgi_WHITE       lgi_RGBA_U(0xFF, 0xFF, 0xFF, 0xFF)
#define lgi_BLACK       lgi_RGBA_U(0x00, 0x00, 0x00, 0xFF)
#define lgi_RED         lgi_RGBA_U(0xFF, 0x00, 0x00, 0xFF)
#define lgi_GREEN       lgi_RGBA_U(0x00, 0xFF, 0x00, 0xFF)
#define lgi_BLUE        lgi_RGBA_U(0x00, 0x00, 0xFF, 0xFF)
#define lgi_YELLOW      lgi_RGBA_U(0xFF, 0xFF, 0x00, 0xFF)
#define lgi_CYAN        lgi_RGBA_U(0x00, 0xFF, 0xFF, 0xFF)
#define lgi_MAGENTA     lgi_RGBA_U(0xFF, 0x00, 0xFF, 0xFF)
#define lgi_ORANGE      lgi_RGBA_U(0xFF, 0xA5, 0x00, 0xFF)
#define lgi_PURPLE      lgi_RGBA_U(0x80, 0x00, 0x80, 0xFF)
#define lgi_PINK        lgi_RGBA_U(0xFF, 0xC0, 0xCB, 0xFF)
#define lgi_LIME        lgi_RGBA_U(0x00, 0xFF, 0x00, 0xFF)
#define lgi_TEAL        lgi_RGBA_U(0x00, 0x80, 0x80, 0xFF)
#define lgi_SKY_BLUE    lgi_RGBA_U(0x87, 0xCE, 0xEB, 0xFF)
#define lgi_GOLD        lgi_RGBA_U(0xFF, 0xD7, 0x00, 0xFF)
#define lgi_INDIGO      lgi_RGBA_U(0x4B, 0x00, 0x82, 0xFF)
#define lgi_SILVER      lgi_RGBA_U(0xC0, 0xC0, 0xC0, 0xFF)
#define lgi_TURQUOISE   lgi_RGBA_U(0x40, 0xE0, 0xD0, 0xFF)
#define lgi_CORAL       lgi_RGBA_U(0xFF, 0x7F, 0x50, 0xFF)
#define lgi_ORCHID      lgi_RGBA_U(0xDA, 0x70, 0xD6, 0xFF)
#define lgi_LAVENDER    lgi_RGBA_U(0xE6, 0xE6, 0xFA, 0xFF)
#define lgi_MAROON      lgi_RGBA_U(0x80, 0x00, 0x00, 0xFF)
#define lgi_NAVY        lgi_RGBA_U(0x00, 0x00, 0x80, 0xFF)
#define lgi_OLIVE       lgi_RGBA_U(0x80, 0x80, 0x00, 0xFF)
#define lgi_SALMON      lgi_RGBA_U(0xFA, 0x80, 0x72, 0xFF)
#define lgi_AQUAMARINE  lgi_RGBA_U(0x7F, 0xFF, 0xD4, 0xFF)

//
// Common Types:
//

typedef ID3D11DeviceChild *lgi_Unknown;

typedef struct lgi_Sampler lgi_Sampler;

typedef struct lgi_Texture_Config lgi_Texture_Config;
typedef struct lgi_Texture lgi_Texture;

typedef struct lgi_Buffer lgi_Buffer;

typedef struct lgi_Shader_Config lgi_Shader_Config;
typedef struct lgi_Shader lgi_Shader;

typedef struct lgi_Program lgi_Program;



//
// TODO: Could We Just Make This A Part Of Texture!
//

typedef struct lgi_Image {
	int size_x, size_y;
	int format;
	int stride;
	void * memory;
} lgi_Image;


//
// API:
//

// Main API:

lgi_API void lgi_initWindowed(int windowWidth, int windowHeight, char const *windowTitle);
lgi_API void lgi_clearBackground(lgi_Color color);
lgi_API void lgi_bindProgram(lgi_Program);
lgi_API void lgi_bindShaders(lgi_Shader *v, lgi_Shader *p, int flush);
lgi_API void lgi_bindTexture(int slot, lgi_Texture *texture, int flush);
lgi_API void lgi_flushImmediatly();

// Platform API:


/* timing functions have been renamed */
lgi_API lgi_longInt lgi_queryClockHz();
lgi_API lgi_longInt lgi_pollClock();

lgi_API lgi_Bool lgi_setActiveWindow(HWND window);
lgi_API void lgi_setCursor(HCURSOR cursor);


lgi_API void lgi_unloadFileContents(void *fileContents);
lgi_API void *lgi_loadFileContents(char const *fileName, int *length);

// Input API:

lgi_API int lgi_wasButtonDown(int x);
lgi_API int lgi_isButtonDown(int x);
lgi_API int lgi_isButtonReleased(int x);
lgi_API int lgi_isButtonPressed(int x);
lgi_API int lgi_testKey(int x);
lgi_API int lgi_testFKey(int x);
lgi_API int lgi_testCtrlKey();
lgi_API int lgi_testAltKey();
lgi_API int lgi_testShiftKey();
lgi_API int lgi_lastChar();

//
// Shader API:
//

/* [[BEWARE]]:
	May not handle padding very well should you choose to let the API create the input layout for you
	automatically using the reflection API.
	Try to pack things into bigger units if you run into alignment issues, or try
	re-ordering the structure. */
lgi_API lgi_Error lgi_initShader(lgi_Shader *shader, lgi_Shader_Config *config);
lgi_API lgi_Shader *lgi_buildShader(int flags, char const *label, size_t length, void *memory);
lgi_API lgi_Shader *lgi_loadShader(int flags, char const *label, char const *entry, char const *fileName);

lgi_API lgi_Buffer *lgi_getShaderInputBlock(lgi_Shader *shader);
lgi_API void lgi_resizeShaderInputBlock(lgi_Shader *shader, int length);
lgi_API void lgi_updateShaderInputBlock(lgi_Shader *shader, int length, void *memory);

//
// Buffer API:
//

lgi_API void lgi_deleteBuffer(lgi_Buffer *xx);

lgi_API void lgi_returnBufferContents(lgi_Buffer *xx);
lgi_API void lgi_updateBufferContents(lgi_Buffer *xx, void *lpMemory, size_t length);
lgi_API void *lgi_borrowBufferContents(lgi_Buffer *xx, int *lpStride);

lgi_API lgi_Buffer *lgi_makeConstBuffer(unsigned int length, void *memory);
lgi_API lgi_Buffer *lgi_makeIndexBuffer(int index_size, int index_count);
lgi_API lgi_Buffer *lgi_makeVertexBuffer(int vertex_size, int vertex_count);
lgi_API lgi_Buffer *lgi_makeStructuredBuffer(int struct_size, int struct_count);


//
// Image API:
//

lgi_API lgi_Image lgi_makeImage(int size_x, int size_y, int format);
lgi_API lgi_Image lgi_loadImage(const char *name);

//
// Texture API:
//

lgi_API lgi_Texture_Config lgi_makeTextureConfig(int size_x, int size_y, DXGI_FORMAT format, int stride, void  *memory, int samples, int quality, D3D11_USAGE memtype, int useflag, int memflag);
lgi_API lgi_Error lgi_initTexture(lgi_Texture *xx, lgi_Texture_Config *config);
lgi_API void lgi_closeTexture(lgi_Texture *xx);
lgi_API void lgi_deleteTexture(lgi_Texture *xx);
lgi_API lgi_Texture *lgi_createTexture(lgi_Texture_Config *config);
lgi_API lgi_Texture *lgi_makeTexture(int size_x, int size_y, DXGI_FORMAT format, int stride, void *memory);
lgi_API lgi_Texture *lgi_loadTexture(char const *fileName);
lgi_API lgi_Texture *lgi_uploadImage(lgi_Image image);
lgi_API void *lgi_borrowTextureContents(lgi_Texture *xx, int *stride);
lgi_API void lgi_returnTextureContents(lgi_Texture *xx);
lgi_API void lgi_updateTexture(lgi_Texture *xx, lgi_Image image);
lgi_API void lgi_copyTexture(lgi_Texture *dst, lgi_Texture *src);
lgi_API lgi_Texture *lgi_makeDepthTarget(int size_x, int size_y, int format);
lgi_API lgi_Texture *lgi_makeColorTarget(int size_x, int size_y, int format, int samples, int quality);


lgi_API void lgi_closeHandle(lgi_Unknown handle) {
	if (handle != 0) {
		IUnknown_Release(handle);
	}
}


enum {
	rx_kNONE = 0,
	rx_kESCAPE,
	// NOTE: Could be stored in a byte
	rxKEY_kLEFT,rxKEY_kRIGHT,rxKEY_kUP,rxKEY_kDOWN,
	// NOTE: Could be stored in a short
	rxKEY_kF1,rxKEY_kF2,rxKEY_kF3,rxKEY_kF4,rxKEY_kF5,
	rxKEY_kF6,rxKEY_kF7,rxKEY_kF8,rxKEY_kF9,
	rxKEY_kF10,rxKEY_kF11,rxKEY_kF12,

	// NOTE: Could be stored in a byte!
	rx_kLCTRL,rx_kRCTRL,
	rx_kLSFHT,rx_kRSFHT,

	rxKEY_kRETURN,
	rx_kBCKSPC,rx_kDELETE,
	rx_kHOME,rx_kEND,

	// NOTE: Could be stored with 2 ints
	rxKEY_kSPACE = ' ',
	rxKEY_kA     = 'A',
	rxKEY_kZ     = 'Z',
	rxKEY_k0     = '0',
	rxKEY_k9     = '9',
};

typedef int lgi_Index;

typedef union {
	struct {
		vec4 xyzw;
		vec4 rgba;
		vec2 uv;
	};
	struct {
		float x,y,z,w;
		float r,g,b,a;
		float u,v;
	};
	//
	// TODO: Remove!
	//
	struct {
		vec2 xy;
		vec4 xyxy;
		vec4 rgba;
		vec4 flag;
	} rect;
} lgi_Vertex;

typedef struct {
	lgi_Matrix matrix;
	vec2   xyscreen;
	vec2   xysource;
	vec2   xycursor;
	double total_seconds;
	double delta_seconds;
} lgi_Uniform_Slots;

typedef struct lgi_Buffer {
	struct {
		int   stride;
		void *memory;
	} mapped;
	struct {
		union {
			ID3D11Buffer *buffer;
			ID3D11Resource *resource;
		};
		ID3D11ShaderResourceView *shader_target;
	} d3d11;
} lgi_Buffer;

// TODO: we can associate this with a texture and forget about it!
typedef struct lgi_Sampler {
	struct {
		ID3D11SamplerState *state;
	} d3d11;
} lgi_Sampler;

typedef struct lgi_Texture_Config {
	int    format;
	int    size_x;
	int    size_y;
  	/* multisampling, default should be 1,0 */
	int    samples;
	int    quality;

  	/* replace this with a single flag field instead #todo */
	struct {
  		/* GPU allocation flags */
		D3D11_USAGE memtype;
		int useflag;
    	/* CPU access flags */
		int memflag;
	};

	/* memory and stride are optional, if memory provided stride may not be 0 */
	int    stride;
	void  *memory;

 	/* optional in/out */
	struct {
		union {
			ID3D11Resource  *resource;
			ID3D11Texture2D *texture_2d;
		};

		ID3D11ShaderResourceView  *shader_target;
		ID3D11RenderTargetView     *color_target;
		ID3D11DepthStencilView     *depth_target;

		ID3D11SamplerState *sampler;
	} d3d11;
} lgi_Texture_Config;

typedef struct lgi_Texture {
	int format;
	int size_x;
	int size_y;
	int samples;
	int quality;

	struct {
		int   stride;
		void *memory;
	} mapped;

	struct {
		union {
			ID3D11Resource  *resource;
			ID3D11Texture2D *texture_2d;
		};
		// NOTE(RJ):
		ID3D11SamplerState *sampler;

    	// NOTE(RJ): optional!
		ID3D11ShaderResourceView *shader_target;
		ID3D11RenderTargetView   *color_target;
		ID3D11DepthStencilView   *depth_target;
	} d3d11;
	char const *debug_label;
} lgi_Texture;


enum {
	lgi_INVALID_SHADER_TYPE = 0 << 0,
	lgi_PIXEL_SHADER_TYPE = 1 << 1,
	lgi_VERTEX_SHADER_TYPE = 1 << 2,
	lgi_COMPUTE_SHADER_TYPE = 1 << 3,
};

typedef struct lgi_Shader_Config {
	char const *label;
	int flags;
	int expected_color_attachments;
	int expected_input_buffers;
	struct {
		struct {
			size_t length;
			union {
				void *memory;
				char *string;
			};
		} bytecode;
		struct {
			char const *debug_label;
			char const *model;
			char const *entry;
			struct {
				size_t  length;
				void   *memory;
			};
		} compile;
	} source;
	struct {
		D3D11_INPUT_ELEMENT_DESC attr_array[0x20];
		int attr_count;
		struct {
			ID3D11InputLayout *layout;
		} d3d11;
	} layout;
} lgi_Shader_Config;

typedef struct lgi_Shader {
	char const *label;
	int flags;
	int expected_color_attachments;
	int expected_input_buffers;
	lgi_Buffer *inputBlock;
	struct {
		union {
			ID3D11DeviceChild *unknown;
			ID3D11VertexShader *vertexShader;
			ID3D11PixelShader *pixelShader;
		};
		ID3D11InputLayout *layout;
	} d3d11;
} lgi_Shader;

typedef struct lgi_Program {

	lgi_Shader *vertexShader;
	lgi_Shader *pixelShader;

	struct {
		ID3D11BlendState *blendState;
	} d3d11;
} lgi_Program;

typedef struct {

	int mode;

	lgi_Matrix view_matrix;
	lgi_Matrix world_matrix;

	lgi_Uniform_Slots constSlots;


	lgi_Shader *liveVertexShader;
	lgi_Shader *livePixelShader;

	lgi_Texture *liveTextures[8];

	struct {
		ID3D11BlendState *blendState;
		ID3D11BlendState *rasterizerState;
		ID3D11SamplerState *samplerStates[8];
	} d3d11;
	lgi_Buffer  *constBuffer;


	lgi_Buffer *vertexBuffer;
	lgi_Buffer *indexBuffer;

	lgi_Vertex *vertex_array;
	lgi_Index *index_array;
	int vertex_tally;
	int index_tally;

	int index_offset;
	lgi_Vertex attr;
} lgi_State;

//
// Core:
//

typedef struct lgi_Core lgi_Core;
typedef struct lgi_Core {

	/* timing stuff */
	__int64 frameTally;

	// lgi::Time
	struct {
		unsigned __int64 ticksPerSecond;

		unsigned __int64 start_ticks;
		unsigned __int64 total_ticks;

		unsigned __int64 frame_ticks;
		unsigned __int64 delta_ticks;
		union {
			double totalSeconds;
			double total_seconds;
		};
		union {
			double deltaSeconds;
			double delta_seconds;
		};
	} Time;

	// lgi::Input
	struct {
		struct {
			int lastChar;

			/* todo: store this better */
			char     oldKeyState[0x100];
			char     newKeyState[0x100];

			unsigned is_ctrl: 1;
			unsigned is_menu: 1;
			unsigned is_shft: 1;
		} Keyboard;
		struct {
			int xcursor;
			int ycursor;
			int yscroll;
			int xscroll;

			int oldButtonState;
			int newButtonState;
		} Mice;
	} Input;

	// lgi::Window
	struct {
		unsigned isClosed: 1;
		unsigned isVisible: 1;

		struct {
			int size_x;
			int size_y;
			int center_x;
			int center_y;
		};

		struct {
			HWND handle;
		} win32;

		/* output media */
		struct {
			lgi_Texture *target;

			struct {
				IDXGISwapChain2 *stage;
				void            *frame_await;
			} d3d11;
		} Output;
	} Window;


	/* todo: constants, should be upper case */
	lgi_Sampler linear_sampler;
	lgi_Sampler point_sampler;
	lgi_Sampler anisotropic_sampler;

	ID3D11SamplerState *LINEAR_SAMPLER;
	ID3D11SamplerState *POINT_SAMPLER;
	ID3D11SamplerState *ANISOTROPIC_SAMPLER;


	lgi_Program defaultProgram;
	lgi_Program lcdTextProgram;
	lgi_Program sdfTextProgram;
	lgi_Program sdfCircleProgram;
	lgi_Program sdfBoxProgram;

	lgi_Texture *defaultDepthTarget;
	lgi_Texture *defaultColorTarget;
	lgi_Buffer  *defaultConstBlock;

	lgi_Texture *whiteTexture;


	// lgi::d3d11
	struct {
		struct {
			ID3D11InfoQueue        *inf;
			ID3D11Device           *dev;
			ID3D11DeviceContext    *ctx;
		};
		ID3D11BlendState *defaultBlendState;
		ID3D11RasterizerState *defaultRasterizerState;
		ID3D11DepthStencilState *defaultStencilState;
	} d3d11;

	struct {
		HMODULE user32_dll;
		struct {
			HCURSOR arrow;
		} cursor;
	} win32;

	lgi_State State;
} lgi_Core;

/* the source of all evil is here */
lgi_Global lgi_Core lgi;
#define LGI lgi
#define lgi lgi


//
// Logging:
//

char *_fmt(char const *format, ...) {
	// TODO(RJ): Ensure we don't overflow buffer!
	static char buffer[0x1000];
	static char *cursor = buffer;
	va_list xx;
	va_start(xx,format);
	int length = stbsp_vsnprintf(NULL,0,format,xx);
	if (((cursor-buffer)+length) >= sizeof(buffer)) {
		cursor=buffer;
	}
	stbsp_vsnprintf(cursor,(int)sizeof(buffer) - (cursor-buffer),format,xx);
	char *result = cursor;
	cursor += length+1;
	va_end(xx);
	return result;
}

static void lgi_logFunction(int severity, char const *message) {
	char const *name = "TRACE";
	switch (severity) {
		case lgi_CORRUPTION: name = "CORRUPTION";
		case lgi_ERROR: name = "ERROR";
		case lgi_WARNING: name = "WARNING";
		case lgi_INFO: name = "INFO";
		case lgi_MESSAGE: name = "MESSAGE";
	}
	printf("%s => %s\n",name,message);
}

static void lgi_appLogFunction(int severity, char const *message, char const *file, char const *func, int line) {
	char const *name = "TRACE";
	switch (severity) {
		case lgi_CORRUPTION: name = "CORRUPTION";
		case lgi_ERROR: name = "ERROR";
		case lgi_WARNING: name = "WARNING";
		case lgi_INFO: name = "INFO";
		case lgi_MESSAGE: name = "MESSAGE";
	}
	char const *fileName = file;
	for(; *file != 0; file += 1) {
		if ((*file == '/') || (*file == '\\')) {
			fileName = file+1;
		}
	}
	// ID3D11InfoQueue_AddApplicationMessage(lgi.d3d11.inf,D3D11_MESSAGE_SEVERITY_INFO,completeMessage);

	printf("%s => %s[%i] %s(): %s\n",name,fileName,line,func,message);
}

//
// Platform API:
//

lgi_API lgi_Bool lgi_setActiveWindow(HWND window) {
	HWND lastWnd = SetActiveWindow(window);
	return lastWnd != INVALID_HANDLE_VALUE;
}

lgi_API void lgi_setCursor(HCURSOR cur) {
	SetCursor(cur);
}

lgi_API lgi_longInt lgi_queryClockHz() {
#if defined(_WIN32)
	LARGE_INTEGER l;
	QueryPerformanceFrequency(&l);
	return l.QuadPart;
#else
	#error
#endif
}

lgi_API lgi_longInt lgi_pollClock() {
#if defined(_WIN32)
	LARGE_INTEGER l;
	QueryPerformanceCounter(&l);
	return l.QuadPart;
#else
	#error
#endif
}


lgi_API void lgi_unloadFileContents(LPVOID fileContents) {
	VirtualFree(fileContents,0,MEM_RELEASE);
}

lgi_API void *lgi_loadFileContents(char const *fileName, __int32 *lpSize) {
	*lpSize = 0;
	char *lpBuffer = lgi_Null;
	HANDLE hFile = CreateFileA(fileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0x00,NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD highFileSize;
		DWORD lowFileSize = GetFileSize(hFile,&highFileSize);
		lpBuffer = VirtualAlloc(NULL,lowFileSize,MEM_COMMIT|MEM_RESERVE,0);
		DWORD bytesRead;
		if (ReadFile(hFile,lpBuffer,lowFileSize,&bytesRead,NULL) != FALSE) {
			*lpSize = bytesRead;
		} else {
			lgi_unloadFileContents(lpBuffer);
			lpBuffer = lgi_Null;
		}
		CloseHandle(hFile);
	}
	return lpBuffer;
}

//
// Input API:
//


lgi_API int lgi_wasButtonDown(int x) {
	return ((lgi.Input.Mice.oldButtonState & (1 << x)) != 0);
}

lgi_API int lgi_isButtonDown(int x) {
	return ((lgi.Input.Mice.newButtonState & (1 << x)) != 0);
}

lgi_API int lgi_isButtonReleased(int x) {
	return !lgi_isButtonDown(x) && lgi_wasButtonDown(x);
}

lgi_API int lgi_isButtonPressed(int x) {
	return lgi_isButtonDown(x) && !lgi_wasButtonDown(x);
}

lgi_API int lgi_testKey(int x) {
	return (lgi.Input.Keyboard.newKeyState[x] != 0);
}

lgi_API int lgi_testFKey(int x) {
	return (lgi_testKey(rxKEY_kF1 + iclamp(x,1,12)-1));
}

lgi_API int lgi_testCtrlKey() {
	return (lgi.Input.Keyboard.is_ctrl != 0);
}

lgi_API int lgi_testAltKey() {
	return (lgi.Input.Keyboard.is_menu != 0);
}

lgi_API int lgi_testShiftKey() {
	return (lgi.Input.Keyboard.is_shft != 0);
}

lgi_API int lgi_lastChar() {
	return lgi.Input.Keyboard.lastChar;
}

//
// Image API:
//

lgi_API lgi_Texture *lgi_uploadImage(lgi_Image image) {

	return lgi_makeTexture(image.size_x,image.size_y,image.format,image.stride,image.memory);
}

lgi_API lgi_Image lgi_makeImage(int size_x, int size_y, int format) {
	int bpp = 0;
	if (format == lgi_Format_R8_UNORM) {
		bpp = 1;
	} else
	if (format == lgi_Format_R8G8B8A8_UNORM) {
		bpp = 4;
	}
	size_t size = size_y * size_x * bpp;

	lgi_ASSERT(bpp != 0);

	lgi_Image result;
	result.size_x = size_x;
	result.size_y = size_y;
	result.format = format;
	result.stride = size_x * bpp;
	result.memory = lgi__allocate_memory(size, NULL);
	lgi__clear_memory(result.memory,size);

	return result;
}

lgi_API lgi_Image lgi_loadImage(const char *name) {

	lgi_Image result;
	lgi__clear_typeof(&result);

	/* XXX use own memory */
	void *memory=stbi_load(name,&result.size_x,&result.size_y,0,4);

	result.format=lgi_Format_R8G8B8A8_UNORM;
	result.memory=memory;
	result.stride=result.size_x*4;
	return result;
}



#include <src/lgi_core.c>
#include <src/lgi_draw.c>



//
// Polling:
//

lgi_API void lgi_pollDebugMessages() {
	// int infoMessageCount = ID3D11InfoQueue_GetNumStoredMessages(lgi.d3d11.inf);
	// int infoMessageCountStored = ID3D11InfoQueue_GetNumMessagesAllowedByStorageFilter(lgi.d3d11.inf);
	// int infoMessageCountDenied = ID3D11InfoQueue_GetNumMessagesDeniedByStorageFilter(lgi.d3d11.inf);
	// int infoMessageCountRetrieved = ID3D11InfoQueue_GetNumStoredMessagesAllowedByRetrievalFilter(lgi.d3d11.inf);
	// int infoMessageCountDeniedByLimit = ID3D11InfoQueue_GetNumMessagesDiscardedByMessageCountLimit(lgi.d3d11.inf);

	int messageCount = ID3D11InfoQueue_GetNumStoredMessagesAllowedByRetrievalFilter(lgi.d3d11.inf);
	for (int i=0; i<messageCount; i+=1) {
		char buffer[sizeof(D3D11_MESSAGE) + 0x1000];
		size_t length = sizeof(buffer);
		D3D11_MESSAGE *message = (void *) buffer;
		HRESULT error = ID3D11InfoQueue_GetMessage(lgi.d3d11.inf,i,message,&length);
		if (SUCCEEDED(error)) {
			lgi_logFunction(message->Severity,message->pDescription);
		}
	}

	ID3D11InfoQueue_ClearStoredMessages(lgi.d3d11.inf);
}

lgi_API void lgi_pollTime() {
	unsigned __int64 ticks = lgi_pollClock();
	lgi.Time.total_ticks = ticks - lgi.Time.start_ticks;
	lgi.Time.delta_ticks = ticks - lgi.Time.frame_ticks;
	lgi.Time.frame_ticks = ticks;

	lgi.Time.total_seconds = lgi.Time.total_ticks / (double) lgi.Time.ticksPerSecond;
	lgi.Time.delta_seconds = lgi.Time.delta_ticks / (double) lgi.Time.ticksPerSecond;
}

lgi_API int lgi_pollInput() {
	/* todo: */
	memcpy(lgi.Input.Keyboard.oldKeyState,lgi.Input.Keyboard.newKeyState,sizeof(lgi.Input.Keyboard.newKeyState));
	memset(lgi.Input.Keyboard.newKeyState,0,sizeof(lgi.Input.Keyboard.newKeyState));
	lgi.Input.Keyboard.lastChar = 0;

	lgi.Input.Mice.yscroll = 0;
	lgi.Input.Mice.xscroll = 0;
	lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
	// lgi.Input.Mice.newButtonState = 0;

	MSG message;
	while (PeekMessage(&message,NULL,0,0,PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	RECT client;
	if (GetClientRect(lgi.Window.win32.handle,&client)) {
		lgi.Window.size_x = client. right - client.left;
		lgi.Window.size_y = client.bottom - client. top;
		lgi.Window.center_x=lgi.Window.size_x>>1;
		lgi.Window.center_y=lgi.Window.size_y>>1;
	}

	return !lgi.Window.isClosed;
}

lgi_API void lgi_clearBackground(lgi_Color color) {
	ID3D11DeviceContext_ClearDepthStencilView(lgi.d3d11.ctx,lgi.defaultDepthTarget->d3d11.depth_target,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);
	ID3D11DeviceContext_ClearRenderTargetView(lgi.d3d11.ctx,lgi.defaultColorTarget->d3d11.color_target,(float *) &color);
}

lgi_API void lgi_finishDrawing() {

	lgi_flushImmediatly();

	lgi_copyTexture(lgi.Window.Output.target,lgi.defaultColorTarget);

	if (lgi.Window.isVisible != lgi_False) {
		HRESULT error = IDXGISwapChain_Present(lgi.Window.Output.d3d11.stage,1u,0);
		if FAILED(error) {
			lgi.Window.isClosed = lgi_True;
		}
		/* this does not serve any purpose in full screen mode */
		// WaitForSingleObjectEx(lgi.Window.Output.d3d11.frame_await,INFINITE,TRUE);
	}
}

lgi_API int lgi_tick() {
	// NOTE: Ensure the Window is Visible!
	if (lgi.Window.isVisible != TRUE) {
		lgi.Window.isVisible = TRUE;
		ShowWindow(lgi.Window.win32.handle,SW_SHOW);
	}

	lgi_finishDrawing();

#if defined(lgi_CLEAR_BACKGROUND_AUTOMATICALLY)
	lgi_clearBackground(lgi_Color__BLACK);
#endif

	if (lgi_False) {
		lgi_pollDebugMessages();
	}
	lgi_pollInput();
	lgi_pollTime();

	lgi.frameTally += 1;
	return !lgi.Window.isClosed;
}


int lgi_windowMessageHandler_win32(UINT,WPARAM,LPARAM);
LRESULT CALLBACK lgi_windowMessageCallback_win32(HWND,UINT,WPARAM,LPARAM);
static void lgi__initDefaults();

lgi_API void lgi_initWindowed(int window_width, int window_height, char const *window_title) {

	lgi.Time.ticksPerSecond = lgi_queryClockHz();

	//
	// NOTE: Look more into this because I really don't even know whether this works
	//
	typedef BOOL (WINAPI * XXX)(HANDLE);
	lgi.win32.user32_dll = LoadLibraryA("user32.dll");
	XXX SetProcessDpiAwarenessContext = (XXX) GetProcAddress(lgi.win32.user32_dll, "SetProcessDpiAwarenessContext");
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	lgi.win32.cursor.arrow = LoadCursorA(NULL,IDC_ARROW);

	D3D_FEATURE_LEVEL featureMenu[2][2]= {
		{D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0},
		{D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0},
	};

	D3D_FEATURE_LEVEL featureLevel;

	HRESULT error = D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_HARDWARE,0,lgi_DEFAULT_DEVICE_SETUP_FLAGS,featureMenu[0],
	ARRAYSIZE(featureMenu[0]),D3D11_SDK_VERSION,&lgi.d3d11.dev,&featureLevel,&lgi.d3d11.ctx);

	if FAILED(error) {
		error = D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_WARP,0,lgi_DEFAULT_DEVICE_SETUP_FLAGS,featureMenu[1],
		ARRAYSIZE(featureMenu[1]),D3D11_SDK_VERSION,&lgi.d3d11.dev,&featureLevel,&lgi.d3d11.ctx);
	}

#if defined(_DEBUG)
	if SUCCEEDED(error) {
		error = IUnknown_QueryInterface(lgi.d3d11.dev,&IID_ID3D11InfoQueue,(void**)&lgi.d3d11.inf);
		if SUCCEEDED(error) {
			lgi_logInfo("Enabled d3d11 Debug Info Layer Successfully");

			if (ID3D11InfoQueue_GetMuteDebugOutput(lgi.d3d11.inf)) {
				lgi_logInfo("Debug Info Output Is Muted!");
			}

			ID3D11InfoQueue_ClearStorageFilter(lgi.d3d11.inf);
			ID3D11InfoQueue_ClearRetrievalFilter(lgi.d3d11.inf);
			ID3D11InfoQueue_PushEmptyStorageFilter(lgi.d3d11.inf);

			// NOTE: Don't enable this because otherwise we don't get to see the messages!
			// ID3D11InfoQueue_SetBreakOnSeverity(lgi.d3d11.inf, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			// ID3D11InfoQueue_SetBreakOnSeverity(lgi.d3d11.inf, D3D11_MESSAGE_SEVERITY_ERROR,      TRUE);
			// ID3D11InfoQueue_SetBreakOnSeverity(lgi.d3d11.inf, D3D11_MESSAGE_SEVERITY_WARNING,    TRUE);

			// ID3D11InfoQueue_AddApplicationMessage(lgi.d3d11.inf,D3D11_MESSAGE_SEVERITY_INFO,"Test Info Message!");
			// lgi_pollDebugMessages();
		}
	}
#endif
	lgi_ASSERT(SUCCEEDED(error));

	//
	// TODO:
	//
	wchar_t unicodeWindowTitle[MAX_PATH];
	MultiByteToWideChar(CP_ACP,0,window_title,-1,unicodeWindowTitle,MAX_PATH);

	WNDCLASSW windowClass;
	lgi__clear_typeof(&windowClass);
	windowClass.lpfnWndProc=lgi_windowMessageCallback_win32;
	windowClass.hInstance=GetModuleHandleW(NULL);
	windowClass.lpszClassName=unicodeWindowTitle;

	if (RegisterClassW(&windowClass)) {
		window_width = window_width != 0 ? window_width  : lgi_DEFAULT_WINDOW_WIDTH;
		window_height = window_height != 0 ? window_height : lgi_DEFAULT_WINDOW_HEIGHT;

		window_width = window_width != CW_USEDEFAULT ? window_width : 720;
		window_height = window_height != CW_USEDEFAULT ? window_height : 720;

		RECT wnd_rect;
		wnd_rect.left=0;
		wnd_rect.top=0;
		wnd_rect.right=window_width;
		wnd_rect.bottom=window_height;
		AdjustWindowRect(&wnd_rect,WS_OVERLAPPEDWINDOW,FALSE);

		window_width = wnd_rect. right - wnd_rect.left;
		window_height = wnd_rect.bottom - wnd_rect. top;

		/* This makes the window not resizable */
		// &~WS_THICKFRAME
		lgi.Window.win32.handle = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP,windowClass.lpszClassName,unicodeWindowTitle,WS_OVERLAPPEDWINDOW
		,	0,0,window_width,window_height,NULL,NULL,windowClass.hInstance,NULL);

		if (IsWindow(lgi.Window.win32.handle)) {
			lgi_logInfo("Created Window (%ix%i)",window_width,window_height);
		}
	}


	//
	// NOTE: Create Some Default Samplers, These Are Needed For Creating Textures!
	//

	D3D11_SAMPLER_DESC samplerInfo_d3d;
	ZeroMemory(&samplerInfo_d3d,sizeof(samplerInfo_d3d));
	samplerInfo_d3d.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
	samplerInfo_d3d.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
	samplerInfo_d3d.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
	samplerInfo_d3d.MaxAnisotropy  = 0;
	samplerInfo_d3d.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerInfo_d3d.MinLOD         = 0;
	samplerInfo_d3d.MaxLOD         = D3D11_FLOAT32_MAX;
	samplerInfo_d3d.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ID3D11Device_CreateSamplerState(lgi.d3d11.dev,&samplerInfo_d3d,(ID3D11SamplerState **)&lgi.LINEAR_SAMPLER);
	samplerInfo_d3d.Filter=D3D11_FILTER_MIN_MAG_MIP_POINT;
	ID3D11Device_CreateSamplerState(lgi.d3d11.dev,&samplerInfo_d3d,(ID3D11SamplerState **)&lgi.POINT_SAMPLER);
	samplerInfo_d3d.Filter=D3D11_FILTER_ANISOTROPIC;
	ID3D11Device_CreateSamplerState(lgi.d3d11.dev,&samplerInfo_d3d,(ID3D11SamplerState **)&lgi.ANISOTROPIC_SAMPLER);

	lgi.linear_sampler.d3d11.state = (ID3D11SamplerState *) lgi.LINEAR_SAMPLER;
	lgi.point_sampler.d3d11.state = (ID3D11SamplerState *) lgi.POINT_SAMPLER;
	lgi.anisotropic_sampler.d3d11.state = (ID3D11SamplerState *) lgi.ANISOTROPIC_SAMPLER;




	//
	// NOTE: Poll Window Dimensions Before Creating Window-Dependent Resources
	//
	lgi_pollInput();

	IDXGIDevice * device_dxgi = NULL;
	IDXGIAdapter * adapter_dxgi = NULL;
	IDXGIFactory2 * factory_dxgi = NULL;

	ID3D11Device_QueryInterface(lgi.d3d11.dev,&IID_IDXGIDevice,(void **)&device_dxgi);
	IDXGIDevice_GetAdapter(device_dxgi,&adapter_dxgi);
	IDXGIAdapter_GetParent(adapter_dxgi,&IID_IDXGIFactory2,(void**)&factory_dxgi);

	// DXGI_USAGE_UNORDERED_ACCESS
	DXGI_SWAP_CHAIN_DESC1 sc_config_d3d;
	lgi__clear_typeof(&sc_config_d3d);

	sc_config_d3d.Width=lgi.Window.size_x;
	sc_config_d3d.Height=lgi.Window.size_y;
	sc_config_d3d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
	sc_config_d3d.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sc_config_d3d.BufferCount=2;
	sc_config_d3d.SwapEffect=DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	// ResizeBuffers conflicts with this flag :(
	// |DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	sc_config_d3d.Flags=DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	/* todo: allow the user to specify this */
	sc_config_d3d.SampleDesc.  Count=1;
	sc_config_d3d.SampleDesc.Quality=0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_config;
	lgi__clear_typeof(&sc_fs_config);

	sc_fs_config.RefreshRate.Numerator=lgi_REFRESH_RATE;
	sc_fs_config.RefreshRate.Denominator=1;
	sc_fs_config.Windowed=TRUE;

	error = IDXGIFactory2_CreateSwapChainForHwnd(factory_dxgi,(IUnknown *)lgi.d3d11.dev,lgi.Window.win32.handle,&sc_config_d3d,&sc_fs_config,NULL,(IDXGISwapChain1 **)&lgi.Window.Output.d3d11.stage);
	lgi_ASSERT(SUCCEEDED(error));

	IDXGIFactory_Release(factory_dxgi);
	IDXGIAdapter_Release(adapter_dxgi);
	IDXGIDevice_Release(device_dxgi);

	ID3D11Texture2D *texture_d3d;
	error = IDXGISwapChain_GetBuffer(lgi.Window.Output.d3d11.stage,0,&IID_ID3D11Texture2D,(void **)&texture_d3d);
	lgi_ASSERT(SUCCEEDED(error));


	// ID3D11DeviceContext_ClearState(lgi.d3d11.ctx);
	// error = IDXGISwapChain_ResizeBuffers(lgi.Window.Output.d3d11.stage,sc_config_d3d.BufferCount,512,512,sc_config_d3d.Format,0);

	// lgi.Window.Output.d3d11.frame_await = IDXGISwapChain2_GetFrameLatencyWaitableObject(lgi.Window.Output.d3d11.stage);

	lgi_Texture_Config config;
	lgi__clear_typeof(&config);
	config.memtype = D3D11_USAGE_DEFAULT;
	config.useflag = D3D11_BIND_RENDER_TARGET;
	config.size_x = sc_config_d3d. Width;
	config.size_y = sc_config_d3d.Height;
	config.format = sc_config_d3d.Format;
	config.d3d11.texture_2d = texture_d3d;

	lgi.Window.Output.target = lgi_createTexture(&config);



	lgi__initDefaults();

	lgi_setClippingZone(0,0,lgi.Window.size_x,lgi.Window.size_y);

	D3D11_VIEWPORT viewport_d3d;
	viewport_d3d.TopLeftX=0;
	viewport_d3d.TopLeftY=0;
	viewport_d3d.   Width=lgi.Window.size_x;
	viewport_d3d.  Height=lgi.Window.size_y;
	viewport_d3d.MinDepth=0;
	viewport_d3d.MaxDepth=1;
	ID3D11DeviceContext_RSSetViewports(lgi.d3d11.ctx,1,&viewport_d3d);

	lgi.Time.start_ticks = lgi_pollClock();
	lgi.Time.frame_ticks = lgi.Time.start_ticks;

	lgi_pollInput();
	lgi_beginMode2D();
}

LRESULT CALLBACK
lgi_windowMessageCallback_win32(HWND Window,UINT Message,WPARAM wParam,LPARAM lParam) {
	LRESULT result = lgi_windowMessageHandler_win32(Message,wParam,lParam);
	if(result != TRUE) {
		result = DefWindowProcW(Window,Message,wParam,lParam);
	}
	return result;
}

int lgi_windowMessageHandler_win32(UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CLOSE:
		case WM_QUIT: {
			PostQuitMessage(0);
			lgi.Window.isClosed = TRUE;
		} break;
		case WM_SIZE: {
			RECT client;
			if (GetClientRect(lgi.Window.win32.handle,&client)) {
				int size_x = client. right - client.left;
				int size_y = client.bottom - client. top;

				if (lgi.Window.size_x != size_x || lgi.Window.size_y != size_y) {
					lgi.Window.size_x = size_x;
					lgi.Window.size_y = size_y;
					lgi.Window.center_x=size_x>>1;
					lgi.Window.center_y=size_y>>1;

					D3D11_VIEWPORT viewport_d3d;
					viewport_d3d.TopLeftX=0;
					viewport_d3d.TopLeftY=0;
					viewport_d3d.   Width=size_x;
					viewport_d3d.  Height=size_y;
					viewport_d3d.MinDepth=0;
					viewport_d3d.MaxDepth=1;
					ID3D11DeviceContext_RSSetViewports(lgi.d3d11.ctx,1,&viewport_d3d);

					// lgi_deleteTexture(lgi.State.pip.out.color[0]);
					// lgi_deleteTexture(lgi.State.pip.out.depth);
					// lgi_deleteTexture(lgi.Window.Output.target);

					HRESULT error = IDXGISwapChain_ResizeBuffers(lgi.Window.Output.d3d11.stage,0,size_x,size_y,DXGI_FORMAT_UNKNOWN,0);
					lgi_ASSERT(SUCCEEDED(error));

					/* TODO: */
					// lgi.State.pip.out.count = 1;
					// lgi.State.pip.out.color[0] = lgi_makeColorTarget(lgi.Window.Output.target->size_x,lgi.Window.Output.target->size_y,lgi.Window.Output.target->format,lgi_MSAA,0);
					// lgi.State.pip.out.depth    = lgi_makeDepthTarget(lgi.Window.Output.target->size_x,lgi.Window.Output.target->size_y,DXGI_FORMAT_D32_FLOAT);

					lgi_setClippingZone(0,0,size_x,size_y);

					#if 1
					ID3D11Texture2D *texture_d3d;
					error = IDXGISwapChain_GetBuffer(lgi.Window.Output.d3d11.stage,0,&IID_ID3D11Texture2D,(void **)&texture_d3d);
					lgi_ASSERT(SUCCEEDED(error));

					lgi_Texture_Config config;
					ZeroMemory(&config,sizeof(config));
					config.memtype = D3D11_USAGE_DEFAULT;
					config.useflag = D3D11_BIND_RENDER_TARGET;
					config.size_x = lgi.Window.Output.target->size_x;
					config.size_y = lgi.Window.Output.target->size_y;
					config.format = DXGI_FORMAT_UNKNOWN;
					config.d3d11.texture_2d = texture_d3d;

					lgi.Window.Output.target = lgi_createTexture(&config);
					#endif
				}

			}
		} break;
		case WM_MOUSEMOVE: {
			lgi_setCursor(lgi.win32.cursor.arrow);

			int xcursor=GET_X_LPARAM(lParam);
			int ycursor=GET_Y_LPARAM(lParam);
			lgi.Input.Mice.xcursor=xcursor;
			lgi.Input.Mice.ycursor=lgi.Window.size_y-ycursor;
		} break;
		case WM_MOUSEWHEEL: {
			lgi.Input.Mice.yscroll = GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA;
		} break;
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK: {
			lgi_logInfo("Error:!");
		} break;
		case WM_LBUTTONUP: {
			// lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
			lgi.Input.Mice.newButtonState ^= 1 << 0;
		} break;
		case WM_RBUTTONUP: {
			// lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
			lgi.Input.Mice.newButtonState ^= 1 << 1;
		} break;
		case WM_MBUTTONUP: {
			// lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
			lgi.Input.Mice.newButtonState ^= 1 << 2;
		} break;
		case WM_LBUTTONDOWN: {
			// lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
			lgi.Input.Mice.newButtonState |= 1 << 0;
		} break;
		case WM_RBUTTONDOWN: {
			// lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
			lgi.Input.Mice.newButtonState |= 1 << 1;
		} break;
		case WM_MBUTTONDOWN: {
			// lgi.Input.Mice.oldButtonState = lgi.Input.Mice.newButtonState;
			lgi.Input.Mice.newButtonState |= 1 << 2;
		} break;

// SetCapture((HWND)lgi.Window.win32.handle);
// ReleaseCapture();
		case WM_CHAR: {
			lgi.Input.Keyboard.lastChar = wParam;
		} break;
		case WM_SYSKEYUP: case WM_SYSKEYDOWN: case WM_KEYUP: case WM_KEYDOWN: {

			lgi.Input.Keyboard.is_shft = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;
			lgi.Input.Keyboard.is_ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			lgi.Input.Keyboard.is_menu = (GetKeyState(VK_MENU)    & 0x8000) != 0;
#if defined(lgi_CLOSE_ON_ESCAPE)
			if (wParam == VK_ESCAPE) {
				lgi.Window.isClosed = TRUE;
			}
#endif
			/* todo: there's probably a better way to do this */

			int key_map = 0;
			if isWithin3(wParam,'a','z','A','Z','0','9') {
				key_map = wParam;
			}

			switch(wParam) {
				case VK_CONTROL:    key_map = rx_kLCTRL;     break;
				case VK_RETURN:     key_map = rxKEY_kRETURN;    break;
				case VK_BACK:       key_map = rx_kBCKSPC;    break;
				case VK_DELETE:     key_map = rx_kDELETE;    break;
				case VK_HOME:       key_map = rx_kHOME;      break;
				case VK_END:        key_map = rx_kEND;       break;
				case VK_ESCAPE:     key_map = rx_kESCAPE;    break;

				case VK_LEFT:       key_map = rxKEY_kLEFT;  break;
				case VK_RIGHT:      key_map = rxKEY_kRIGHT; break;
				case VK_UP:         key_map = rxKEY_kUP;    break;
				case VK_DOWN:       key_map = rxKEY_kDOWN;  break;

				case VK_F1:  key_map = rxKEY_kF1;  break;
				case VK_F2:  key_map = rxKEY_kF2;  break;
				case VK_F3:  key_map = rxKEY_kF3;  break;
				case VK_F4:  key_map = rxKEY_kF4;  break;
				case VK_F5:  key_map = rxKEY_kF5;  break;
				case VK_F6:  key_map = rxKEY_kF6;  break;
				case VK_F7:  key_map = rxKEY_kF7;  break;
				case VK_F8:  key_map = rxKEY_kF8;  break;
				case VK_F9:  key_map = rxKEY_kF9;  break;
				case VK_F10: key_map = rxKEY_kF10; break;
				case VK_F11: key_map = rxKEY_kF11; break;
				case VK_F12: key_map = rxKEY_kF12; break;
				/* waste of time */
				case VK_SPACE:      key_map =  ' '; break;
				case VK_OEM_PLUS:   key_map =  '='; break;
				case VK_OEM_PERIOD: key_map =  '.'; break;
				case VK_OEM_COMMA:  key_map =  ','; break;
				case VK_OEM_1:      key_map =  ';'; break;
				case VK_OEM_2:      key_map =  '/'; break;
				case VK_OEM_3:      key_map =  '~'; break;
				case VK_OEM_4:      key_map =  '['; break;
				case VK_OEM_5:      key_map = '\\'; break;
				case VK_OEM_6:      key_map =  ']'; break;
				case VK_OEM_7:      key_map = '\''; break;
			}

			lgi.Input.Keyboard.newKeyState[key_map] = (Message == WM_KEYDOWN) || (Message == WM_SYSKEYDOWN);
		} break;
		default: {
			return FALSE;
		}
	}
	return TRUE;
}

static void lgi__initDefaults() {
	lgi.defaultConstBlock = lgi_makeConstBuffer(sizeof(lgi_Uniform_Slots),NULL);

	//
	// NOTE: Create Default Programs:
	//

	// TODO:
	// Call A Function that generates a new program,
	// the program gets intialized to all the defaults,
	// then you can call attach vertex/pixel shader or blend state...

	lgi.defaultProgram.vertexShader = lgi_buildVertexShader(0, "default_vs", sizeof(g_MainVS),(void*)g_MainVS);
	lgi.defaultProgram.pixelShader = lgi_buildPixelShader(0, "default_ps", sizeof(g_MainPS),(void*)g_MainPS);

	lgi.lcdTextProgram.vertexShader = lgi.defaultProgram.vertexShader;
	lgi.lcdTextProgram.pixelShader = lgi_buildPixelShader(0, "ps_txt", sizeof(g_MainPS_Text),(void*)g_MainPS_Text);

	lgi.sdfTextProgram.vertexShader = lgi.defaultProgram.vertexShader;
	lgi.sdfTextProgram.pixelShader = lgi_buildPixelShader(0, "ps_txt_sdf", sizeof(g_MainPS_TextSDF),(void*)g_MainPS_TextSDF);

	lgi.sdfCircleProgram.vertexShader = lgi_buildVertexShader(0, "vs_sdf", sizeof(g_MainVS_SDF),(void*)g_MainVS_SDF);
	lgi.sdfCircleProgram.pixelShader = lgi_buildPixelShader(0, "ps_sdf_cir", sizeof(g_MainPS_CircleSDF),(void*)g_MainPS_CircleSDF);

	lgi.sdfBoxProgram.vertexShader = lgi.sdfCircleProgram.vertexShader;
	lgi.sdfBoxProgram.pixelShader = lgi_buildPixelShader(0, "ps_sdf_box", sizeof(g_MainPS_BoxSDF),(void*)g_MainPS_BoxSDF);


	D3D11_BLEND_DESC blendConfig; lgi__clear_typeof(&blendConfig);
	blendConfig.RenderTarget[0].          BlendEnable=TRUE;
	blendConfig.RenderTarget[0].             SrcBlend=D3D11_BLEND_SRC_ALPHA;
	blendConfig.RenderTarget[0].            DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
	blendConfig.RenderTarget[0].              BlendOp=D3D11_BLEND_OP_ADD;
	blendConfig.RenderTarget[0].        SrcBlendAlpha=D3D11_BLEND_ZERO;
	blendConfig.RenderTarget[0].       DestBlendAlpha=D3D11_BLEND_ZERO;
	blendConfig.RenderTarget[0].         BlendOpAlpha=D3D11_BLEND_OP_ADD;
	blendConfig.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
	ID3D11Device_CreateBlendState(lgi.d3d11.dev, &blendConfig,&lgi.d3d11.defaultBlendState);

	blendConfig.RenderTarget[0].BlendEnable = TRUE;
	blendConfig.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC1_COLOR;
	blendConfig.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC1_COLOR;
	blendConfig.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendConfig.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendConfig.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendConfig.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendConfig.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ID3D11BlendState *lcdTextBlendState;
	ID3D11Device_CreateBlendState(lgi.d3d11.dev, &blendConfig,&lcdTextBlendState);

	lgi.defaultProgram.vertexShader->inputBlock = lgi.defaultConstBlock;
	lgi.defaultProgram.pixelShader->inputBlock = lgi.defaultConstBlock;
	lgi.lcdTextProgram.vertexShader->inputBlock = lgi.defaultConstBlock;
	lgi.lcdTextProgram.pixelShader->inputBlock = lgi.defaultConstBlock;
	lgi.sdfTextProgram.vertexShader->inputBlock = lgi.defaultConstBlock;
	lgi.sdfTextProgram.pixelShader->inputBlock = lgi.defaultConstBlock;
	lgi.sdfCircleProgram.vertexShader->inputBlock = lgi.defaultConstBlock;
	lgi.sdfCircleProgram.pixelShader->inputBlock = lgi.defaultConstBlock;
	lgi.sdfBoxProgram.vertexShader->inputBlock = lgi.defaultConstBlock;
	lgi.sdfBoxProgram.pixelShader->inputBlock = lgi.defaultConstBlock;

	lgi.defaultProgram.d3d11.blendState = lgi.d3d11.defaultBlendState;
	lgi.lcdTextProgram.d3d11.blendState = lcdTextBlendState;
	lgi.sdfTextProgram.d3d11.blendState = lgi.d3d11.defaultBlendState;
	lgi.sdfCircleProgram.d3d11.blendState = lgi.d3d11.defaultBlendState;
	lgi.sdfBoxProgram.d3d11.blendState = lgi.d3d11.defaultBlendState;


	lgi.State.indexBuffer = lgi_makeIndexBuffer(sizeof(lgi_Index), lgi_DEFAULT_INDEX_BUFFER_LENGTH);
	lgi.State.vertexBuffer = lgi_makeVertexBuffer(sizeof(lgi_Vertex), lgi_DEFAULT_VERTEX_BUFFER_LENGTH);


	lgi.whiteTexture = lgi_makeTexture(512,512,lgi_Format_R8G8B8A8_UNORM,0,NULL);
	lgi_clearTexture(lgi.whiteTexture,0xff);

	lgi.defaultColorTarget = lgi_makeColorTarget(lgi.Window.Output.target->size_x,lgi.Window.Output.target->size_y,lgi.Window.Output.target->format,lgi_MSAA,0);
	lgi.defaultDepthTarget = lgi_makeDepthTarget(lgi.Window.Output.target->size_x,lgi.Window.Output.target->size_y,DXGI_FORMAT_D32_FLOAT);

	D3D11_DEPTH_STENCIL_DESC stencil_config_d3d;
	stencil_config_d3d.     DepthEnable=FALSE;
	stencil_config_d3d.   StencilEnable=FALSE;
	stencil_config_d3d.  DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
	stencil_config_d3d.       DepthFunc=D3D11_COMPARISON_LESS;
	stencil_config_d3d. StencilReadMask=D3D11_DEFAULT_STENCIL_READ_MASK;
	stencil_config_d3d.StencilWriteMask=D3D11_DEFAULT_STENCIL_WRITE_MASK;
	stencil_config_d3d.FrontFace.      StencilFailOp=D3D11_STENCIL_OP_KEEP;
	stencil_config_d3d.FrontFace. StencilDepthFailOp=D3D11_STENCIL_OP_DECR;
	stencil_config_d3d.FrontFace.      StencilPassOp=D3D11_STENCIL_OP_KEEP;
	stencil_config_d3d.FrontFace.        StencilFunc=D3D11_COMPARISON_ALWAYS;
	stencil_config_d3d. BackFace.      StencilFailOp=D3D11_STENCIL_OP_KEEP;
	stencil_config_d3d. BackFace. StencilDepthFailOp=D3D11_STENCIL_OP_DECR;
	stencil_config_d3d. BackFace.      StencilPassOp=D3D11_STENCIL_OP_KEEP;
	stencil_config_d3d. BackFace.        StencilFunc=D3D11_COMPARISON_ALWAYS;
	ID3D11Device_CreateDepthStencilState(lgi.d3d11.dev,&stencil_config_d3d,&lgi.d3d11.defaultStencilState);


	// NOTE: Create Default Rasterizer State:
	//

	D3D11_RASTERIZER_DESC2 rasterizerConfig;
	ZeroMemory(&rasterizerConfig,sizeof(rasterizerConfig));
	rasterizerConfig.             FillMode=D3D11_FILL_SOLID;
	rasterizerConfig.             CullMode=D3D11_CULL_NONE;
	rasterizerConfig.FrontCounterClockwise=FALSE;
	rasterizerConfig.            DepthBias=D3D11_DEFAULT_DEPTH_BIAS;
	rasterizerConfig.       DepthBiasClamp=D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerConfig. SlopeScaledDepthBias=D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerConfig.      DepthClipEnable= FALSE;
	rasterizerConfig.        ScissorEnable= TRUE;
	rasterizerConfig.    MultisampleEnable=lgi_MSAA >= 2;
	rasterizerConfig.AntialiasedLineEnable=FALSE;
	rasterizerConfig.ConservativeRaster=D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	ID3D11Device3 *lpDevice3;
	HRESULT error = IUnknown_QueryInterface(lgi.d3d11.dev,&IID_ID3D11Device3,(void**)&lpDevice3);
	lgi_ASSERT(SUCCEEDED(error));

	ID3D11RasterizerState2 *rasterizer_d3d;
	error = lpDevice3->lpVtbl->CreateRasterizerState2(lpDevice3,&rasterizerConfig,&rasterizer_d3d);
	lgi_ASSERT(SUCCEEDED(error));

	error = IUnknown_QueryInterface(rasterizer_d3d,&IID_ID3D11RasterizerState,(void**)&lgi.d3d11.defaultRasterizerState);
	lgi_ASSERT(SUCCEEDED(error));

	IUnknown_Release(lpDevice3);
	IUnknown_Release(rasterizer_d3d);

	//
	// TODO: Do This At The Right Place!
	//
	unsigned int Stride=sizeof(lgi_Vertex);
	unsigned int Offset=0;
	ID3D11DeviceContext_IASetVertexBuffers(lgi.d3d11.ctx,0,1,&lgi.State.vertexBuffer->d3d11.buffer,&Stride,&Offset);
	ID3D11DeviceContext_IASetIndexBuffer(lgi.d3d11.ctx,lgi.State.indexBuffer->d3d11.buffer,DXGI_FORMAT_R32_UINT,0);
	ID3D11DeviceContext_IASetPrimitiveTopology(lgi.d3d11.ctx,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11DeviceContext_RSSetState(lgi.d3d11.ctx,lgi.d3d11.defaultRasterizerState);
	ID3D11DeviceContext_OMSetDepthStencilState(lgi.d3d11.ctx,lgi.d3d11.defaultStencilState,1);
	ID3D11DeviceContext_OMSetRenderTargets(lgi.d3d11.ctx,1,&lgi.defaultColorTarget->d3d11.color_target,lgi.defaultDepthTarget->d3d11.depth_target);
}

#pragma warning(pop)
#endif

