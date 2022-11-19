#include <iostream>
#include <fstream>
#include <cstring>

#include "json.hpp"

void c_str_test() {
    const char* json = R"({
    "face_num": 1,
    "face_list": [
        {
            "face_token": "3e9ec61f1d2c1a7ad3d06eb5d1a47fc4",
            "location": {
                "left": 728.56,
                "top": 275.63,
                "width": 321,
                "height": 326,
                "rotation": 8
            },
            "face_probability": 1,
            "angle": {
                "yaw": -13.45,
                "pitch": -2.43,
                "roll": 7.23
            },
            "age": 23,
            "expression": {
                "type": "smile",
                "probability": 0.73
            },
            "face_shape": {
                "type": "oval",
                "probability": 0.5
            },
            "gender": {
                "type": "female",
                "probability": 1
            },
            "glasses": {
                "type": "none",
                "probability": 1
            },
            "landmark150": {
                "cheek_right_1": {
                    "x": 709.37,
                    "y": 363.57
                },
                "cheek_right_3": {
                    "x": 715.34,
                    "y": 418.18
                },
                "cheek_right_5": {
                    "x": 723.4,
                    "y": 474.36
                },
                "cheek_right_7": {
                    "x": 745.12,
                    "y": 527.91
                },
                "cheek_right_9": {
                    "x": 790.36,
                    "y": 573.56
                },
                "cheek_right_11": {
                    "x": 841.64,
                    "y": 607.22
                },
                "chin_2": {
                    "x": 893.16,
                    "y": 613.44
                },
                "cheek_left_11": {
                    "x": 931.06,
                    "y": 596.24
                },
                "cheek_left_9": {
                    "x": 973.07,
                    "y": 560.24
                },
                "cheek_left_7": {
                    "x": 999.04,
                    "y": 521.14
                },
                "cheek_left_5": {
                    "x": 1018.68,
                    "y": 483.55
                },
                "cheek_left_3": {
                    "x": 1032.67,
                    "y": 441.29
                },
                "cheek_left_1": {
                    "x": 1041.75,
                    "y": 396.14
                },
                "eye_right_corner_right": {
                    "x": 805.99,
                    "y": 348.2
                },
                "eye_right_eyelid_upper_2": {
                    "x": 823.76,
                    "y": 333.95
                },
                "eye_right_eyelid_upper_4": {
                    "x": 843.92,
                    "y": 330.32
                },
                "eye_right_eyelid_upper_6": {
                    "x": 862.3,
                    "y": 339.65
                },
                "eye_right_corner_left": {
                    "x": 874.05,
                    "y": 359.21
                },
                "eye_right_eyelid_lower_6": {
                    "x": 858.07,
                    "y": 358.82
                },
                "eye_right_eyelid_lower_4": {
                    "x": 840.17,
                    "y": 357.02
                },
                "eye_right_eyelid_lower_2": {
                    "x": 821.72,
                    "y": 352.9
                },
                "eye_right_eyeball_center": {
                    "x": 837.69,
                    "y": 344.03
                },
                "eyebrow_right_corner_right": {
                    "x": 784.32,
                    "y": 322.12
                },
                "eyebrow_right_upper_2": {
                    "x": 814.09,
                    "y": 301.74
                },
                "eyebrow_right_upper_3": {
                    "x": 845.5,
                    "y": 297.64
                },
                "eyebrow_right_upper_4": {
                    "x": 875.81,
                    "y": 301.6
                },
                "eyebrow_right_corner_left": {
                    "x": 899.51,
                    "y": 323.03
                },
                "eyebrow_right_lower_3": {
                    "x": 872.19,
                    "y": 318.16
                },
                "eyebrow_right_lower_2": {
                    "x": 842.95,
                    "y": 313.1
                },
                "eyebrow_right_lower_1": {
                    "x": 814.08,
                    "y": 314.34
                },
                "eye_left_corner_right": {
                    "x": 954.52,
                    "y": 367.89
                },
                "eye_left_eyelid_upper_2": {
                    "x": 969.4,
                    "y": 352.14
                },
                "eye_left_eyelid_upper_4": {
                    "x": 987.94,
                    "y": 348.8
                },
                "eye_left_eyelid_upper_6": {
                    "x": 1002.39,
                    "y": 356.65
                },
                "eye_left_corner_left": {
                    "x": 1009.62,
                    "y": 374.83
                },
                "eye_left_eyelid_lower_6": {
                    "x": 998.45,
                    "y": 376.56
                },
                "eye_left_eyelid_lower_4": {
                    "x": 984.31,
                    "y": 375.17
                },
                "eye_left_eyelid_lower_2": {
                    "x": 967.92,
                    "y": 371.54
                },
                "eye_left_eyeball_center": {
                    "x": 975.9,
                    "y": 361.55
                },
                "eyebrow_left_corner_right": {
                    "x": 956.82,
                    "y": 332.62
                },
                "eyebrow_left_upper_2": {
                    "x": 975.04,
                    "y": 317.78
                },
                "eyebrow_left_upper_3": {
                    "x": 994.67,
                    "y": 316.36
                },
                "eyebrow_left_upper_4": {
                    "x": 1013.23,
                    "y": 323.82
                },
                "eyebrow_left_corner_left": {
                    "x": 1025.44,
                    "y": 341.81
                },
                "eyebrow_left_lower_3": {
                    "x": 1011.53,
                    "y": 334.38
                },
                "eyebrow_left_lower_2": {
                    "x": 994.85,
                    "y": 331.48
                },
                "eyebrow_left_lower_1": {
                    "x": 975.94,
                    "y": 331.57
                },
                "nose_right_contour_1": {
                    "x": 900.73,
                    "y": 367
                },
                "nose_right_contour_2": {
                    "x": 897.39,
                    "y": 393.53
                },
                "nose_right_contour_3": {
                    "x": 895.57,
                    "y": 420.02
                },
                "nose_right_contour_4": {
                    "x": 886.88,
                    "y": 450.66
                },
                "nose_right_contour_6": {
                    "x": 906.57,
                    "y": 449
                },
                "nose_left_contour_6": {
                    "x": 947.55,
                    "y": 458.09
                },
                "nose_left_contour_4": {
                    "x": 962.23,
                    "y": 451.07
                },
                "nose_left_contour_3": {
                    "x": 955.56,
                    "y": 420.09
                },
                "nose_left_contour_2": {
                    "x": 947.51,
                    "y": 394.15
                },
                "nose_left_contour_1": {
                    "x": 939.88,
                    "y": 367.61
                },
                "nose_tip": {
                    "x": 939.48,
                    "y": 436.86
                },
                "mouth_corner_right_outer": {
                    "x": 845.47,
                    "y": 490.88
                },
                "mouth_lip_upper_outer_3": {
                    "x": 884.81,
                    "y": 481.9
                },
                "mouth_lip_upper_outer_6": {
                    "x": 923,
                    "y": 486.95
                },
                "mouth_lip_upper_outer_9": {
                    "x": 951.9,
                    "y": 493.31
                },
                "mouth_corner_left_outer": {
                    "x": 963.9,
                    "y": 508.8
                },
                "mouth_lip_lower_outer_9": {
                    "x": 941.7,
                    "y": 524.81
                },
                "mouth_lip_lower_outer_6": {
                    "x": 911.22,
                    "y": 529.5
                },
                "mouth_lip_lower_outer_3": {
                    "x": 875.11,
                    "y": 515.89
                },
                "mouth_lip_upper_inner_3": {
                    "x": 882.92,
                    "y": 491.12
                },
                "mouth_lip_upper_inner_6": {
                    "x": 919.75,
                    "y": 499.3
                },
                "mouth_lip_upper_inner_9": {
                    "x": 948.03,
                    "y": 501.84
                },
                "mouth_lip_lower_inner_9": {
                    "x": 938.4,
                    "y": 515.38
                },
                "mouth_lip_lower_inner_6": {
                    "x": 912.02,
                    "y": 513.48
                },
                "mouth_lip_lower_inner_3": {
                    "x": 878.6,
                    "y": 505.97
                },
                "cheek_right_2": {
                    "x": 712.3,
                    "y": 389.95
                },
                "cheek_right_4": {
                    "x": 719.34,
                    "y": 445.3
                },
                "cheek_right_6": {
                    "x": 731.32,
                    "y": 502.05
                },
                "cheek_right_8": {
                    "x": 765.16,
                    "y": 552.14
                },
                "cheek_right_10": {
                    "x": 815.54,
                    "y": 591.26
                },
                "chin_1": {
                    "x": 867.97,
                    "y": 614.99
                },
                "chin_3": {
                    "x": 912.13,
                    "y": 608.38
                },
                "cheek_left_10": {
                    "x": 951.88,
                    "y": 576.95
                },
                "cheek_left_8": {
                    "x": 985.24,
                    "y": 540.61
                },
                "cheek_left_6": {
                    "x": 1007.58,
                    "y": 502.84
                },
                "cheek_left_4": {
                    "x": 1026.19,
                    "y": 463.55
                },
                "cheek_left_2": {
                    "x": 1035.55,
                    "y": 419.06
                },
                "eyebrow_right_upper_1": {
                    "x": 784.31,
                    "y": 315.24
                },
                "eyebrow_right_upper_5": {
                    "x": 901.91,
                    "y": 312.52
                },
                "eyebrow_left_upper_1": {
                    "x": 958.55,
                    "y": 324.84
                },
                "eyebrow_left_upper_5": {
                    "x": 1025.77,
                    "y": 337.1
                },
                "eye_right_eyelid_upper_1": {
                    "x": 814.14,
                    "y": 340.43
                },
                "eye_right_eyelid_upper_3": {
                    "x": 833.21,
                    "y": 331.14
                },
                "eye_right_eyelid_upper_5": {
                    "x": 853.22,
                    "y": 333.64
                },
                "eye_right_eyelid_upper_7": {
                    "x": 868.3,
                    "y": 348.6
                },
                "eye_right_eyelid_lower_7": {
                    "x": 865.11,
                    "y": 357.99
                },
                "eye_right_eyelid_lower_5": {
                    "x": 848.9,
                    "y": 357.54
                },
                "eye_right_eyelid_lower_3": {
                    "x": 830.86,
                    "y": 355.07
                },
                "eye_right_eyelid_lower_1": {
                    "x": 814.44,
                    "y": 350.11
                },
                "eye_right_eyeball_right": {
                    "x": 823.18,
                    "y": 343.45
                },
                "eye_right_eyeball_left": {
                    "x": 852.46,
                    "y": 346.78
                },
                "eye_left_eyelid_upper_1": {
                    "x": 960.57,
                    "y": 359.65
                },
                "eye_left_eyelid_upper_3": {
                    "x": 978.17,
                    "y": 349.71
                },
                "eye_left_eyelid_upper_5": {
                    "x": 995.94,
                    "y": 352.27
                },
                "eye_left_eyelid_upper_7": {
                    "x": 1007.23,
                    "y": 365.65
                },
                "eye_left_eyelid_lower_7": {
                    "x": 1003.54,
                    "y": 375.81
                },
                "eye_left_eyelid_lower_5": {
                    "x": 991.42,
                    "y": 375.68
                },
                "eye_left_eyelid_lower_3": {
                    "x": 976.56,
                    "y": 373.58
                },
                "eye_left_eyelid_lower_1": {
                    "x": 961.02,
                    "y": 369.92
                },
                "eye_left_eyeball_right": {
                    "x": 963.46,
                    "y": 361.75
                },
                "eye_left_eyeball_left": {
                    "x": 989.36,
                    "y": 364.77
                },
                "nose_bridge_1": {
                    "x": 926.37,
                    "y": 365.57
                },
                "nose_bridge_2": {
                    "x": 931.35,
                    "y": 391.91
                },
                "nose_bridge_3": {
                    "x": 936.08,
                    "y": 418.04
                },
                "nose_right_contour_5": {
                    "x": 894.31,
                    "y": 458.59
                },
                "nose_right_contour_7": {
                    "x": 907.9,
                    "y": 439.84
                },
                "nose_left_contour_7": {
                    "x": 951.94,
                    "y": 451.3
                },
                "nose_left_contour_5": {
                    "x": 948.61,
                    "y": 465.34
                },
                "nose_middle_contour": {
                    "x": 928.05,
                    "y": 464.28
                },
                "mouth_corner_right_inner": {
                    "x": 848.8,
                    "y": 490.1
                },
                "mouth_corner_left_inner": {
                    "x": 959.6,
                    "y": 507.4
                },
                "mouth_lip_upper_outer_1": {
                    "x": 857.01,
                    "y": 485.75
                },
                "mouth_lip_upper_outer_2": {
                    "x": 870.35,
                    "y": 483.4
                },
                "mouth_lip_upper_outer_4": {
                    "x": 897.37,
                    "y": 481.7
                },
                "mouth_lip_upper_outer_5": {
                    "x": 911.33,
                    "y": 482.22
                },
                "mouth_lip_upper_outer_7": {
                    "x": 931.77,
                    "y": 487.02
                },
                "mouth_lip_upper_outer_8": {
                    "x": 943.03,
                    "y": 489.53
                },
                "mouth_lip_upper_outer_10": {
                    "x": 956.41,
                    "y": 497.74
                },
                "mouth_lip_upper_outer_11": {
                    "x": 960.43,
                    "y": 502.09
                },
                "mouth_lip_lower_outer_11": {
                    "x": 956.49,
                    "y": 514.74
                },
                "mouth_lip_lower_outer_10": {
                    "x": 949.06,
                    "y": 519.83
                },
                "mouth_lip_lower_outer_8": {
                    "x": 929.68,
                    "y": 528.33
                },
                "mouth_lip_lower_outer_7": {
                    "x": 917.86,
                    "y": 530.42
                },
                "mouth_lip_lower_outer_5": {
                    "x": 899.58,
                    "y": 527.13
                },
                "mouth_lip_lower_outer_4": {
                    "x": 886.1,
                    "y": 522.44
                },
                "mouth_lip_lower_outer_2": {
                    "x": 863.15,
                    "y": 508
                },
                "mouth_lip_lower_outer_1": {
                    "x": 853.04,
                    "y": 498.93
                },
                "mouth_lip_upper_inner_1": {
                    "x": 857.74,
                    "y": 489.35
                },
                "mouth_lip_upper_inner_2": {
                    "x": 869.94,
                    "y": 489.6
                },
                "mouth_lip_upper_inner_4": {
                    "x": 895.71,
                    "y": 492.25
                },
                "mouth_lip_upper_inner_5": {
                    "x": 908.13,
                    "y": 494.82
                },
                "mouth_lip_upper_inner_7": {
                    "x": 929,
                    "y": 498.86
                },
                "mouth_lip_upper_inner_8": {
                    "x": 938.87,
                    "y": 499.48
                },
                "mouth_lip_upper_inner_10": {
                    "x": 952.95,
                    "y": 503.48
                },
                "mouth_lip_upper_inner_11": {
                    "x": 957.56,
                    "y": 505.79
                },
                "mouth_lip_lower_inner_11": {
                    "x": 954.11,
                    "y": 511.01
                },
                "mouth_lip_lower_inner_10": {
                    "x": 946.4,
                    "y": 513.07
                },
                "mouth_lip_lower_inner_8": {
                    "x": 927.97,
                    "y": 515.04
                },
                "mouth_lip_lower_inner_7": {
                    "x": 917.27,
                    "y": 514.95
                },
                "mouth_lip_lower_inner_5": {
                    "x": 900.76,
                    "y": 510.85
                },
                "mouth_lip_lower_inner_4": {
                    "x": 888.98,
                    "y": 507.93
                },
                "mouth_lip_lower_inner_2": {
                    "x": 866.55,
                    "y": 500.07
                },
                "mouth_lip_lower_inner_1": {
                    "x": 856.07,
                    "y": 494.82
                }
            },
            "quality": {
                "occlusion": {
                    "left_eye": 0,
                    "right_eye": 0.26,
                    "nose": 0.79,
                    "mouth": 0.97,
                    "left_cheek": 0.87,
                    "right_cheek": 0.98,
                    "chin_contour": 0.99
                },
                "blur": 0,
                "illumination": 224,
                "completeness": 1
            },
            "emotion": {
                "type": "angry",
                "probability": 0.26
            },
            "mask": {
                "type": 1,
                "probability": 1
            }
        }
    ]
})";

    auto obj = json::parse(json, json + strlen(json));
    obj["version"] = { 1.1 };
    auto str = json::to_json(obj);

    std::cout << "json: \n" << str << '\n';
}

void file_test() {
    try {
        const char* path = R"(./test/face.json)";
        auto obj = json::parse(path, 4096);        //缓冲区设置为1，逐个字符解析
        std::cout << "size=" << obj.size() << '\n';
        std::cout << "json:\n" << json::to_json(obj) << '\n';
        std::ofstream os{ R"(./test/test.json)" };

        obj.insert({ "array", json::array{{ {123}, {"123"} }} });

        os << json::to_json(obj);
        os.close();

    }
    catch (const std::exception& e) {
        std::cout << "fuck" << "\n";
        std::cout << e.what() << '\n';
    }
}

int main() {
    c_str_test();
    // file_test();
}
