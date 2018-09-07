#ifndef ANTUTILS_H
#define ANTUTILS_H

#include <QFile>
#include <QTextStream>
#include <QDataStream>


namespace utils{
void writeByteToFile(char *fileName, char *buffer, int size) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.writeBytes(buffer, size);
}

int rgb32_to_i420(int width, int height, const char * src, char * dst)
{
    unsigned char * dst_y_even;
    unsigned char * dst_y_odd;
    unsigned char * dst_u;
    unsigned char * dst_v;
    const unsigned char *src_even;
    const unsigned char *src_odd;
    int i, j;

    src_even = (const unsigned char *)src;
    src_odd = src_even + width * 4;

    dst_y_even = (unsigned char *)dst;
    dst_y_odd = dst_y_even + width;
    dst_u = dst_y_even + width * height;
    dst_v = dst_u + ((width * height) >> 2);

    for ( i = 0; i < height / 2; ++i )
    {
        for ( j = 0; j < width / 2; ++j )
        {
            short r, g, b;
            b = *src_even++;
            g = *src_even++;
            r = *src_even++;
            ++src_even;
            *dst_y_even++ = (( r * 66 + g * 129 + b * 25 + 128 ) >> 8 ) + 16;

            *dst_u++ = (( r * -38 - g * 74 + b * 112 + 128 ) >> 8 ) + 128;
            *dst_v++ = (( r * 112 - g * 94 - b * 18 + 128 ) >> 8 ) + 128;

            b = *src_even++;
            g = *src_even++;
            r = *src_even++;
            ++src_even;
            *dst_y_even++ = (( r * 66 + g * 129 + b * 25 + 128 ) >> 8 ) + 16;

            b = *src_odd++;
            g = *src_odd++;
            r = *src_odd++;
            ++src_odd;
            *dst_y_odd++ = (( r * 66 + g * 129 + b * 25 + 128 ) >> 8 ) + 16;

            b = *src_odd++;
            g = *src_odd++;
            r = *src_odd++;
            ++src_odd;
            *dst_y_odd++ = (( r * 66 + g * 129 + b * 25 + 128 ) >> 8 ) + 16;
        }

        dst_y_even += width;
        dst_y_odd += width;
        src_even += width * 4;
        src_odd += width * 4;
    }

    return 0;
}
}


#endif // ANTUTILS_H

