#include "render.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <utility>

void WorldToScreen(double x, double y, double* out_x, double* out_y) {
	*out_x = x;
	*out_y = WINDOW_HEIGHT - y;
}

Color GameColourToRaylibColor(const GameColour* gc) {
	Color result{};
	result.r = gc->r;
	result.g = gc->g;
	result.b = gc->b;
	result.a = gc->a;
	return result;
}

void RenderRectangleObstacle(const RenderData* data, const EntityRectangleObstacle* rect) {
	double top_left_x_w = 0;
	double top_left_y_w = 0;
	double bottom_right_x_w = 0;
	double bottom_right_y_w = 0;
	AABB_GetCornerCoords(&rect->aabb, AABB_TOP_LEFT, &top_left_x_w, &top_left_y_w);
	AABB_GetCornerCoords(&rect->aabb, AABB_BOTTOM_RIGHT, &bottom_right_x_w, &bottom_right_y_w);

	double top_left_x = 0;
	double top_left_y = 0;
	double bottom_right_x = 0;
	double bottom_right_y = 0;
	WorldToScreen(top_left_x_w, top_left_y_w, &top_left_x, &top_left_y);
	WorldToScreen(bottom_right_x_w, bottom_right_y_w, &bottom_right_x, &bottom_right_y);

	auto w = bottom_right_x - top_left_x;
	auto h = bottom_right_y - top_left_y;
	auto color = GameColourToRaylibColor(&rect->colour);
	DrawRectangle(top_left_x - data->camera.start_x, top_left_y - data->camera.start_y, w, h, color);
}

void RenderEntity(const RenderData* renderer, const Entity* entity) {
	switch (entity->type) {
		case ENTITY_RECTANGLE: { RenderRectangleObstacle(renderer, &entity->entity.rect); break; }
		default: { assert("RenderEntity: Unaccounted entity type encountered." && false); }
	}
}

void RenderEntities(const RenderData* renderer, const SandGame* game) {
	static const RenderData* rr;
	rr = renderer;
	auto cb = [](Entity* ent) {
		RenderEntity(rr, ent);
	};
	SandGame_ForEachEntity(game, cb);
}

void InitSandTexture(RenderData* renderer) {
	auto img = GenImageColor(PIT_WIDTH, PIT_HEIGHT, BLANK);
	renderer->sand_pixel_buffer = (Color*)malloc(PIT_WIDTH * PIT_HEIGHT * sizeof(Color));
	memset(renderer->sand_pixel_buffer, 0, PIT_WIDTH * PIT_HEIGHT * sizeof(Color));
	renderer->sand_texture = LoadTextureFromImage(img);
	UnloadImage(img);
}

void InitRenderCamera(RenderCamera* camera) {
	camera->start_x = 0;
	camera->start_y = 0;
	camera->w = WINDOW_WIDTH;
	camera->h = WINDOW_HEIGHT;
}

void Render_Init(RenderData* data, std::uint32_t window_w, std::uint32_t window_h, std::uint8_t fps, const char* window_name) {
	InitWindow(window_w, window_h, window_name);
	SetTargetFPS(fps);
	InitRenderCamera(&data->camera);
	InitSandTexture(data);
}

void Render_Shutdown(RenderData* renderer) {
	UnloadTexture(renderer->sand_texture);
	free(renderer->sand_pixel_buffer);
	CloseWindow();
}

void BeginRendering() {
	BeginDrawing();
	ClearBackground(BLACK);
	DrawFPS(20, 20);
}

void EndRendering() {
	EndDrawing();
}

void RenderSand(const RenderData* r, const SandPit* pit) {
	// Hacky....
	static auto start_x = 0;
	static auto start_y = 0;
	static auto* sand_pixel_buffer = r->sand_pixel_buffer;		// ASSUMES BUFFER DOES NOT MOVE IN MEMORY...
	auto grain_size = pit->grain_size;
	start_x = r->camera.start_x / grain_size;
	start_y = r->camera.start_y / grain_size;

	// Clear texture
    memset(r->sand_pixel_buffer, 0, PIT_WIDTH * PIT_HEIGHT * sizeof(Color));
    auto cb = [](TWO_BIT_ID id, uint32_t x, uint32_t y) {
		if (x >= start_x && x <= start_x + PIT_WIDTH && y >= start_y && y <= start_y + PIT_HEIGHT) {
			x -= start_x;
			y -= start_y;
			sand_pixel_buffer[(PIT_HEIGHT - y - 1) * PIT_WIDTH + x] = PARTICLE_COLOURS[id];
		}
    };
    SandPit_ForEachGrain(pit, cb);

    UpdateTexture(r->sand_texture, sand_pixel_buffer);
    DrawTextureEx(r->sand_texture, {0, 0}, 0.0, (float)grain_size, WHITE);
}

void RenderPlayer(const RenderData* r, const Player* player) {
	auto start_x = r->camera.start_x;
	auto start_y = r->camera.start_y;
	double player_x_w = 0;
	double player_y_w = 0;
	AABB_GetCornerCoords(&player->bbox, AABB_TOP_LEFT, &player_x_w, &player_y_w);
	double player_w = AABB_GetWidth(&player->bbox);
	double player_h = AABB_GetHeight(&player->bbox);

	double player_x = 0;
	double player_y = 0;
	WorldToScreen(player_x_w, player_y_w, &player_x, &player_y);
	Rectangle player_rect = { 
		player_x - start_x, 
		player_y - start_y, 
		player_w, player_h 
	};
	DrawRectangleRec(player_rect, RED);
}

void MoveCamera(RenderCamera* camera, const SandGame* game) {
	double player_x = 0;
	double player_y = 0;
	AABB_GetCornerCoords(&game->player.bbox, AABB_BOTTOM, &player_x, &player_y);
	double player_w = AABB_GetWidth(&game->player.bbox);
	double player_h = AABB_GetHeight(&game->player.bbox);

	auto start_x = ((int)player_x / camera->w) * camera->w;
	auto start_y = ((int)player_y / camera->h) * camera->h;
	camera->start_x = start_x;
	camera->start_y = start_y;
}

void Render_RenderGame(RenderData* r, const SandGame* game) {
	BeginRendering();

	MoveCamera(&r->camera, game);
	RenderEntities(r, game);
	RenderPlayer(r, &game->player);
	RenderSand(r, &game->pit);

	EndRendering();
}

bool Render_ShouldGameClose(const RenderData* data) {
	return WindowShouldClose();
}

