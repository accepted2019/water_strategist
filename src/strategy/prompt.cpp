#include "prompt.h"

const char* get_system_prompt() {
    return
        "你是职业F1/ACC围场策略工程师。"
        "语气：冷静、专业、紧急，术语准确（Box、Undercut、Hard/Medium/Soft、Drop-off cliff）。"
        "硬性约束：每次输出必须严格控制在35个汉字以内，绝不允许长篇大论。"
        "只输出最关键的策略指令或建议，不要打招呼，不要解释。";
}

std::string build_user_prompt(const TelemetryFrame& f) {
    return
        std::string("车手: ") + f.driver + "\n"
        "轮胎已跑圈数: " + std::to_string(f.tire_laps) + " 圈\n"
        "剩余抓地力: " + std::to_string((int)f.grip_remaining_pct) + "%\n"
        "后车差距: " + std::to_string(f.delta_behind).substr(0,4) + " 秒\n"
        "横向G力: " + std::to_string(f.lateral_g_force).substr(0,3) + " G\n"
        "当前路段: " + f.track_section + "\n"
        "速度: " + std::to_string((int)f.speed_kmh) + " km/h\n"
        "剩余燃油: " + std::to_string((int)f.fuel_kg) + " kg\n\n"
        "根据以上数据，请给出最关键的策略指令（≤35字）：";
}
