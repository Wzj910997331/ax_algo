#include <unistd.h>
#include <sys/stat.h>

#include <ax_sys_api.h>
#include <ax_ivps_api.h>
#include <ax_engine_api.h>

#include <opencv2/opencv.hpp>

#include "ax_algorithm_sdk.h"
#include "cmdline.hpp"
#include "string_utils.hpp"

#define ALIGN_UP(x, align) ((((x) + ((align) - 1)) / (align)) * (align))

int inference(ax_algorithm_handle_t handle, cv::Mat &image, float feature[512])
{
    ax_image_t image_rgb;
    ax_create_image(image.cols, image.rows, image.cols, ax_color_space_rgb, &image_rgb);
    memcpy(image_rgb.pVir, image.data, image_rgb.nSize);

    ax_result_t result;
    memset(&result, 0, sizeof(ax_result_t));
    int ret = ax_algorithm_get_face_feature(handle, &image_rgb, &result, -1, feature);
    if (ret != 0)
    {
        printf("ax_algorithm_get_face_feature failed ret:%d\n" , ret);
        return ret;
    }
    ax_release_image(&image_rgb);

    for (size_t i = 0; i < result.n_objects; i++)
    {
        auto &box = result.objects[i];
        printf("quality: %0.2f\n", box.face_info.quality);
        cv::rectangle(image, cv::Rect(box.bbox.x, box.bbox.y, box.bbox.w, box.bbox.h), cv::Scalar(255, 0, 0), 2);

        for (size_t j = 0; j < AX_ALGORITHM_FACE_POINT_LEN; j++)
        {
            cv::circle(image, cv::Point(box.face_info.points[j].x, box.face_info.points[j].y), 2, cv::Scalar(0, 0, 255), 2);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    cmdline::parser parser;
    parser.add<std::string>("model", 'm', "model path", true);
    parser.add<std::string>("a", 'a', "image path", true);
    parser.add<std::string>("b", 'b', "image path", true);
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
    npu_attr.eHardMode = AX_ENGINE_VIRTUAL_NPU_STD;
    ret = AX_ENGINE_Init(&npu_attr);
    if (0 != ret)
    {
        printf("AX_ENGINE_Init failed\n");
        return -1;
    }

    std::string model_path = parser.get<std::string>("model");
    std::string image_path_a = parser.get<std::string>("a");
    std::string image_path_b = parser.get<std::string>("b");
    std::string output_path = parser.get<std::string>("output");

    ax_algorithm_handle_t handle;
    ax_algorithm_init_t init_info;
    init_info.model_type = ax_model_type_e::ax_model_type_face_recognition;
    sprintf(init_info.model_file, model_path.c_str());
    init_info.param = ax_algorithm_get_default_param();

    if (ax_algorithm_init(&init_info, &handle) != 0)
    {
        return -1;
    }

    // output_path exists
    if (access(output_path.c_str(), 0) != 0)
    {
        mkdir(output_path.c_str(), 0755);
    }

    cv::Mat image_a = cv::imread(image_path_a);
    cv::Mat image_b = cv::imread(image_path_b);
    if (image_a.data && image_b.data)
    {
        cv::resize(image_a, image_a, cv::Size(ALIGN_UP(image_a.cols, 128), ALIGN_UP(image_a.rows, 128)));
        cv::cvtColor(image_a, image_a, cv::COLOR_BGR2RGB);

        cv::resize(image_b, image_b, cv::Size(ALIGN_UP(image_b.cols, 128), ALIGN_UP(image_b.rows, 128)));
        cv::cvtColor(image_b, image_b, cv::COLOR_BGR2RGB);

        float feature_a[512] = {0};
        float feature_b[512] = {0};
        inference(handle, image_a, feature_a);
        inference(handle, image_b, feature_b);
        cv::cvtColor(image_a, image_a, cv::COLOR_RGB2BGR);
        cv::cvtColor(image_b, image_b, cv::COLOR_RGB2BGR);

        float score = ax_algorithm_face_compare(feature_a, feature_b);

        printf("score: %f\n", score);

        auto out_path_a = string_utils::join(output_path, string_utils::basename(image_path_a));
        printf("out_path_a: %s\n", out_path_a.c_str());
        cv::imwrite(out_path_a, image_a);

        auto out_path_b = string_utils::join(output_path, string_utils::basename(image_path_b));
        printf("out_path_b: %s\n", out_path_b.c_str());
        cv::imwrite(out_path_b, image_b);
    }

    ax_algorithm_deinit(handle);
    AX_ENGINE_Deinit();
    AX_IVPS_Deinit();
    AX_SYS_Deinit();

    return 0;
}
