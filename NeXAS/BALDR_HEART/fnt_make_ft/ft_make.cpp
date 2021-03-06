#include "ft_make.h"

FT_Make::FT_Make(string font_path, DWORD font_height)
{
	if (!FT_Init(font_path, font_height))
	{
		cout << "ft Init fail!\n";
		exit(0);
	}
}

bool FT_Make::FT_Init(string font_path, DWORD font_height)
{
	FT_Error error;
	ifstream infile;
	infile.open(font_path, ios::in | ios::binary | ios::ate);
	if (!infile.is_open())
	{
		cout << font_path + " open fail!\n";
		exit(0);
	}
	DWORD filesize = (DWORD)infile.tellg();
	infile.seekg(0, ios::beg);
	char* buff = new char[filesize];
	infile.read(buff, filesize);
	infile.close();
	error = FT_Init_FreeType(&library);
	if (error)
	{
		cout << "Init_FreeType Error!\n";
		return false;
	}
	cout << "read ttf...\n";
	error = FT_New_Memory_Face(library,(BYTE *)buff, filesize, 0, &face);
	if (error)
	{
		cout << "New_Memory_Face Error!\n";
		return false;
	}
	error = FT_Set_Char_Size(face, font_height * 64, font_height * 64, 0, 0);
	if (error)
	{
		cout << "Set_Char_Size Error!\n";
		return false;
	}
	error = FT_Set_Pixel_Sizes(face, font_height, font_height);
	if (error)
	{
		cout << "Set_Pixel_Sizes Error!\n";
		return false;
	}
	return true;
}

CharBitmap FT_Make::GetCharBitmap(WCHAR wchar)
{
	FT_GlyphSlot slot = face->glyph;
	FT_Error error;
	FT_Bitmap bmp;
	CharBitmap cbmp;
	//似乎加不加都一样，可能要FT_Outline_Embolden，不过现在效果足够了
	FT_Bitmap_Embolden(library,&face->glyph->bitmap, 48 << 6, 48 << 6);
	error = FT_Load_Char(face, wchar, FT_LOAD_RENDER);
	if (error)
	{
		cout << "Load_Char Error!\n";
		exit(0);
	}
	bmp = slot->bitmap;
	cbmp.bmp_width = bmp.width;
	cbmp.bmp_height = bmp.rows;
	cbmp.bearingX = slot->bitmap_left;
	cbmp.bearingY = slot->bitmap_top;
	cbmp.Advance = slot->advance.x / 64;
	cbmp.bmpBuffer = bmp.buffer;
	return cbmp;
}

FT_Make::~FT_Make()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void WritePng(FILE *pngfile, DWORD width, DWORD height, BYTE* data)
{
	png_structp png_ptr;
	png_infop info_ptr;
	DWORD i = 0, k = 0;
	BYTE *dst, *src;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		cout << "PNG create fail!\n";
		exit(0);
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		cout << "info create fail!\n";
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		exit(0);
	}
	png_init_io(png_ptr, pngfile);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	dst = new BYTE[width*height * 4];
	src = data;
	for (i = 0, k = 0; i < width*height; i++, k++)
	{
		if (src[i] == 0)
		{
			dst[k * 4 + 0] = 0;
			dst[k * 4 + 1] = 0;
			dst[k * 4 + 2] = 0;
		}
		else
		{
			dst[k * 4 + 0] = 0xFF;
			dst[k * 4 + 1] = 0xFF;
			dst[k * 4 + 2] = 0xFF;
		}
		dst[k * 4 + 3] = src[i];
	}
	for (i = 0; i < height; i++)
		png_write_row(png_ptr, dst + i*width * 4);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete[] dst;
}