#include "r_skybox.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "common/nums.h"

// Generate cubemap texture from HDR texture
static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format) {
	TextureCubemap cubemap = (TextureCubemap) {0};
	
	rlDisableBackfaceCulling();		// Disable backface culling to render inside the cube
	
	// ------------------------------------------------------------------------------------------------------------
	// Step 1. setup framebuffer
	// **
	u32 rbo = rlLoadTextureDepth(size, size, true);
	cubemap.id = rlLoadTextureCubemap(0, size, format, 1);

	u32 fbo = rlLoadFramebuffer();
	rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
	rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

	// Check if framebuffer is valid
	if(rlFramebufferComplete(fbo)) TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);	
	
	// ------------------------------------------------------------------------------------------------------------
	// Step 2. draw to framebuffer
	// **
	// NOTE: Shader is used to convert HDR equirectangular env map to cubemap equivalent (six faces)
	rlEnableShader(shader.id);

	// Define projection matrix and send to shader
	Matrix matFboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, rlGetCullDistanceNear(), rlGetCullDistanceFar());
	rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

	// Define view matrix for all six sides of the cubemap 
	Matrix fboViews[6] = {
		MatrixLookAt(Vector3Zero(), (Vector3) {  1,  0,  0 }, (Vector3) {  0, -1,  0 } ),
		MatrixLookAt(Vector3Zero(), (Vector3) { -1,  0,  0 }, (Vector3) {  0, -1,  0 } ),
		MatrixLookAt(Vector3Zero(), (Vector3) {  0,  1,  0 }, (Vector3) {  0,  0,  1 } ),
		MatrixLookAt(Vector3Zero(), (Vector3) {  0, -1,  0 }, (Vector3) {  0,  0, -1 } ),
		MatrixLookAt(Vector3Zero(), (Vector3) {  0,  0,  1 }, (Vector3) {  0, -1,  0 } ),
		MatrixLookAt(Vector3Zero(), (Vector3) {  0,  0, -1 }, (Vector3) {  0, -1,  0 } ),
	};

	// Set viewport to current fbo dimensions
	rlViewport(0, 0, size, size);

	// Activate and enable texture for drawing to cubemap faces
	rlActiveTextureSlot(0);
	rlEnableTexture(panorama.id);

	for(int i = 0; i < 6; i++) {
		// Set view matrix for current cube face
		rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);

		// Select the current cubemap face attachment for the fbo
		// NOTE: This function by default enables -> attaches -> disables fbo. 
		rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);
		rlEnableFramebuffer(fbo);

		// Load and draw a cube using current enabled texture
		rlClearScreenBuffers();
		rlLoadDrawCube();
	}

	// ------------------------------------------------------------------------------------------------------------
	// Step 3. unload framebuffer and reset state 
	rlDisableTexture();
	rlDisableFramebuffer();
	rlUnloadFramebuffer(fbo);
	rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
	rlEnableBackfaceCulling();

	// ------------------------------------------------------------------------------------------------------------
	cubemap.width = size;
	cubemap.height = size;
	cubemap.mipmaps = 1;
	cubemap.format = format;

	return cubemap;
}

Skybox skybox_Init(const char *img_path, int size, int format) {
	Skybox skybox = (Skybox) {0};

	const char *path_pref = "resources/shaders";
	skybox.shader = LoadShader(TextFormat("%s/skybox_v.glsl", path_pref), TextFormat("%s/skybox_f.glsl", path_pref));

	Mesh mesh = GenMeshCube(100, 100, 100);
	skybox.model = LoadModelFromMesh(mesh);

	Image img = LoadImage(img_path);
	skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT); 
	//SetTextureFilter(skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture, TEXTURE_FILTER_TRILINEAR);
	skybox.model.materials[0].shader = skybox.shader;
	UnloadImage(img);

    SetShaderValue(
			skybox.model.materials[0].shader,
			GetShaderLocation(skybox.model.materials[0].shader, "environmentMap"),
			(int[1]) { MATERIAL_MAP_CUBEMAP },
			SHADER_UNIFORM_INT
			);

    SetShaderValue(
			skybox.model.materials[0].shader,
			GetShaderLocation(skybox.model.materials[0].shader, "doGamma"),
			(int[1]){ skybox.useHDR ? 1 : 0 },
			SHADER_UNIFORM_INT
			);

    SetShaderValue(
			skybox.model.materials[0].shader,
			GetShaderLocation(skybox.model.materials[0].shader, "vflipped"),
			(int[1]) { skybox.useHDR ? 1 : 0 },
			SHADER_UNIFORM_INT
			);

	return skybox;
}

void skybox_Close(Skybox *skybox) {
	UnloadTexture(skybox->model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	UnloadModel(skybox->model);
	UnloadShader(skybox->shader);

	*skybox = (Skybox) {0};
}

void skybox_Render(Skybox *skybox) {
	rlDisableBackfaceCulling();
	rlDisableDepthMask();

	DrawModel(skybox->model, Vector3Zero(), 1.0f, WHITE);

	rlEnableDepthMask();
	rlEnableBackfaceCulling();
}

