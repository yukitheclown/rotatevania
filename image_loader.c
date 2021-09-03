#include "image_loader.h"
#include "log.h"
#include <GL/glew.h>
#include <png.h>
#include <string.h>

static unsigned char *LoadDataIntoStack(char *path, int *w, int *h){

    FILE *fp = fopen(path,"rb");

    if( fp == NULL ){
        LOGF(LOG_YELLOW, "Error loading PNG %s: No such file.\n", path);
        return NULL;
    }

    unsigned char header[8];
    fread(header, 1, 8, fp);
    int ispng = !png_sig_cmp(header, 0, 8);

    if(!ispng){
        fclose(fp);
        LOGF(LOG_YELLOW, "Not png %s\n", path);
        return NULL;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr) {
        LOGF(LOG_YELLOW, "Error loading %s\n",path );
        fclose(fp);
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr){
        LOGF(LOG_YELLOW, "Error loading %s\n",path );
        fclose(fp);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    if(setjmp(png_jmpbuf(png_ptr))){
        LOGF(LOG_YELLOW, "Error loading %s\n",path );
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    png_set_sig_bytes(png_ptr, 8);
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_uint_32 twidth, theight;

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if(bit_depth < 8)
        png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    *w = twidth;
    *h = theight;

    png_read_update_info(png_ptr, info_ptr);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    png_byte *imageData = (png_byte *)Memory_AllocStack(sizeof(png_byte) * rowbytes * theight, TMP_STACK);
    if(!imageData){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    png_bytep *row_pointers = (png_bytep *) Memory_AllocStack(sizeof(png_bytep) * theight, TMP_STACK);
    if(!row_pointers){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        Memory_PopStack(1, TMP_STACK);
        fclose(fp);
        return NULL;
    }

    int i;
    for(i = 0; i < (int)theight; ++i)
        row_pointers[theight - 1 - i] = imageData + i * rowbytes;

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    Memory_PopStack(1, TMP_STACK);

    fclose(fp);

    return (unsigned char *)imageData;
}

Image ImageLoader_CreateAtlas(char **paths, int nPaths, IRect2D *specs){

    Image ret;
    memset(&ret, 0, sizeof(Image));

    glGenTextures(1, &ret.glTexture);
    glBindTexture(GL_TEXTURE_2D, ret.glTexture);

    void **pointers = Memory_AllocStack(sizeof(void *) * nPaths,TMP_STACK);


    int k, num = 0;
    for(k = 0; k < nPaths; k++){

        int w, h;
        if(!(pointers[num] = LoadDataIntoStack(paths[k], &w, &h))){
            specs[k].w = -1;
            specs[k].h = -1;
            continue;
        }

        specs[k].w = w;
        specs[k].h = h;

        ++num;

        ret.w += w;
        ret.h = h < ret.h ? ret.h : h;
    }

    if(!num){
        Memory_PopStack(1,TMP_STACK);
        return ret;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret.w, ret.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    int x = 0;

    for(k = 0; k < nPaths; k++){

        int w = specs[k].w;
        int h = specs[k].h;

        if(w < 0) continue;

        int y = ret.h - h;

        glTexSubImage2D(GL_TEXTURE_2D,0,x,y,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pointers[k]);

        specs[k].x = x;
        specs[k].y = 0;

        x += w;
    }    

    Memory_PopStack(num + 1,TMP_STACK);

    ImageLoader_SetImageParameters(ret, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    return ret;
}


Image ImageLoader_CreateImage(char *path){

    Image ret;
    memset(&ret, 0, sizeof(Image));

    unsigned char *imageData = LoadDataIntoStack(path, &ret.w, &ret.h);

    if(!imageData)
        return ret;

    glGenTextures(1, &ret.glTexture);
    glBindTexture(GL_TEXTURE_2D, ret.glTexture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret.w, ret.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    ImageLoader_SetImageParameters(ret, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);

    Memory_PopStack(1, TMP_STACK);

    glBindTexture(GL_TEXTURE_2D, 0);

    return ret;
}

void ImageLoader_SetImageParameters(Image img, int minFilter, int magFilter, int glTextureWrapS, int glTextureWrapT){
    glBindTexture(GL_TEXTURE_2D, img.glTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,glTextureWrapS);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,glTextureWrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ImageLoader_DeleteImage(Image *img){
    glDeleteTextures(1, &img->glTexture);
    img->glTexture = 0;
}