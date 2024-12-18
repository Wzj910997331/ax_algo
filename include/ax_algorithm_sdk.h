#ifndef __AX_ALGORITHM_SDK_H__
#define __AX_ALGORITHM_SDK_H__

#ifdef __cplusplus
extern "C"
{
#endif
#define AX_ALGORITHM_MAX_OBJ_NUM 32
#define AX_ALGORITHM_FACE_POINT_LEN 5
#define AX_ALGORITHM_FACE_FEATURE_LEN 512
    typedef enum _log_level_e
    {
        ax_log_disable = -1,
        ax_log_emergency = 0,
        ax_log_alert = 1,
        ax_log_critical = 2,
        ax_log_error = 3,
        ax_log_warn = 4,
        ax_log_notice = 5,
        ax_log_info = 6,
        ax_log_debug = 7,
        ax_log_all = 8,
    } ax_log_level_e;

    typedef enum _error_code_e
    {
        ax_error_code_fail = -1,
        ax_error_code_success = 0,

        ax_error_code_init_fail = 0x10000,
        ax_error_code_init_bsp_fail,
        ax_error_code_init_license_fail,
        ax_error_code_init_model_fail,

        ax_error_code_run_fail = 0x20000,
        ax_error_code_run_det_fail,
        ax_error_code_run_roi_fail,
        ax_error_code_run_crop_fail,
        ax_error_code_run_align_fail,
        ax_error_code_run_quality_fail,
        ax_error_code_run_recog_fail,
        ax_error_code_run_invalid_index,
        ax_error_code_run_type_not_match,
        ax_error_code_run_no_implement,
    } ax_error_code_e;

    typedef enum _color_space_e
    {
        ax_color_space_unknown,
        ax_color_space_nv12,
        ax_color_space_nv21,
        ax_color_space_bgr,
        ax_color_space_rgb,
    } ax_color_space_e;

    typedef struct _point_t
    {
        float x, y;
    } ax_point_t;

    typedef struct _bbox_t
    {
        float x, y, w, h;
    } ax_bbox_t;

    typedef struct _image_t
    {
        unsigned long long int pPhy;
        void *pVir;
        unsigned int nSize;
        unsigned int nWidth;
        unsigned int nHeight;
        ax_color_space_e eDtype;
        int tStride_W;

    } ax_image_t;

    typedef enum _model_type_e
    {
        ax_model_type_person_detection = 0,
        ax_model_type_person_attr,
        ax_model_type_lpr,
        ax_model_type_face_detection,
        ax_model_type_face_recognition,
        ax_model_type_fire_smoke,
        ax_model_type_end
    } ax_model_type_e;

    typedef struct _body_attr_t
    {
        /**
         * track_id: 人体跟踪ID,用作历史状态跟踪，如果设置成 0，则不跟踪，只输出当前图像推理结果
         */
        unsigned long int track_id;

        unsigned char isHuman;          // ["Uncertain", "Normal", "Abnormal"]
        unsigned char age;              // ["Uncertain", "Toddler", "Teenager", "Youth", "Middle-aged", "Elderly"]
        unsigned char gender;           // ["Uncertain", "Male", "Female"]
        unsigned char race;             // ["Uncertain", "East Asia", "Caucasian", "African", "South Asia"]
        unsigned char umbrella;         // ["Uncertain", "No", "Yes"]
        unsigned char headwear;         // ["Uncertain", "No", "hat", "Helmet"]
        unsigned char glasses;          // ["Uncertain", "No", "Glasses", "Sunglasses"]
        unsigned char faceMask;         // ["Uncertain", "No", "Yes"]
        unsigned char smoke;            // ["Uncertain", "No", "Yes"]
        unsigned char carryingItem;     // ["Uncertain", "No", "Yes"]
        unsigned char cellphone;        // ["Uncertain", "No", "Yes"]
        unsigned char safetyClothing;   // ["Uncertain", "No", "Yes"]
        unsigned char upperWear;        // ["Uncertain", "Long-sleeve", "Short-sleeve"]
        unsigned char upperColor;       // ["Uncertain", "Red", "Orange", "Yellow", "Green", "Blue", "Purple", "Pink", "Black", "White", "Gray", "Brown"]
        unsigned char upperWearFg;      // ["Uncertain", "T-shirt", "Sleeveless Top", "Shirt", "Suit", "Sweater", "Jacket", "Down Jacket", "Trench Coat", "Coat"]
        unsigned char upperWearTexture; // ["Uncertain", "Solid Color", "Pattern", "Small Floral", "Stripes or Plaid"]
        unsigned char bag;              // ["Uncertain", "No", "Crossbody Bag", "Backpack"]
        unsigned char safetyRope;       // ["Uncertain", "No", "Yes"]
        unsigned char upperCut;         // ["Uncertain", "No", "Yes"]
        unsigned char lowerWear;        // ["Uncertain", "Long Pants", "Shorts", "Long Dress", "Short Skirt"]
        unsigned char lowerColor;       // ["Uncertain", "Red", "Orange", "Yellow", "Green", "Blue", "Purple", "Pink", "Black", "White", "Gray", "Brown"]
        unsigned char vehicle;          // ["Uncertain", "No", "Motorcycle", "Bicycle", "Tricycle"]
        unsigned char lowerCut;         // ["Uncertain", "No", "Yes"]
        unsigned char occlusion;        // ["Uncertain", "No", "Mild Occlusion", "Heavy Occlusion"]
        unsigned char orientation;      // ["Uncertain", "Front", "Back", "Right Side", "Left Side"]
    } ax_body_attr_t;

    typedef struct _result_t
    {
        ax_model_type_e model_type;
        struct
        {
            ax_bbox_t bbox;
            float score;
            int label;
            unsigned long int track_id;

            struct
            {
                /*
                0到1之间的值，表示人脸质量，越高越好
                */
                float quality;
                ax_point_t points[AX_ALGORITHM_FACE_POINT_LEN];
            } face_info;

            struct
            {
                /*
                人体状态： 0：正面， 1：侧面，2：背面， 3：非人
                */
                int status;
            } person_info;

            struct
            {
                /**
                火、烟、其他
                */
                int label;
            } fire_smoke_info;

            struct
            {
                /*
                车辆类型: 0：UNKNOWN 1：SEDAN 2：SUV 3：BUS 4：MICROBUS 5：TRUCK
                */
                int cartype;

                /*
                如果 b_is_track_plate = 1，则表示当前帧没有识别到车牌，返回的是历史上 track_id 上一次识别到的车牌结果
                如果 b_is_track_plate = 0，且 len_plate_id > 0, 则表示当前帧识别到了车牌
                如果 b_is_track_plate = 0，且 len_plate_id = 0, 则表示当前帧没有识别到车牌，且是历史上 track_id 也没有结果
                */
                int b_is_track_plate;
                int len_plate_id;
                int plate_id[16];
            } vehicle_info;

        } objects[AX_ALGORITHM_MAX_OBJ_NUM];
        int n_objects;
    } ax_result_t;

    typedef struct _algorithm_param_t
    {
        struct
        {
            /**
             *  det_threshold: 人脸检测阈值，0-1之间
             *  quality_threshold: 人脸质量评分阈值，0-1之间
             */
            float det_threshold;
            float quality_threshold;
        } face_param;
        struct
        {
            /**
             *  det_threshold: 人体检测阈值，0-1之间
             */
            float det_threshold;
        } person_param;
        struct
        {
            /**
             *  det_threshold: 车辆检测阈值，0-1之间
             *  lpr_threshold: 车牌识别阈值，0-1之间
             */
            float det_threshold;
            float lpr_threshold;
        } vehicle_param;
        struct
        {
            float det_threshold;
        } fire_smoke_param;
    } ax_algorithm_param_t;

    typedef void *ax_algorithm_handle_t;

    typedef struct _algorithm_init_t
    {
        char license_path[256];
        char model_file[256];
        ax_model_type_e model_type;
        ax_algorithm_param_t param;
    } ax_algorithm_init_t;

    int ax_algorithm_init(ax_algorithm_init_t *init_info, ax_algorithm_handle_t *handle);
    void ax_algorithm_deinit(ax_algorithm_handle_t handle);

    /**
     * @brief: 与ax_algorithm_track的区别是不进行跟踪, 一般只用作精度验证，或者人脸注册的检测阶段
     * @param[in] handle: 算法句柄
     * @param[in] image: 图像数据
     * @param[out] result: 检测结果
     * @return 0 成功，非零表示失败。
     */
    int ax_algorithm_detect(ax_algorithm_handle_t handle, ax_image_t *image, ax_result_t *result);
    /**
     * @brief: 检测+跟踪等一系列算法
     * @param[in] handle: 算法句柄
     * @param[in] image: 图像数据
     * @param[out] result: 跟踪结果
     * @return 0 成功，非零表示失败。
     */
    int ax_algorithm_track(ax_algorithm_handle_t handle, ax_image_t *image, ax_result_t *result);


    ax_model_type_e ax_algorithm_get_model_type(ax_algorithm_handle_t handle);

    /**
     * @brief: 获取算法参数
     * @param[in] handle: 算法句柄
     * @return 算法参数
     */
    ax_algorithm_param_t ax_algorithm_get_param(ax_algorithm_handle_t handle);
    /**
     * @brief: 设置算法参数
     * @param[in] handle: 算法句柄
     * @param[in] param: 算法参数
     */
    void ax_algorithm_set_param(ax_algorithm_handle_t handle, ax_algorithm_param_t *param);
    /**
     * @brief: 获取默认算法参数
     * @return 算法参数
     */
    ax_algorithm_param_t ax_algorithm_get_default_param();

    /**
     * @brief: 设置日志级别
     * @param[in] level: 小于level的日志将被打印，大于level的日志将被忽略
     */
    void ax_algorithm_set_log_level(ax_log_level_e level);

    /**
     * @brief: 保存调试图像
     * @param[in] handle: 算法句柄
     * @param[in] enable: 1: 启用保存调试图像, 0: 禁用
     */
    void ax_algorithm_save_debug_image(ax_algorithm_handle_t handle, int enable);

    /**
     * @brief: 将 plate_id 转换为字符串
     * @param[in] plate_id: plate_id 数组
     * @param[in] len: plate_id 数组的长度
     * @param[out] plate_str: plate_id 的字符串
     * @return 0 成功，非零表示失败。
     */
    int ax_algorithm_get_plate_str(int *plate_id, int len, char *plate_str);

    /**
     * @brief: 获取检测到的人体属性
     * @param[in] handle: 算法句柄
     * @param[in] image: 图像数据
     * @param[in] bbox: 检测到的人体 bounding box
     * @param[out] body_attr: 检测到的人体属性
     * @return 0 成功，非零表示失败。
     */
    int ax_algorithm_get_body_attr(ax_algorithm_handle_t handle, ax_image_t *image, ax_bbox_t *bbox, ax_body_attr_t *body_attr);

    /**
     * @brief: 获取检测到的人脸特征
     * @param[in] handle: 算法句柄
     * @param[in] image: 图像数据
     * @param[in] result: 检测结果
     * @param[in] idx: result.objects 中的人脸索引
     *                  如果 idx 是 -1, 则表示此图片还没有进行人脸检测
     *                  这个函数将自动检测图像中的人脸并获取特征，但不进行跟踪
     * @param[out] feature: 512维的人脸特征
     * @return 0 成功，非零表示失败。
     */
    int ax_algorithm_get_face_feature(ax_algorithm_handle_t handle, ax_image_t *image, ax_result_t *result, int idx, float feature[AX_ALGORITHM_FACE_FEATURE_LEN]);

    /**
     * @brief: 比较两个人脸特征
     * @param[in] a: 第一个人脸特征数组
     * @param[in] b: 第二个人脸特征数组
     * @return 两个人脸特征之间的相似度评分
     */
    float ax_algorithm_face_compare(float a[AX_ALGORITHM_FACE_FEATURE_LEN], float b[AX_ALGORITHM_FACE_FEATURE_LEN]);

    /**
     * @brief: 根据指定的参数创建一幅图像。
     * @param[in] width: 图像的宽度。
     * @param[in] height: 图像的高度。
     * @param[in] stride: 图像的步幅。
     * @param[in] color: 图像的颜色空间 (例如，NV12, NV21, BGR, RGB)。
     * @param[out] image: 指向图像结构体的指针，用于初始化。
     * @return 成功时返回 0，失败时返回非零值。
     */
    int ax_create_image(int width, int height, int stride, ax_color_space_e color, ax_image_t *image);

    /**
     * @brief: 释放ax_create_image创建的一幅图像。
     * @param[in] image: 指向要释放的图像结构体的指针。
     */
    void ax_release_image(ax_image_t *image);

#ifdef __cplusplus
}

#endif

#endif // __AX_ALGORITHM_SDK_H__