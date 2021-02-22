#ifdef __cplusplus
extern "C"
{
#endif

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define WASM_EXPORT __attribute__((visibility("default")))

    uint8_t *__flipBuffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        int image_size = img_width * img_height * 4;
        uint8_t *buffer_write = (uint8_t *)(image_buffer + image_size);
        uint8_t *tranposed_image = buffer_write;

        int offset_row = 4 * img_width;

        for (int i = 0; i < img_width; i++)
        {
            int index = 4 * i;
            for (int j = 0; j < img_height; j++)
            {
                buffer_write[0] = image_buffer[index];
                buffer_write[1] = image_buffer[index + 1];
                buffer_write[2] = image_buffer[index + 2];
                buffer_write[3] = 255;
                buffer_write += 4;

                index += offset_row;
            }
        }
        return tranposed_image;
    }

    // -- Buffer - Buffer

    uint8_t *__buffer_raw_buffer_to_gray_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        int offset_buffer_write = img_width * img_height * 4;

        uint8_t *buffer_write = (uint8_t *)(image_buffer + offset_buffer_write);

        for (int i = 0; i < img_height; i++)
        {
            for (int j = 0; j < img_width; j++)
            {
                uint8_t val = 0.3 * image_buffer[0] + 0.59 * image_buffer[1] + 0.11 * image_buffer[2];
                buffer_write[0] = val;
                buffer_write[1] = val;
                buffer_write[2] = val;
                buffer_write[3] = 255;
                image_buffer += 4;
                buffer_write += 4;
            }
        }

        return image_buffer;
    }

    // -- Buffer -> Image & Image -> Buffer

    uint8_t *__buffer_raw_buffer_to_gray_image(uint8_t *image_buffer, int img_width, int img_height)
    {
        int image_size = img_width * img_height;
        int offset_buffer_write = image_size * 4;

        uint8_t *buffer_write = (uint8_t *)(image_buffer + offset_buffer_write);

        for (int i = 0; i < img_height; i++)
        {
            for (int j = 0; j < img_width; j++)
            {
                uint8_t val = 0.3 * image_buffer[0] + 0.59 * image_buffer[1] + 0.11 * image_buffer[2];
                *buffer_write = val;

                image_buffer += 4;
                buffer_write++;
            }
        }

        return image_buffer;
    }

    uint8_t *__one_channel_image_to_buffer(uint8_t *image, int img_width, int img_height)
    {
        int image_size = img_width * img_height;

        uint8_t *buffer_write = (uint8_t *)(image + image_size);

        for (int i = 0; i < img_height; i++)
        {
            for (int j = 0; j < img_width; j++)
            {
                uint8_t val = *image;
                buffer_write[0] = val;
                buffer_write[1] = val;
                buffer_write[2] = val;
                buffer_write[3] = 255;

                image++;
                buffer_write += 4;
            }
        }

        return image;
    }

    // -- Special

    void hueToRGB(int hue, int *r_ret, int *g_ret, int *b_ret)
    {
        // Must be fractions of 1
        float s = 1.0;
        float l = 0.5;
        float c, x, m, r, g, b;
        c = (1 - abs(2 * l - 1)) * s;
        // x = c * (1 - abs((hue / 60) % 2 - 1));
        x = c * (1 - abs((float)(hue % 120) / 60 - 1));
        m = l - c / 2.0;
        if (0 <= hue && hue < 60)
        {
            r = c;
            g = x;
            b = 0;
        }
        else if (60 <= hue && hue < 120)
        {
            r = x;
            g = c;
            b = 0;
        }
        else if (120 <= hue && hue < 180)
        {
            r = 0;
            g = c;
            b = x;
        }
        else if (180 <= hue && hue < 240)
        {
            r = 0;
            g = x;
            b = c;
        }
        else if (240 <= hue && hue < 300)
        {
            r = x;
            g = 0;
            b = c;
        }
        else if (300 <= hue && hue < 360)
        {
            r = c;
            g = 0;
            b = x;
        }
        *r_ret = round((r + m) * 255);
        *g_ret = round((g + m) * 255);
        *b_ret = round((b + m) * 255);
    }

    uint8_t *__min_energy_to_buffer(uint16_t *min_energy_image, int img_width, int img_height)
    {
        int image_size = img_width * img_height;

        uint8_t *buffer_write = (uint8_t *)(min_energy_image + image_size);

        float max_value = 0;
        int tmp;

        int index = 0;
        for (int i = 0; i < img_height; i++)
        {
            for (int j = 0; j < img_width; j++)
            {
                tmp = min_energy_image[index];
                if (tmp > max_value)
                {
                    max_value = tmp;
                }
                index++;
            }
        }
        float norm_val = max_value / 255;

        for (int i = 0; i < img_height; i++)
        {
            for (int j = 0; j < img_width; j++)
            {
                float val = *min_energy_image;
                int hue = round(255 - (val / norm_val));
                int r, g, b;

                hueToRGB(hue, &r, &g, &b);

                // printf("%d %d %d %d\n", hue, r, g, b);

                buffer_write[0] = r;
                buffer_write[1] = g;
                buffer_write[2] = b;
                buffer_write[3] = 255;

                min_energy_image++;
                buffer_write += 4;
            }
        }

        return (uint8_t *)min_energy_image;
    }

    // -- Process

    uint8_t __get_energy_from_pixel(uint8_t *gray_image, int base_width, int actual_width, int img_height, int pixel_row, int pixel_col)
    {
        int pixel_index = pixel_row * base_width + pixel_col;
        uint8_t *pixel_pointer = gray_image + pixel_index;

        if ((pixel_row < img_height - 1) && (pixel_col < actual_width - 1))
        {
            return abs(*pixel_pointer - *(pixel_pointer + 1)) + abs(*pixel_pointer - *(pixel_pointer + base_width));
        }

        if (pixel_row < img_height - 1) // Si on est pas à la dernière ligne
        {
            return abs(*pixel_pointer - *(pixel_pointer + base_width));
        }
        else if (pixel_col < actual_width - 1) // Si on est pas à la dernière colonne
        {
            return abs(*pixel_pointer - *(pixel_pointer + 1));
        }
        else // Cas du dernier pixel !
        {
            return 0;
        }
    }

    uint8_t *__gray_image_to_energy_image(uint8_t *gray_image, int img_width, int img_height)
    {
        int image_size = img_width * img_height;

        uint8_t *buffer_write = (uint8_t *)(gray_image + image_size);

        // Iterate over each row before last row
        for (int i = 0; i < img_height - 1; i++)
        {
            for (int j = 0; j < img_width - 1; j++)
            {
                *buffer_write = min(255, abs(*gray_image - *(gray_image + 1)) + abs(*gray_image - *(gray_image + img_width)));

                gray_image++;
                buffer_write++;
            }

            // Finish last row's element which only use vertical energy
            *buffer_write = abs(*gray_image - *(gray_image + img_width));
            gray_image++;
            buffer_write++;
        }

        // Last row only use horizontal energy
        for (int j = 0; j < img_width - 1; j++)
        {
            *buffer_write = abs(*gray_image - *(gray_image + 1));
            gray_image++;
            buffer_write++;
        }

        // Bottom Right Corner
        *buffer_write = 0;
        gray_image++;
        buffer_write++;

        return gray_image;
    }

    uint16_t *__energy_to_min_energy(uint8_t *energy_image, int base_width, int actual_width, int img_height)
    {
        int image_size = base_width * img_height;

        int empty_pixels_per_row = base_width - actual_width;

        uint16_t *buffer_write = (uint16_t *)(energy_image + image_size);
        uint16_t *one_row_late = buffer_write; // One row late wrt to buffer write

        // Copy first row
        for (int j = 0; j < base_width; j++)
        {
            *buffer_write = *energy_image;
            energy_image++;
            buffer_write++;
        }

        for (int i = 1; i < img_height; i++)
        {
            for (int j = 0; j < actual_width; j++)
            {
                uint16_t lowest_energy_val = *one_row_late;
                if (j > 0)
                {
                    lowest_energy_val = min(lowest_energy_val, *(one_row_late - 1));
                }
                if (j < actual_width - 1)
                {
                    lowest_energy_val = min(lowest_energy_val, *(one_row_late + 1));
                }

                *buffer_write = *energy_image + lowest_energy_val; // Energy of case + minimum cost to go there

                energy_image++;
                buffer_write++;
                one_row_late++;
            }

            energy_image += empty_pixels_per_row;
            buffer_write += empty_pixels_per_row;
            one_row_late += empty_pixels_per_row;
        }
        return (uint16_t *)energy_image;
    }

    uint16_t *__find_min_energy_path(uint16_t *min_energy_image, int base_width, int actual_width, int img_height)
    {
        int image_size = base_width * img_height;

        // On se place au premier spot pour ecrire
        uint16_t *buffer_write = (uint16_t *)(min_energy_image + image_size);

        // On commence au début de la dernière ligne
        uint16_t *buffer_read = (uint16_t *)(min_energy_image + image_size - base_width);

        // -- Get starting point

        uint16_t min_index = 0;
        int min_value = 1000000;
        int tmp;

        for (int j = 0; j < actual_width; j++)
        {
            tmp = buffer_read[j];
            if (tmp < min_value)
            {
                min_value = tmp;
                min_index = j;
            }
        }
        buffer_write[img_height - 1] = min_index;

        // -- Going Up

        uint16_t last_index = min_index;

        for (int i = img_height - 2; i > 0; i--)
        {
            uint16_t min_index = last_index;
            uint16_t min_value = buffer_read[last_index];

            buffer_read -= base_width; // On monte d'1 ligne

            if (last_index > 0)
            {
                tmp = *(buffer_read + last_index - 1);
                if (tmp < min_value)
                {
                    min_value = tmp;
                    min_index = last_index - 1;
                }
            }
            if (last_index < actual_width - 1)
            {
                tmp = *(buffer_read + last_index + 1);
                if (tmp < min_value)
                {
                    min_value = tmp;
                    min_index = last_index + 1;
                }
            }
            buffer_write[i] = min_index;

            last_index = min_index;
        }

        return buffer_write;
    }

    void __color_min_energy_path(uint16_t *min_energy_path, uint8_t *image_buffer, int img_width, int img_height)
    {
        int image_size = img_width * img_height;

        uint8_t *buffer_write = image_buffer;

        for (int i = 0; i < img_height; i++)
        {
            uint16_t delete_index = *min_energy_path;
            int offset = 4 * delete_index;
            buffer_write[offset] = 255;
            buffer_write[offset + 1] = 0;
            buffer_write[offset + 2] = 0;
            buffer_write[offset + 3] = 255;

            buffer_write += 4 * img_width;
            min_energy_path++;
        }
    }

    uint8_t *__delete_path_from_image_buffer_n_create_new_buffer(uint32_t *image_buffer, uint16_t *min_energy_path, int base_width, int width_before_delete, int img_height)
    {
        uint32_t *buffer_write = (uint32_t *)(min_energy_path + img_height);
        int new_width = width_before_delete - 1;

        for (int i = 0; i < img_height; i++)
        {
            uint16_t deleted_index = *min_energy_path;

            memcpy(buffer_write, image_buffer, sizeof(uint32_t) * deleted_index);

            int rest_to_copy = width_before_delete - deleted_index - 1;

            memcpy(buffer_write + deleted_index, image_buffer + (deleted_index + 1), sizeof(uint32_t) * rest_to_copy);

            image_buffer += base_width;
            buffer_write += new_width;

            min_energy_path++;
        }

        return (uint8_t *)min_energy_path;
    }

    void __delete_path_from_all(uint32_t *image_buffer, uint8_t *gray_image,
                                uint8_t *energy_image, uint16_t *min_energy_image,
                                uint16_t *min_energy_path, int base_width, int width_before_delete, int img_height)
    {

        int image_size = base_width * img_height;

        for (int i = 0; i < img_height; i++)
        {
            uint16_t deleted_index = *min_energy_path;
            int rest_to_copy = width_before_delete - deleted_index - 1;
            // std::cout << width_before_delete << " " << deleted_index << std::endl;

            // Shift image buffer
            memmove(image_buffer + deleted_index, image_buffer + deleted_index + 1, sizeof(uint32_t) * rest_to_copy);

            // Shift gray image
            memmove(gray_image + deleted_index, gray_image + deleted_index + 1, sizeof(uint8_t) * rest_to_copy);

            // Shift energy image
            memmove(energy_image + deleted_index, energy_image + deleted_index + 1, sizeof(uint8_t) * rest_to_copy);

            // -- Increment ce qu'il faut !
            image_buffer += base_width;
            gray_image += base_width;
            energy_image += base_width;

            min_energy_path++;
        }

        // - Update energies

        energy_image -= image_size;
        min_energy_path -= image_size;

        int new_width = width_before_delete - 1;

        for (int i = 0; i < img_height; i++)
        {
            uint16_t deleted_index = *min_energy_path;

            // Update actual case
            energy_image[deleted_index] = __get_energy_from_pixel(gray_image, base_width, new_width, img_height,
                                                                  i, deleted_index);

            // Update upper case
            if (i > 0)
            {
                energy_image[deleted_index - base_width] = __get_energy_from_pixel(gray_image, base_width, new_width, img_height,
                                                                                   i - 1, deleted_index);
            }

            // Update left case
            if (deleted_index > 0)
            {
                energy_image[deleted_index - 1] = __get_energy_from_pixel(gray_image, base_width, new_width, img_height,
                                                                          i, deleted_index - 1);
            }

            // - Increment Pointers
            energy_image += base_width;
            min_energy_path++;
        }
    }

    //  --- Callable

    WASM_EXPORT
    uint8_t *convert_raw_buffer_to_gray_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        return __buffer_raw_buffer_to_gray_buffer(image_buffer, img_width, img_height);
    }

    WASM_EXPORT
    uint8_t *convert_raw_buffer_to_energy_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *gray_image = __buffer_raw_buffer_to_gray_image(image_buffer, img_width, img_height);
        uint8_t *energy_image = __gray_image_to_energy_image(gray_image, img_width, img_height);
        return __one_channel_image_to_buffer(energy_image, img_width, img_height);
    }

    WASM_EXPORT
    uint8_t *convert_raw_buffer_to_min_energy_buffer_col(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *gray_image = __buffer_raw_buffer_to_gray_image(image_buffer, img_width, img_height);
        uint8_t *energy_image = __gray_image_to_energy_image(gray_image, img_width, img_height);
        uint16_t *min_energy_image = __energy_to_min_energy(energy_image, img_width, img_width, img_height);

        return __min_energy_to_buffer(min_energy_image, img_width, img_height);
    }

    WASM_EXPORT
    uint8_t *convert_raw_buffer_to_min_energy_buffer_row(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *transposed_image = __flipBuffer(image_buffer, img_width, img_height);

        uint8_t *im_res_transposed = convert_raw_buffer_to_min_energy_buffer_col(transposed_image, img_height, img_width);

        uint8_t *im_res = __flipBuffer(im_res_transposed, img_height, img_width);

        return im_res;
    }

    WASM_EXPORT
    uint8_t *color_one_col_from_raw_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *gray_image = __buffer_raw_buffer_to_gray_image(image_buffer, img_width, img_height);
        uint8_t *energy_image = __gray_image_to_energy_image(gray_image, img_width, img_height);
        uint16_t *min_energy_image = __energy_to_min_energy(energy_image, img_width, img_width, img_height);
        uint16_t *min_energy_path = __find_min_energy_path(min_energy_image, img_width, img_width, img_height);

        __color_min_energy_path(min_energy_path, image_buffer, img_width, img_height);

        return image_buffer;
    }

    WASM_EXPORT
    uint8_t *delete_one_col_from_raw_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *gray_image = __buffer_raw_buffer_to_gray_image(image_buffer, img_width, img_height);
        uint8_t *energy_image = __gray_image_to_energy_image(gray_image, img_width, img_height);
        uint16_t *min_energy_image = __energy_to_min_energy(energy_image, img_width, img_width, img_height);
        uint16_t *min_energy_path = __find_min_energy_path(min_energy_image, img_width, img_width, img_height);

        uint8_t *res_image = __delete_path_from_image_buffer_n_create_new_buffer((uint32_t *)image_buffer, min_energy_path, img_width, img_width, img_height);

        return res_image;
    }

    WASM_EXPORT
    uint8_t *color_one_row_from_raw_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *transposed_image = __flipBuffer(image_buffer, img_width, img_height);

        color_one_col_from_raw_buffer(transposed_image, img_height, img_width);

        uint8_t *im_res = __flipBuffer(transposed_image, img_height, img_width);
        return im_res;
    }

    WASM_EXPORT
    uint8_t *delete_one_row_from_raw_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *transposed_image = __flipBuffer(image_buffer, img_width, img_height);

        uint8_t *im_res_transposed = delete_one_col_from_raw_buffer(transposed_image, img_height, img_width);

        uint8_t *im_res = __flipBuffer(im_res_transposed, img_height - 1, img_width);
        return im_res;
    }

    WASM_EXPORT
    uint8_t *delete_multi_col_from_raw_buffer(uint8_t *image_buffer, int base_width, int img_height, int col_to_delete)
    {
        if (col_to_delete < 1)
        {
            return image_buffer;
        }

        if (col_to_delete == 1)
        {
            return delete_one_col_from_raw_buffer(image_buffer, base_width, img_height);
        }

        // -- Compute first round !
        uint8_t *gray_image = __buffer_raw_buffer_to_gray_image(image_buffer, base_width, img_height);
        uint8_t *energy_image = __gray_image_to_energy_image(gray_image, base_width, img_height);
        uint16_t *min_energy_image = __energy_to_min_energy(energy_image, base_width, base_width, img_height);
        uint16_t *min_energy_path = __find_min_energy_path(min_energy_image, base_width, base_width, img_height);

        int actual_width = base_width;

        uint32_t *image_buffer_32_bits = (uint32_t *)image_buffer;

        for (int i = 0; i < col_to_delete - 1; i++)
        {
            __delete_path_from_all(image_buffer_32_bits, gray_image, energy_image, min_energy_image, min_energy_path,
                                   base_width, actual_width, img_height);
            actual_width--;

            __energy_to_min_energy(energy_image, base_width, actual_width, img_height);
            __find_min_energy_path(min_energy_image, base_width, actual_width, img_height);
        }

        uint8_t *res_buffer = __delete_path_from_image_buffer_n_create_new_buffer(image_buffer_32_bits, min_energy_path, base_width, actual_width, img_height);

        return res_buffer;
    }

    WASM_EXPORT
    uint8_t *delete_multi_row_from_raw_buffer(uint8_t *image_buffer, int base_width, int img_height, int row_to_delete)
    {
        uint8_t *transposed_image = __flipBuffer(image_buffer, base_width, img_height);

        uint8_t *im_res_transposed = delete_multi_col_from_raw_buffer(transposed_image, img_height, base_width, row_to_delete);

        uint8_t *im_res = __flipBuffer(im_res_transposed, img_height - row_to_delete, base_width);
        return im_res;
    }

    // --- Test

    uint8_t *overwrite_buffer_with_gray_buffer(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *buffer_write = image_buffer;

        for (int i = 0; i < img_height; i++)
        {
            for (int j = 0; j < img_width; j++)
            {
                uint8_t val = 0.3 * buffer_write[0] + 0.59 * buffer_write[1] + 0.11 * buffer_write[2];
                buffer_write[0] = val;
                buffer_write[1] = val;
                buffer_write[2] = val;
                buffer_write[3] = 255;
                buffer_write += 4;
            }
        }

        return image_buffer;
    }

    WASM_EXPORT
    uint8_t *flip_image(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *transposed_image = __flipBuffer(image_buffer, img_width, img_height);
        return transposed_image;
    }

    WASM_EXPORT
    uint8_t *double_flip_image(uint8_t *image_buffer, int img_width, int img_height)
    {
        uint8_t *transposed_image = __flipBuffer(image_buffer, img_width, img_height);
        uint8_t *double_transposed_image = __flipBuffer(transposed_image, img_height, img_width);
        return double_transposed_image;
    }

#ifdef __cplusplus
}
#endif
