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

std::map<std::string, std::vector<std::string>> g_attr_label_map{
    {"isHuman", {"Uncertain", "Normal", "Abnormal"}},
    {"age", {"Uncertain", "Toddler", "Teenager", "Youth", "Middle-aged", "Elderly"}},
    {"gender", {"Uncertain", "Male", "Female"}},
    {"race", {"Uncertain", "East Asia", "Caucasian", "African", "South Asia"}},
    {"umbrella", {"Uncertain", "No", "Yes"}},
    {"headwear", {"Uncertain", "No", "hat", "Helmet"}},
    {"glasses", {"Uncertain", "No", "Glasses", "Sunglasses"}},
    {"faceMask", {"Uncertain", "No", "Yes"}},
    {"smoke", {"Uncertain", "No", "Yes"}},
    {"carryingItem", {"Uncertain", "No", "Yes"}},
    {"cellphone", {"Uncertain", "No", "Yes"}},
    {"safetyClothing", {"Uncertain", "No", "Yes"}},
    {"upperWear", {"Uncertain", "Long-sleeve", "Short-sleeve"}},
    {"upperColor", {"Uncertain", "Red", "Orange", "Yellow", "Green", "Blue", "Purple", "Pink", "Black", "White", "Gray", "Brown"}},
    {"upperWearFg", {"Uncertain", "T-shirt", "Sleeveless Top", "Shirt", "Suit", "Sweater", "Jacket", "Down Jacket", "Trench Coat", "Coat"}},
    {"upperWearTexture", {"Uncertain", "Solid Color", "Pattern", "Small Floral", "Stripes or Plaid"}},
    {"bag", {"Uncertain", "No", "Crossbody Bag", "Backpack"}},
    {"safetyRope", {"Uncertain", "No", "Yes"}},
    {"upperCut", {"Uncertain", "No", "Yes"}},
    {"lowerWear", {"Uncertain", "Long Pants", "Shorts", "Long Dress", "Short Skirt"}},
    {"lowerColor", {"Uncertain", "Red", "Orange", "Yellow", "Green", "Blue", "Purple", "Pink", "Black", "White", "Gray", "Brown"}},
    {"vehicle", {"Uncertain", "No", "Motorcycle", "Bicycle", "Tricycle"}},
    {"lowerCut", {"Uncertain", "No", "Yes"}},
    {"occlusion", {"Uncertain", "No", "Mild Occlusion", "Heavy Occlusion"}},
    {"orientation", {"Uncertain", "Front", "Back", "Right Side", "Left Side"}},
};

std::string get_attr_str(std::string name, unsigned char lab)
{
    if (g_attr_label_map.find(name) == g_attr_label_map.end())
    {
        return "\033[1;30;31munknown\033[0m";
    }
    return "\033[1;30;32m" + g_attr_label_map[name][lab] + "\033[0m";
}

