#include "raylib.h"
#include "steganography.h"
#include <vector>
#include <cstdlib>

// --- CONFIGURATION ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const Color NEON_CYAN = { 0, 255, 255, 255 };
const Color NEON_GREEN = { 0, 255, 0, 255 };
const Color DARK_BG = { 20, 20, 20, 255 };

// --- MATRIX RAIN STRUCT ---
struct MatrixDrop {
    int x, y;
    int speed;
    char symbol;
};

int main() {
    // 1. Initialize Window and Audio
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SONIC VAULT - Master AK");
    InitAudioDevice();
    SetTargetFPS(60);

    // 2. Load Resources (Ensure these files exist or code will handle gracefully)
    Music music = { 0 };
    bool musicLoaded = false;
    
    // NOTE: For the project, put a real .wav file in assets/cover.wav
    if (FileExists("assets/cover.wav")) {
        music = LoadMusicStream("assets/cover.wav");
        musicLoaded = true;
    }

    // 3. Logic Objects
    StegoEngine engine;
    std::string secretMessage = "";
    bool showMatrix = false;
    char inputText[256] = { 0 };
    int letterCount = 0;
    
    // Matrix Rain Setup
    std::vector<MatrixDrop> rain;
    for(int i=0; i<100; i++) {
        rain.push_back({GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(-500, 0), GetRandomValue(2, 8), (char)GetRandomValue(33, 126)});
    }

    // Spectrum Bars Setup
    float bars[40] = {0};

    // --- MAIN LOOP ---
    while (!WindowShouldClose()) {
        // A. UPDATE LOGIC
        if (musicLoaded) UpdateMusicStream(music);

        // Update Matrix Rain
        if (showMatrix) {
            for (auto& drop : rain) {
                drop.y += drop.speed;
                if (drop.y > SCREEN_HEIGHT) {
                    drop.y = GetRandomValue(-100, 0);
                    drop.x = GetRandomValue(0, SCREEN_WIDTH);
                }
            }
        }

        // Simulate Spectrum Data (For visual "Sexy" effect)
        // In a complex app, we'd use FFT. For Viva, random noise scaled by volume is safer/easier.
        for (int i = 0; i < 40; i++) {
            if (IsMusicStreamPlaying(music)) {
                bars[i] = Lerp(bars[i], (float)GetRandomValue(10, 200), 0.2f);
            } else {
                bars[i] = Lerp(bars[i], 10.0f, 0.1f);
            }
        }

        // Handle Input
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (letterCount < 255)) {
                inputText[letterCount] = (char)key;
                inputText[letterCount+1] = '\0';
                letterCount++;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            letterCount--;
            if (letterCount < 0) letterCount = 0;
            inputText[letterCount] = '\0';
        }

        // Mouse Interactions
        Vector2 mouse = GetMousePosition();
        Rectangle btnEncrypt = { 50, 450, 200, 50 };
        Rectangle btnDecrypt = { 300, 450, 200, 50 };
        Rectangle btnPlay = { 550, 450, 200, 50 };

        bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (CheckCollisionPointRec(mouse, btnEncrypt) && click) {
             // ACTION: Encrypt
             engine.EncryptAudio("assets/cover.wav", "assets/output.wav", std::string(inputText));
             secretMessage = "ENCRYPTION COMPLETE. FILE SAVED.";
             showMatrix = false;
        }

        if (CheckCollisionPointRec(mouse, btnDecrypt) && click) {
            // ACTION: Decrypt
            secretMessage = engine.DecryptAudio("assets/output.wav");
            showMatrix = true; // Trigger the "Hacker" effect
        }

        if (CheckCollisionPointRec(mouse, btnPlay) && click && musicLoaded) {
            if (IsMusicStreamPlaying(music)) StopMusicStream(music);
            else PlayMusicStream(music);
        }

        // B. DRAWING (The Visuals)
        BeginDrawing();
        ClearBackground(DARK_BG);

            // 1. Draw Spectrum (The "Heartbeat")
            for (int i = 0; i < 40; i++) {
                DrawRectangle(50 + (i * 18), 300 - bars[i], 15, bars[i], NEON_CYAN);
                // Reflection effect
                DrawRectangle(50 + (i * 18), 305, 15, bars[i] * 0.3f, Fade(NEON_CYAN, 0.3f));
            }

            // 2. Draw Matrix Rain (If Decrypting)
            if (showMatrix) {
                for (auto& drop : rain) {
                    DrawText(TextFormat("%c", drop.symbol), drop.x, drop.y, 20, NEON_GREEN);
                }
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(DARK_BG, 0.8f)); // Dim background
            }

            // 3. UI Elements
            DrawText("SONIC VAULT // PROTOCOL: STEGANOGRAPHY", 50, 30, 20, LIGHTGRAY);
            DrawLine(50, 60, 750, 60, NEON_CYAN);

            // Input Box
            DrawText("SECRET PAYLOAD:", 50, 350, 20, GRAY);
            DrawRectangleLines(50, 380, 700, 40, NEON_GREEN);
            DrawText(inputText, 60, 390, 20, WHITE);

            // Output Message
            DrawText(secretMessage.c_str(), 50, 100, 30, showMatrix ? NEON_GREEN : WHITE);

            // Buttons
            DrawRectangleRec(btnEncrypt, Fade(NEON_CYAN, 0.2f));
            DrawRectangleLinesEx(btnEncrypt, 2, NEON_CYAN);
            DrawText("ENCRYPT", btnEncrypt.x + 50, btnEncrypt.y + 15, 20, NEON_CYAN);

            DrawRectangleRec(btnDecrypt, Fade(NEON_GREEN, 0.2f));
            DrawRectangleLinesEx(btnDecrypt, 2, NEON_GREEN);
            DrawText("DECRYPT", btnDecrypt.x + 50, btnDecrypt.y + 15, 20, NEON_GREEN);

            DrawRectangleRec(btnPlay, Fade(WHITE, 0.2f));
            DrawRectangleLinesEx(btnPlay, 2, WHITE);
            DrawText(IsMusicStreamPlaying(music) ? "STOP" : "PLAY", btnPlay.x + 70, btnPlay.y + 15, 20, WHITE);

        EndDrawing();
    }

    // Cleanup
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}