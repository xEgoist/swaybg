#include <assert.h>
#include "background-image.h"
#include "cairo_util.h"
#include "log.h"

enum background_gravity parse_background_gravity(const char *gravity) {
	if (strcmp(gravity, "north") == 0) {
		return BACKGROUND_GRAVITY_NORTH;
	} else if (strcmp(gravity, "south") == 0) {
		return BACKGROUND_GRAVITY_SOUTH;
	} else if (strcmp(gravity, "east") == 0) {
		return BACKGROUND_GRAVITY_EAST;
	} else if (strcmp(gravity, "west") == 0) {
		return BACKGROUND_GRAVITY_WEST;
	} else if (strcmp(gravity, "center") == 0) {
		return BACKGROUND_GRAVITY_CENTER;
	}
	swaybg_log(LOG_ERROR, "Unsupported background gravity: %s", gravity);
	return BACKGROUND_GRAVITY_INVALID;
}

enum background_mode parse_background_mode(const char *mode) {
	if (strcmp(mode, "stretch") == 0) {
		return BACKGROUND_MODE_STRETCH;
	} else if (strcmp(mode, "fill") == 0) {
		return BACKGROUND_MODE_FILL;
	} else if (strcmp(mode, "fit") == 0) {
		return BACKGROUND_MODE_FIT;
	} else if (strcmp(mode, "center") == 0) {
		return BACKGROUND_MODE_CENTER;
	} else if (strcmp(mode, "tile") == 0) {
		return BACKGROUND_MODE_TILE;
	} else if (strcmp(mode, "solid_color") == 0) {
		return BACKGROUND_MODE_SOLID_COLOR;
	}
	swaybg_log(LOG_ERROR, "Unsupported background mode: %s", mode);
	return BACKGROUND_MODE_INVALID;
}

cairo_surface_t *load_background_image(const char *path) {
	cairo_surface_t *image;
#if HAVE_GDK_PIXBUF
	GError *err = NULL;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, &err);
	if (!pixbuf) {
		swaybg_log(LOG_ERROR, "Failed to load background image (%s).",
				err->message);
		return NULL;
	}
	image = gdk_cairo_image_surface_create_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
#else
	image = cairo_image_surface_create_from_png(path);
#endif // HAVE_GDK_PIXBUF
	if (!image) {
		swaybg_log(LOG_ERROR, "Failed to read background image.");
		return NULL;
	}
	if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
		swaybg_log(LOG_ERROR, "Failed to read background image: %s."
#if !HAVE_GDK_PIXBUF
				"\nSway was compiled without gdk_pixbuf support, so only"
				"\nPNG images can be loaded. This is the likely cause."
#endif // !HAVE_GDK_PIXBUF
				, cairo_status_to_string(cairo_surface_status(image)));
		return NULL;
	}
	return image;
}

void render_background_image(cairo_t *cairo, cairo_surface_t *image,
		enum background_mode mode, enum background_gravity gravity, int buffer_width, int buffer_height) {
	double width = cairo_image_surface_get_width(image);
	double height = cairo_image_surface_get_height(image);

	cairo_save(cairo);
	switch (mode) {
	case BACKGROUND_MODE_STRETCH:
		cairo_scale(cairo,
				(double)buffer_width / width,
				(double)buffer_height / height);
		cairo_set_source_surface(cairo, image, 0, 0);
		break;
	case BACKGROUND_MODE_FILL: {
		double window_ratio = (double)buffer_width / buffer_height;
		double bg_ratio = width / height;

		if (window_ratio > bg_ratio) {
			double scale = (double)buffer_width / width;
			cairo_scale(cairo, scale, scale);
			if (gravity == BACKGROUND_GRAVITY_NORTH ) {
					cairo_set_source_surface(cairo, image, 0, 0);
			} else if (gravity == BACKGROUND_GRAVITY_SOUTH) {
					cairo_set_source_surface(cairo, image, 0,
						 ((double)buffer_height - scale * height  ) / scale );
			} else if (gravity == BACKGROUND_GRAVITY_CENTER) {
					cairo_set_source_surface(cairo, image, 0,
						 ((double)buffer_height / 2 / scale - height / 2) );
			} else {
					swaybg_log(LOG_ERROR, "Unsupported gravity for this ratio. Defaulting to center");
					cairo_set_source_surface(cairo, image, 0,
						 ((double)buffer_height / 2 / scale - height / 2) );
				}
		} else {
			double scale = (double)buffer_height / height;
			cairo_scale(cairo, scale, scale);
			if (gravity == BACKGROUND_GRAVITY_WEST) {
				cairo_set_source_surface(cairo, image, 0, 0);
			} else if (gravity == BACKGROUND_GRAVITY_EAST) {
				cairo_set_source_surface(cairo, image,
						((double)buffer_width / 2 / scale - width / 2) * 2, 0);
			} else if (gravity == BACKGROUND_GRAVITY_CENTER) {
				cairo_set_source_surface(cairo, image,
					(double)buffer_width / 2 / scale - width / 2, 0);
			} else {
				swaybg_log(LOG_ERROR, "Unsupported gravity for this ratio. Defaulting to center");
				cairo_set_source_surface(cairo, image,
					(double)buffer_width / 2 / scale - width / 2, 0);
				}
		}
		break;
	}
	case BACKGROUND_MODE_FIT: {
		double window_ratio = (double)buffer_width / buffer_height;
		double bg_ratio = width / height;

		if (window_ratio > bg_ratio) {
			double scale = (double)buffer_height / height;
			cairo_scale(cairo, scale, scale);
			cairo_set_source_surface(cairo, image,
					(double)buffer_width / 2 / scale - width / 2, 0);
		} else {
			double scale = (double)buffer_width / width;
			cairo_scale(cairo, scale, scale);
			cairo_set_source_surface(cairo, image,
					0, (double)buffer_height / 2 / scale - height / 2);
		}
		break;
	}
	case BACKGROUND_MODE_CENTER:
		cairo_set_source_surface(cairo, image,
				(double)buffer_width / 2 - width / 2,
				(double)buffer_height / 2 - height / 2);
		break;
	case BACKGROUND_MODE_TILE: {
		cairo_pattern_t *pattern = cairo_pattern_create_for_surface(image);
		cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
		cairo_set_source(cairo, pattern);
		cairo_pattern_destroy(pattern);
		break;
	}
	case BACKGROUND_MODE_SOLID_COLOR:
	case BACKGROUND_MODE_INVALID:
		assert(0);
		break;
	}
	cairo_paint(cairo);
	cairo_restore(cairo);
}