int inference(ax_algorithm_handle_t handle_det, ax_algorithm_handle_t handle_attr, cv::Mat &image)
{
    ax_image_t image_rgb;
    ax_create_image(image.cols, image.rows, image.cols, ax_color_space_rgb, &image_rgb);
    memcpy(image_rgb.pVir, image.data, image_rgb.nSize);

    ax_result_t result;
    memset(&result, 0, sizeof(ax_result_t));
    ax_algorithm_inference(handle_det, &image_rgb, &result);

    for (int i = 0; i < result.n_objects; i++)
    {
        ax_body_attr_t body_attr = {0};
        auto &box = result.objects[i];
        // 设置track_id，用作历史状态跟踪
        body_attr.track_id = box.track_id;
        int ret = ax_algorithm_get_body_attr(handle_attr, &image_rgb, &box.bbox, &body_attr);
        if (ret != 0)
        {
            printf("track_id:%d get body attr failed, ret:%d\n", box.track_id, ret);
            continue;
        }
        printf("track_id:%d umbrella: %s headwear: %s glasses: %s faceMask: %s smoke: %s carryingItem: %s cellphone: %s safetyClothing: %s upperWear: %s upperColor: %s upperWearFg: %s upperWearTexture: %s bag: %s safetyRope: %s upperCut: %s lowerWear: %s lowerColor: %s vehicle: %s lowerCut: %s occlusion: %s orientation: %s isHuman: %s gender: %s race: %s age: %s \n",
               box.track_id,
               get_attr_str("umbrella", body_attr.umbrella).c_str(),
               get_attr_str("headwear", body_attr.headwear).c_str(),
               get_attr_str("glasses", body_attr.glasses).c_str(),
               get_attr_str("faceMask", body_attr.faceMask).c_str(),
               get_attr_str("smoke", body_attr.smoke).c_str(),
               get_attr_str("carryingItem", body_attr.carryingItem).c_str(),
               get_attr_str("cellphone", body_attr.cellphone).c_str(),
               get_attr_str("safetyClothing", body_attr.safetyClothing).c_str(),
               get_attr_str("upperWear", body_attr.upperWear).c_str(),
               get_attr_str("upperColor", body_attr.upperColor).c_str(),
               get_attr_str("upperWearFg", body_attr.upperWearFg).c_str(),
               get_attr_str("upperWearTexture", body_attr.upperWearTexture).c_str(),
               get_attr_str("bag", body_attr.bag).c_str(),
               get_attr_str("safetyRope", body_attr.safetyRope).c_str(),
               get_attr_str("upperCut", body_attr.upperCut).c_str(),
               get_attr_str("lowerWear", body_attr.lowerWear).c_str(),
               get_attr_str("lowerColor", body_attr.lowerColor).c_str(),
               get_attr_str("vehicle", body_attr.vehicle).c_str(),
               get_attr_str("lowerCut", body_attr.lowerCut).c_str(),
               get_attr_str("occlusion", body_attr.occlusion).c_str(),
               get_attr_str("orientation", body_attr.orientation).c_str(),
               get_attr_str("isHuman", body_attr.isHuman).c_str(),
               get_attr_str("gender", body_attr.gender).c_str(),
               get_attr_str("race", body_attr.race).c_str(),
               get_attr_str("age", body_attr.age).c_str());
    }

    ax_release_image(&image_rgb);

    for (int i = 0; i < result.n_objects; i++)
    {
        auto &box = result.objects[i];
        cv::rectangle(image, cv::Rect(box.bbox.x, box.bbox.y, box.bbox.w, box.bbox.h), cv::Scalar(255, 0, 0), 2);
        switch (result.model_type)
        {
        case ax_model_type_person_detection:
        {
            if (box.person_info.status == 3)
            {
                continue;
            }

            cv::putText(image, std::to_string(box.person_info.status) + " " + std::to_string(box.track_id), cv::Point(box.bbox.x, box.bbox.y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
            printf("status: %d, track_id: %d\n", box.person_info.status, box.track_id);
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

    ax_algorithm_handle_t handle_det, handle_attr;

    {
        ax_algorithm_init_t init_info;
        init_info.model_type = ax_model_type_person_detection;
        sprintf(init_info.model_file, model_path.c_str());
        init_info.param = ax_algorithm_get_default_param();

        if (ax_algorithm_init(&init_info, &handle_det) != 0)
        {
            return -1;
        }
    }

    {
        ax_algorithm_init_t init_info;
        init_info.model_type = ax_model_type_person_attr;
        sprintf(init_info.model_file, model_path.c_str());
        init_info.param = ax_algorithm_get_default_param();

        if (ax_algorithm_init(&init_info, &handle_attr) != 0)
        {
            return -1;
        }
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
        inference(handle_det, handle_attr, image);
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        auto out_path = string_utils::join(output_path, string_utils::basename(image_path));
        printf("out_path: %s\n", out_path.c_str());
        cv::imwrite(out_path, image);
    }
    else
    {

        std::vector<std::string> image_list;
        cv::glob(image_path + "/*.*", image_list);

        for (auto image_path_ : image_list)
        {
            cv::Mat image = cv::imread(image_path_);
            if (image.data)
            {
                // cv::resize(image, image, cv::Size(1920, 1080));
                cv::resize(image, image, cv::Size(ALIGN_UP(image.cols, 128), ALIGN_UP(image.rows, 128)));
                cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                inference(handle_det, handle_attr, image);
                cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
                auto out_path = string_utils::join(output_path, string_utils::basename(image_path_));
                printf("out_path: %s\n", out_path.c_str());
                cv::imwrite(out_path, image);
            }
        }
    }
    ax_algorithm_deinit(handle_det);
    ax_algorithm_deinit(handle_attr);
    AX_ENGINE_Deinit();
    AX_IVPS_Deinit();
    AX_SYS_Deinit();

    return 0;
}
