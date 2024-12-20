import os
import json
import xml.etree.ElementTree as ET
import re

def voc_to_coco(xml_folder, output_json):
    # COCO格式数据初始化
    coco_data = {
        "images": [],
        "annotations": [],
        "categories": []
    }

    category_set = {}
    annotation_id = 1
    image_id = 1

    # 获取 XML 文件并按自然顺序排序
    def natural_key(file_name):
        return [int(text) if text.isdigit() else text.lower() for text in re.split(r'(\d+)', file_name)]
    xml_files = sorted([f for f in os.listdir(xml_folder) if f.endswith(".xml")], key=natural_key)

    for xml_file in xml_files:
        xml_path = os.path.join(xml_folder, xml_file)
        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()

            # 提取图像信息
            filename = root.find("filename").text
            size = root.find("size")
            width = int(size.find("width").text)
            height = int(size.find("height").text)

            coco_data["images"].append({
                "id": image_id,
                "file_name": filename,
                "width": width,
                "height": height
            })

            # 提取目标对象信息
            for obj in root.findall("object"):
                category = obj.find("name").text
                if category not in category_set:
                    category_set[category] = len(category_set) + 1
                    coco_data["categories"].append({
                        "id": category_set[category],
                        "name": category
                    })

                bndbox = obj.find("bndbox")
                xmin = int(bndbox.find("xmin").text)
                ymin = int(bndbox.find("ymin").text)
                xmax = int(bndbox.find("xmax").text)
                ymax = int(bndbox.find("ymax").text)

                bbox_width = xmax - xmin
                bbox_height = ymax - ymin
                area = bbox_width * bbox_height

                coco_data["annotations"].append({
                    "id": annotation_id,
                    "image_id": image_id,
                    "category_id": category_set[category],
                    "bbox": [xmin, ymin, bbox_width, bbox_height],
                    "iscrowd": 0,
                    "area": area,
                })

                annotation_id += 1

            image_id += 1
        except Exception as e:
            print(f"Error processing file {xml_file}: {e}")

    # 按顺序构造最终输出
    ordered_coco_data = {
        "images": coco_data["images"],
        "annotations": coco_data["annotations"],
        "categories": coco_data["categories"]
    }

    # 写入 JSON 文件，确保顺序
    try:
        with open(output_json, "w") as json_file:
            json.dump(ordered_coco_data, json_file, indent=4)
        print(f"Converted VOC to COCO format. Output saved to {output_json}")
    except Exception as e:
        print(f"Error saving JSON file: {e}")

# 示例调用
xml_folder = "./img/dst_img"  # 替换为您的 XML 文件夹路径
output_json = "output_coco.json"   # 替换为您的输出 JSON 文件路径
voc_to_coco(xml_folder, output_json)
