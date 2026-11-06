#pragma once
#include "../../../Common/CropState.h"

// 밭 하나의 서버 상태. objectId는 월드 객체 ID와 같은 값을 사용한다.
struct FieldState {
    int objectId = -1;
    CropState crop;
};
