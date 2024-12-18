from pycocotools.coco import COCO
from pycocotools.cocoeval import COCOeval

# 1. 加载真值标注文件
gt_path = "output_coco.json"  # 替换为标注文件路径
coco_gt = COCO(gt_path)

# 2. 加载预测结果文件
pred_path = "model_out.json"  # 替换为预测结果文件路径
coco_dt = coco_gt.loadRes(pred_path)

# 3. 初始化 COCOeval
coco_eval = COCOeval(coco_gt, coco_dt, iouType="bbox")

# 4. 运行评估
coco_eval.evaluate()   # 计算匹配情况
coco_eval.accumulate() # 汇总统计指标
coco_eval.summarize()  # 打印结果

