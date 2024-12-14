#include <unistd.h>
#include <sys/stat.h>

#include <ax_sys_api.h>
#include <ax_ivps_api.h>
#include <ax_engine_api.h>
#include <getopt.h>
#include <signal.h>
#include <opencv2/opencv.hpp>

#include "ax_algorithm_sdk.h"
#include "cmdline.hpp"
#include "string_utils.hpp"
#include "putTextPlate.h"

#define ALIGN_UP(x, align) ((((x) + ((align) - 1)) / (align)) * (align))

int inference(ax_algorithm_handle_t handle, cv::Mat &image)
{
    // 图片分辨率和格式不变的情况下 这个图片一路视频只需要申请一次就可以了
    ax_image_t image_rgb;
    ax_create_image(image.cols, image.rows, image.cols, ax_color_space_rgb, &image_rgb);
    memcpy(image_rgb.pVir, image.data, image_rgb.nSize);

    ax_result_t result;
    memset(&result, 0, sizeof(ax_result_t));
    ax_algorithm_inference(handle, &image_rgb, &result);
    // 释放也只需要一次
    ax_release_image(&image_rgb);

    for (int i = 0; i < result.n_objects; i++)
    {
        auto &box = result.objects[i];
        switch (result.model_type)
        {
        case ax_model_type_person:
        {
            if (box.person_info.status == 3)
            {
                continue;
            }
            printf("status: %d, track_id: %d\n", box.person_info.status, box.track_id);
        }
        break;
        case ax_model_type_face_recognition:
        {
            printf("track_id: %d %0.2f\n", box.track_id, box.face_info.quality);
        }
        break;
        case ax_model_type_lpr:
        {
            char license[32] = {0};
            ax_algorithm_get_plate_str(box.vehicle_info.plate_id, box.vehicle_info.len_plate_id, license);
            printf("license: %s cartype: %d\n", license, box.vehicle_info.cartype);
        }
        break;
        default:
            break;
        }
    }

    return 0;
}
volatile int gLoopExit = 0;
extern "C" void __sigExit(int iSigNo)
{
    gLoopExit = 1;
    return;
}

int main(int argc, char *argv[])
{

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigExit);
    cmdline::parser parser;
    parser.add<std::string>("model", 'm', "model path", true);
    parser.add<int>("model_type", 't', "model type 0:person 1:lpr", true);
    parser.add<std::string>("image", 'i', "image path", true);
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
    std::string image_path = parser.get<std::string>("image");

    ax_algorithm_handle_t handle;
    ax_algorithm_init_t init_info;
    init_info.model_type = (ax_model_type_e)parser.get<int>("model_type");
    sprintf(init_info.model_file, model_path.c_str());
    init_info.param = ax_algorithm_get_default_param();

    if (ax_algorithm_init(&init_info, &handle) != 0)
    {
        return -1;
    }

    while (gLoopExit == 0)
    {
        cv::Mat image = cv::imread(image_path);
        if (image.data)
        {
            // cv::resize(image, image, cv::Size(1920, 1080));
            cv::resize(image, image, cv::Size(ALIGN_UP(image.cols, 128), ALIGN_UP(image.rows, 128)));
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            inference(handle, image);
        }
        else
        {
            std::vector<std::string> image_list;
            cv::glob(image_path + "/*.*", image_list);
            for (auto image_path_ : image_list)
            {
                if (gLoopExit == 1)
                    break;
                cv::Mat image = cv::imread(image_path_);
                if (image.data)
                {
                    // cv::resize(image, image, cv::Size(1920, 1080));
                    cv::resize(image, image, cv::Size(ALIGN_UP(image.cols, 128), ALIGN_UP(image.rows, 128)));
                    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                    printf("image path: %s\n", image_path_.c_str());
                    inference(handle, image);
                }
            }
        }
    }

    ax_algorithm_deinit(handle);
    AX_ENGINE_Deinit();
    AX_IVPS_Deinit();
    AX_SYS_Deinit();

    return 0;
}
