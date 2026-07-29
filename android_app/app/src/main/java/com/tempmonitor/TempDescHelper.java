package com.tempmonitor;

/**
 * 温度描述辅助 — 根据温度值和水/气模式给出趣味性描述
 */
public class TempDescHelper {

    public enum Mode { AIR, WATER }

    /**
     * 根据模式和温度返回描述文字
     */
    public static String getDescription(float temp, Mode mode) {
        if (mode == Mode.WATER) {
            return getWaterDesc(temp);
        } else {
            return getAirDesc(temp);
        }
    }

    public static int getColor(float temp, Mode mode) {
        if (mode == Mode.WATER) {
            if (temp < 5)  return 0xFF5DADE2;   // 冰蓝
            if (temp < 20) return 0xFF3498DB;   // 冷蓝
            if (temp < 30) return 0xFF2ECC71;   // 舒适绿
            if (temp < 40) return 0xFFF39C12;   // 温热橙
            return 0xFFE74C3C;                   // 烫红
        } else {
            if (temp < 0)  return 0xFF5DADE2;
            if (temp < 15) return 0xFF3498DB;
            if (temp < 26) return 0xFF2ECC71;
            if (temp < 35) return 0xFFF39C12;
            return 0xFFE74C3C;
        }
    }

    private static String getWaterDesc(float t) {
        if (t < 0)   return "< 0°C 已结冰！❄️ 冰水混合物，皮肤接触会冻伤";
        if (t < 5)   return t + "°C ❄️ 冷水刺骨，洗把脸瞬间醒";
        if (t < 10)  return t + "°C 🧊 冰水级别，夏日冰镇西瓜的温度";
        if (t < 20)  return t + "°C 💧 凉水，自来水龙头出来的感觉";
        if (t < 26)  return t + "°C 🌊 室温水，游泳馆标准水温";
        if (t < 30)  return t + "°C 🛁 微温水，冬天泡脚刚刚好";
        if (t < 35)  return t + "°C 🥳 舒适温水，泡澡的最佳温度";
        if (t < 40)  return t + "°C 🍵 偏热水，温泉入门级温度";
        if (t < 50)  return t + "°C ☢️ 很烫！手伸进去会立刻缩回来";
        if (t < 70)  return t + "°C 🔥 高温水，泡面刚好但是手不能碰";
        return t + "°C ⚠️ 沸腾级别！水开了，离远点";
    }

    private static String getAirDesc(float t) {
        if (t < -20) return t + "°C 🥶 极寒，东北冬天的日常，哈气成冰";
        if (t < -10) return t + "°C ❄️ 严寒，羽绒服+围巾是标配";
        if (t < 0)   return t + "°C 🌨️ 冰点以下，路面结冰，走路小心";
        if (t < 5)   return t + "°C 🧊 寒冷，裹紧小被子续命的温度";
        if (t < 10)  return t + "°C 🌬️ 微寒，出门需要穿外套";
        if (t < 16)  return t + "°C 🍃 凉爽，长袖正好，秋高气爽";
        if (t < 22)  return t + "°C 🌸 宜人，体感最舒适的温度区间";
        if (t < 26)  return t + "°C ☀️ 温暖，穿单衣出门刚好，心情愉悦";
        if (t < 30)  return t + "°C 🏖️ 微热，穿短袖的天气，想吃冰淇淋";
        if (t < 35)  return t + "°C 🔥 炎热，空调续命，出门一身汗";
        if (t < 40)  return t + "°C ☄️ 酷暑，桑拿天，别在户外待太久";
        return t + "°C 🌋 极端高温！热浪来袭，小心中暑！";
    }
}
