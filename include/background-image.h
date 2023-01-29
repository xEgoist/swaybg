#ifndef _SWAY_BACKGROUND_IMAGE_H
#define _SWAY_BACKGROUND_IMAGE_H
#include "cairo_util.h"

enum background_mode {
	BACKGROUND_MODE_STRETCH,
	BACKGROUND_MODE_FILL,
	BACKGROUND_MODE_FIT,
	BACKGROUND_MODE_CENTER,
	BACKGROUND_MODE_TILE,
	BACKGROUND_MODE_SOLID_COLOR,
	BACKGROUND_MODE_INVALID,
};

enum background_gravity {
	BACKGROUND_GRAVITY_NORTH,
	BACKGROUND_GRAVITY_SOUTH,
	BACKGROUND_GRAVITY_EAST,
	BACKGROUND_GRAVITY_WEST,
	BACKGROUND_GRAVITY_CENTER,
	BACKGROUND_GRAVITY_INVALID,
};

enum background_mode parse_background_mode(const char *mode);
cairo_surface_t *load_background_image(const char *path);
enum background_gravity parse_background_gravity(const char *gravity);
void render_background_image(cairo_t *cairo, cairo_surface_t *image,
		enum background_mode mode, enum background_gravity gravity, int buffer_width, int buffer_height);

#endif
