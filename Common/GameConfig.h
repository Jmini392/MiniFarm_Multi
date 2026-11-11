#pragma once

namespace GameConfig {
    // 서버 농사 요청과 판정이 연결되기 전에는 기존 로컬 농사를 유지한다.
    // true로 전환할 때는 서버와 클라이언트를 모두 다시 빌드한다.
    constexpr bool ServerFarmingEnabled = false;
}
