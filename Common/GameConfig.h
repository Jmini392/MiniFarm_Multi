#pragma once

namespace GameConfig {
    // true: request farming and display server state; false: local farming checks.
    // Rebuild both server and client, then reconnect after changing this setting.
    constexpr bool ServerFarmingEnabled = true;
}
