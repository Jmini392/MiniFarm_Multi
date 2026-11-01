#pragma once

class InputManager {
public:
    static void Init();
    static void Clear();
    static void Update();

    static void KeyDown(unsigned char key, int x, int y);
    static void KeyUp(unsigned char key, int x, int y);
    static bool IsKeyDown(unsigned char key);
    static bool IsKeyPressed(unsigned char key);

    static void MouseButton(int button, int state, int x, int y);
    static void MouseMove(int x, int y);
    static int GetDeltaX(){ return s_deltaX; }
    static int GetDeltaY(){ return s_deltaY; }
    static void ResetDelta();

    static void SetUIMode(bool enable);
    static bool IsUIMode() { return s_uiMode; }

private:
    static std::array<bool, 256> s_keys;
    static std::array<bool, 256> s_prevKeys;

    static int s_deltaX;
    static int s_deltaY;

    static int  s_centerX;
    static int  s_centerY;
    static bool s_warping;

    static bool s_uiMode;
};