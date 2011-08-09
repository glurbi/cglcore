#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <files.h>

char* readTextFile(const char* filename) {
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    char* content = (char*) malloc((size+1)*sizeof(char));
    content[size] = 0;
	// we need to read as binary, not text, otherwise we are screwed on Windows
    FILE *file = fopen(filename, "rb"); 
    fread(content, 1, size, file);
    return content;
}

char* readPngFile(const char* filename, int *width, int *height, GLenum *format) {
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    fp = fopen(filename, "rb");

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    if (info_ptr->color_type == PNG_COLOR_TYPE_RGB) {
        *format = GL_RGB;
    } else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        *format = GL_RGBA;
    } else {
        printf("Unsupported color type in readPngFile(%s)", filename);
        return NULL;
    }

    *width = info_ptr->width;
    *height = info_ptr->height;

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep image_data = (png_bytep) malloc(rowbytes * info_ptr->height);
    png_bytepp row_pointers = (png_bytepp) malloc(info_ptr->height * sizeof(png_bytep));

    for (unsigned int i = 0; i < info_ptr->height; ++i) {
      row_pointers[info_ptr->height - 1 - i] = image_data + i * rowbytes;
    }

    png_read_image(png_ptr, row_pointers);
    //png_read_destroy(png_ptr, info_ptr, (png_infop)0);
    free(png_ptr);
    free(info_ptr);
    free(row_pointers);
    fclose(fp);
    return (char*) image_data;
}
