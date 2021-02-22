#include <iostream>
#include <chrono>

#include <opencv2/opencv.hpp> // Include OpenCV API

#include "exportable.h"

#define NUMBER_TOTAL_OF_BUFFER 10

uint8_t *OpenCVmatToBuffer(cv::Mat &im)
{
    int im_width = im.size().width;
    int im_height = im.size().height;
    int im_channels = im.channels();
    int buffer_size = im_width * im_height * im_channels;

    // std::cout << im_height << " " << im_width << " " << im_channels << std::endl;
    // std::cout << "buffer will have a size of " << buffer_size << std::endl;

    uint8_t *image_buffer = new uint8_t[NUMBER_TOTAL_OF_BUFFER * buffer_size];

    uint8_t *pixelPtr = (uint8_t *)im.data;

    int index = 0;

    for (int i = 0; i < im_height; i++)
    {
        for (int j = 0; j < im_width; j++)
        {
            for (int k = 0; k < im_channels; k++)
            {
                image_buffer[index] = pixelPtr[index];
                index++;
            }
        }
    }
    return image_buffer;
}

cv::Mat bufferToOpenCVmat(uint8_t *image_buffer, int img_width, int img_height, int channels)
{
    cv::Mat image_rgb(cv::Size(img_width, img_height), (channels == 3) ? CV_8UC3 : CV_8UC4,
                      static_cast<void *>(image_buffer), cv::Mat::AUTO_STEP);

    cv::Mat image_bgr;
    cv::cvtColor(image_rgb, image_bgr, cv::COLOR_RGBA2BGR);

    return image_bgr;
}

