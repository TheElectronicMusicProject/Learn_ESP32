#include <stdio.h>

void
app_main (void)
{
    // Pointer to an unsigned char array.
    //
    extern const unsigned char index_html[] asm("_binary_index_html_start");
    printf("html = %s\n", index_html);

    extern const unsigned char sample_txt[] asm("_binary_sample_txt_start");
    printf("sample = %s\n", sample_txt);

    extern const unsigned char img_start[] asm("_binary_pinout_jpg_start");
    extern const unsigned char img_end[] asm("_binary_pinout_jpg_end");
    const unsigned int img_size = img_end - img_start;
    printf("image size is %d\n", img_size);
}
