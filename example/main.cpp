#include <unistd.h>
#include <sys/stat.h>

#include <ax_sys_api.h>
#include <ax_ivps_api.h>
#include <ax_engine_api.h>

#include <opencv2/opencv.hpp>

#include "ax_algorithm_sdk.h"
#include "cmdline.hpp"
#include "string_utils.hpp"
#include "putTextPlate.h"

#define ALIGN_UP(x, align) ((((x) + ((align) - 1)) / (align)) * (align))

int inference(ax_algorithm_handle_t handle, cv::Mat &image)
{
    ax_image_t image_rgb;
    ax_create_image(image.cols, image.rows, image.cols, ax_color_space_rgb, &image_rgb);
    memcpy(image_rgb.pVir, image.data, image_rgb.nSize);

    ax_result_t result;
    memset(&result, 0, sizeof(ax_result_t));
    ax_algorithm_inference(handle, &image_rgb, &result);
    ax_release_image(&image_rgb);

    for (int i = 0; i < result.n_objects; i++)
    {
        auto &box = result.objects[i];
        cv::rectangle(image, cv::Rect(box.bbox.x, box.bbox.y, box.bbox.w, box.bbox.h), cv::Scalar(255, 0, 0), 2);
        switch (result.model_type)
        {
        case ax_model_type_person:
        {
            if (box.person_info.status == 3)
            {
                continue;
            }

            cv::putText(image, std::to_string(box.person_info.status) + " " + std::to_string(box.track_id), cv::Point(box.bbox.x, box.bbox.y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
            printf("status: %d, track_id: %d\n", box.person_info.status, box.track_id);
        }
        break;
        case ax_model_type_face_recognition:
        {
            cv::putText(image, std::to_string(box.track_id), cv::Point(box.bbox.x, box.bbox.y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
            for (size_t j = 0; j < AX_ALGORITHM_FACE_POINT_LEN; j++)
            {
                cv::circle(image, cv::Point(box.face_info.points[j].x, box.face_info.points[j].y), 2, cv::Scalar(0, 0, 255), 1);
            }
            printf("track_id: %d quality: %0.2f \n", box.track_id, box.face_info.quality);
        }
        break;
        case ax_model_type_lpr:
        {
            char license[32] = {0};
            ax_algorithm_get_plate_str(box.vehicle_info.plate_id, box.vehicle_info.len_plate_id, license);
            printf("license: %s cartype: %d\n", license, box.vehicle_info.cartype);
            ax_point_t org = {box.bbox.x + box.bbox.w / 2, box.bbox.y + box.bbox.h / 2};
            unsigned char color[3] = {0, 0, 255};
            ax_image_t ax_image_rgb = {0};
            ax_image_rgb.nWidth = image.cols;
            ax_image_rgb.nHeight = image.rows;
            ax_image_rgb.pVir = image.data;
            putTextPlateID(&ax_image_rgb, box.vehicle_info.plate_id, box.vehicle_info.len_plate_id, &org, color, 1);
        }
        break;
        case ax_model_type_fire_smoke:
        {
            cv::putText(image, std::to_string(box.label) + " " + std::to_string(box.track_id), cv::Point(box.bbox.x, box.bbox.y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
            printf("status: %d, track_id: %d label: %d score: %0.2f\n", box.label, box.track_id, box.label, box.score);
        }
        break;
        default:
            break;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    cmdline::parser parser;
    parser.add<std::string>("model", 'm', "model path", true);
    parser.add<int>("model_type", 't', "model type 0:person 1:lpr 2:face 3:fire smoke", true);
    parser.add<std::string>("image", 'i', "image path", true);
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
    std::string image_path = parser.get<std::string>("image");
    std::string output_path = parser.get<std::string>("output");

    ax_algorithm_handle_t handle;
    ax_algorithm_init_t init_info;
    init_info.model_type = (ax_model_type_e)parser.get<int>("model_type");
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

    cv::Mat image = cv::imread(image_path);
    if (image.data)
    {
        // cv::resize(image, image, cv::Size(1920, 1080));
        cv::resize(image, image, cv::Size(ALIGN_UP(image.cols, 128), ALIGN_UP(image.rows, 128)));
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        inference(handle, image);
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        auto out_path = string_utils::join(output_path, string_utils::basename(image_path));
        printf("out_path: %s\n", out_path.c_str());
        cv::imwrite(out_path, image);
    }
    else
    {

        std::vector<cv::String> image_list;
        cv::glob(image_path + "/*.*", image_list);

        for (auto image_path_ : image_list)
        {
            cv::Mat image = cv::imread(image_path_);
            if (image.data)
            {
                cv::resize(image, image, cv::Size(ALIGN_UP(image.cols, 128), ALIGN_UP(image.rows, 128)));
                cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                inference(handle, image);
                cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                auto out_path = string_utils::join(output_path, string_utils::basename(image_path_));
                printf("out_path: %s\n", out_path.c_str());
                cv::imwrite(out_path, image);
            }
        }
    }
    ax_algorithm_deinit(handle);
    AX_ENGINE_Deinit();
    AX_IVPS_Deinit();
    AX_SYS_Deinit();

    return 0;
}