int main(int argc, char *argv[])
{
    std::string imageName = "winter_750w.jpg";

    bool testing = false;
    int cols_to_delete = 0;
    int rows_to_delete = 0;

    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "-t")
        {
            testing = true;
            std::cout << "Debug mode" << std::endl;
        }
        else if (std::string(argv[i]) == "-c")
        {
            cols_to_delete = atoi(argv[i + 1]);
            std::cout << "Custom Column delete " << cols_to_delete << std::endl;
        }
        else if (std::string(argv[i]) == "-r")
        {
            rows_to_delete = atoi(argv[i + 1]);
            std::cout << "Custom row delete " << rows_to_delete << std::endl;
        }
    }

    cv::Mat im_original = cv::imread("../" + imageName);
    cv::Mat im;
    cv::cvtColor(im_original, im, cv::COLOR_BGR2RGBA);
    int im_width = im.size().width;
    int im_height = im.size().height;
    int im_channels = im.channels();
    cv::imshow("Before", im_original);

    uint8_t *image_buffer = OpenCVmatToBuffer(im);

    if (testing)
    {
        // -- Gray Image
        auto start_gray = std::chrono::system_clock::now();
        uint8_t *gray_buffer = convert_raw_buffer_to_gray_buffer(image_buffer, im_width, im_height);
        cv::Mat im_gray = bufferToOpenCVmat(gray_buffer, im_width, im_height, im_channels);
        auto end_gray = std::chrono::system_clock::now();
        cv::imshow("Gray", im_gray);
        std::cout << "Gray : " << std::chrono::duration_cast<std::chrono::microseconds>(end_gray - start_gray).count() << " µs" << std::endl;

        // -- Energy Image
        auto start_energy = std::chrono::system_clock::now();
        uint8_t *energy_buffer = convert_raw_buffer_to_energy_buffer(image_buffer, im_width, im_height);
        cv::Mat im_energy = bufferToOpenCVmat(energy_buffer, im_width, im_height, im_channels);
        auto end_energy = std::chrono::system_clock::now();
        cv::imshow("Energy", im_energy);
        std::cout << "Energy : " << std::chrono::duration_cast<std::chrono::microseconds>(end_energy - start_energy).count() << " µs" << std::endl;

        // -- Min Energy Images
        auto start_min_energy_col = std::chrono::system_clock::now();
        uint8_t *min_energy_buffer_col = convert_raw_buffer_to_min_energy_buffer_col(image_buffer, im_width, im_height);
        cv::Mat im_min_energy_col = bufferToOpenCVmat(min_energy_buffer_col, im_width, im_height, im_channels);
        auto end_min_energy_col = std::chrono::system_clock::now();
        cv::imshow("Min Energy Col", im_min_energy_col);
        std::cout << "Min Energy Col : " << std::chrono::duration_cast<std::chrono::microseconds>(end_min_energy_col - start_min_energy_col).count() << " µs" << std::endl;

        auto start_min_energy_row = std::chrono::system_clock::now();
        uint8_t *min_energy_buffer_row = convert_raw_buffer_to_min_energy_buffer_row(image_buffer, im_width, im_height);
        cv::Mat im_min_energy_row = bufferToOpenCVmat(min_energy_buffer_row, im_width, im_height, im_channels);
        auto end_min_energy_row = std::chrono::system_clock::now();
        cv::imshow("Min Energy Row", im_min_energy_row);
        std::cout << "Min Energy Row : " << std::chrono::duration_cast<std::chrono::microseconds>(end_min_energy_row - start_min_energy_row).count() << " µs" << std::endl;

        // -- Column deletions
        auto start_colored_col_deletion = std::chrono::system_clock::now();
        uint8_t *deleted_col_colored_buffer = color_one_col_from_raw_buffer(image_buffer, im_width, im_height);
        cv::Mat im_colored_col = bufferToOpenCVmat(deleted_col_colored_buffer, im_width, im_height, im_channels);
        auto end_colored_col_deletion = std::chrono::system_clock::now();
        cv::imshow("Deletion Colored Col", im_colored_col);
        std::cout << "Deletion Colored Col : " << std::chrono::duration_cast<std::chrono::microseconds>(end_colored_col_deletion - start_colored_col_deletion).count() << " µs" << std::endl;
        image_buffer = OpenCVmatToBuffer(im); // Reset

        auto start_one_col_deletion = std::chrono::system_clock::now();
        uint8_t *deleted_one_col_buffer = delete_one_col_from_raw_buffer(image_buffer, im_width, im_height);
        cv::Mat im_deleted_one_col = bufferToOpenCVmat(deleted_one_col_buffer, im_width - 1, im_height, im_channels);
        auto end_one_col_deletion = std::chrono::system_clock::now();
        cv::imshow("Deletion one col", im_deleted_one_col);
        std::cout << "Deletion one col: " << std::chrono::duration_cast<std::chrono::microseconds>(end_one_col_deletion - start_one_col_deletion).count() << " µs" << std::endl;
        image_buffer = OpenCVmatToBuffer(im); // Reset

        // -- Row deletions
        auto start_colored_row_deletion = std::chrono::system_clock::now();
        uint8_t *deleted_row_colored_buffer = color_one_row_from_raw_buffer(image_buffer, im_width, im_height);
        cv::Mat im_colored_row = bufferToOpenCVmat(deleted_row_colored_buffer, im_width, im_height, im_channels);
        auto end_colored_row_deletion = std::chrono::system_clock::now();
        cv::imshow("Deletion Colored Row", im_colored_row);
        std::cout << "Deletion Colored Row: " << std::chrono::duration_cast<std::chrono::microseconds>(end_colored_row_deletion - start_colored_row_deletion).count() << " µs" << std::endl;
        image_buffer = OpenCVmatToBuffer(im); // Reset

        auto start_one_row_deletion = std::chrono::system_clock::now();
        uint8_t *deleted_one_row_buffer = delete_one_row_from_raw_buffer(image_buffer, im_width, im_height);
        cv::Mat im_deleted_one_row = bufferToOpenCVmat(deleted_one_row_buffer, im_width, im_height - 1, im_channels);
        auto end_one_row_deletion = std::chrono::system_clock::now();
        cv::imshow("Deletion one row", im_deleted_one_row);
        std::cout << "Deletion one row: " << std::chrono::duration_cast<std::chrono::microseconds>(end_one_row_deletion - start_one_row_deletion).count() << " µs" << std::endl;
        image_buffer = OpenCVmatToBuffer(im); // Reset
    }

    if (cols_to_delete)
    {
        auto start_process = std::chrono::system_clock::now();
        uint8_t *res_buffer = delete_multi_col_from_raw_buffer(image_buffer, im_width, im_height, cols_to_delete);
        auto end_process = std::chrono::system_clock::now();
        cv::Mat im_res = bufferToOpenCVmat(res_buffer, im_width - cols_to_delete, im_height, im_channels);
        std::cout << "Columns deletion took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end_process - start_process).count() << " ms" << std::endl;
        cv::imshow("After columns deletion", im_res);
    }

    if (rows_to_delete)
    {
        auto start_process = std::chrono::system_clock::now();
        uint8_t *res_buffer = delete_multi_row_from_raw_buffer(image_buffer, im_width, im_height, rows_to_delete);
        auto end_process = std::chrono::system_clock::now();
        cv::Mat im_res = bufferToOpenCVmat(res_buffer, im_width, im_height - rows_to_delete, im_channels);
        std::cout << "Row deletion took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end_process - start_process).count() << " ms" << std::endl;
        cv::imshow("After rows deletion", im_res);
    }

    cv::waitKey(0);
}
