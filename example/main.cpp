#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

#include <ax_sys_api.h>
#include <ax_ivps_api.h>
#include <ax_engine_api.h>

#include <opencv2/opencv.hpp>

#include "ax_algorithm_sdk.h"
#include "cmdline.hpp"
#include "string_utils.hpp"

static bool read_file(const std::string &path, std::vector<char> &data)
{
    std::fstream fs(path, std::ios::in | std::ios::binary);

    if (!fs.is_open())
    {
        return false;
    }

    fs.seekg(std::ios::end);
    auto fs_end = fs.tellg();
    fs.seekg(std::ios::beg);
    auto fs_beg = fs.tellg();

    auto file_size = static_cast<size_t>(fs_end - fs_beg);
    auto vector_size = data.size();

    data.reserve(vector_size + file_size);
    data.insert(data.end(), std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());

    fs.close();

    return true;
}

int inference(std::vector<char> &image, int width, int height, int stride, cv::Mat &image_bgr, ax_algorithm_handle_t handle)
{
    ax_image_t image_nv12;
    ax_create_image(width, height, stride, ax_color_space_nv12, &image_nv12);
    memcpy(image_nv12.pVir, image.data(), image_nv12.nSize);

    ax_result_t result;
    ax_algorithm_inference(handle, &image_nv12, &result);
    printf("result size = %d ddddd\n", result.n_objects);
    for (int i = 0; i < result.n_objects; i++) {
        printf("result xywh = %f %f %f %f\n", result.objects[i].bbox.x, result.objects[i].bbox.y,
        result.objects[i].bbox.w, result.objects[i].bbox.h);
    }
    ax_release_image(&image_nv12);

    cv::Mat image_cv_nv12(height * 3 / 2, width, CV_8UC1, image.data(), stride);
    cv::cvtColor(image_cv_nv12, image_bgr, cv::COLOR_YUV2BGR_NV12);

    for (int i = 0; i < result.n_objects; i++)
    {
        auto &box = result.objects[i];
        // if (box.status == 3)
        // {
        //     continue;
        // }

        cv::rectangle(image_bgr, cv::Rect(box.bbox.x, box.bbox.y, box.bbox.w, box.bbox.h), cv::Scalar(255, 0, 0), 2);

        cv::putText(image_bgr, std::to_string(box.status) + " " + std::to_string(box.track_id), cv::Point(box.bbox.x, box.bbox.y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
        printf("status: %d, track_id: %d\n", box.status, box.track_id);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    printf("dddddddddddddd\n");
    cmdline::parser parser;
    parser.add<std::string>("model", 'm', "model path", true);
    parser.add<std::string>("image", 'i', "image path", true);
    parser.add<int>("width", 'w', "image width", true);
    parser.add<int>("height", 'h', "image height", true);
    parser.add<int>("stride", 's', "image stride", true);
    parser.add<std::string>("output", 'o', "output path", false, "plate_result");
    parser.parse_check(argc, argv);

    int ret = AX_SYS_Init();
    if (0 != ret)
    {
        printf("AX_SYS_Init failed\n");
        return -1;
    }
    ret = AX_IVPS_Init();
    if (0 != ret)
    {
        printf("AX_IVPS_Init failed\n");
        return -1;
    }
    AX_ENGINE_NPU_ATTR_T npu_attr;
    memset(&npu_attr, 0, sizeof(npu_attr));
    npu_attr.eHardMode = AX_ENGINE_VIRTUAL_NPU_DISABLE;
    ret = AX_ENGINE_Init(&npu_attr);
    if (0 != ret)
    {
        printf("AX_ENGINE_Init failed\n");
        return -1;
    }

    std::string model_path = parser.get<std::string>("model");
    std::string image_path = parser.get<std::string>("image");
    int width = parser.get<int>("width");
    int height = parser.get<int>("height");
    int stride = parser.get<int>("stride");
    std::string output_path = parser.get<std::string>("output");

    ax_algorithm_handle_t handle;
    ax_algorithm_init_t init_info;
    sprintf(init_info.model_file, model_path.c_str());

    if (ax_algorithm_init(&init_info, &handle) != 0)
    {
        return -1;
    }

    // output_path exists
    if (access(output_path.c_str(), 0) != 0)
    {
        mkdir(output_path.c_str(), 0755);
    }

    std::vector<char> image;
    cv::Mat image_bgr;
    if (!read_file(image_path, image))
    {
        return -1;
    }
    printf("image size: %ld wh: %dx%d stride: %d\n", image.size(), width, height, stride);
    inference(image, width, height, stride, image_bgr, handle);

    if (image_bgr.data)
    {
        auto out_path = string_utils::join(output_path, string_utils::basename(image_path)) + ".jpg";
        printf("out_path: %s\n", out_path.c_str());
        cv::imwrite(out_path, image_bgr);
    }

    ax_algorithm_deinit(handle);
    AX_ENGINE_Deinit();
    AX_IVPS_Deinit();
    AX_SYS_Deinit();
    return 0;
}
