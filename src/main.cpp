// =========================================================
// PROJECT: GHOST-TALK // FINAL VIVA EDITION
// AUTHOR:  Master AK (CodeHashiraX)
// VERSION: 11.0 (IP Detection, Self-Destruct, Protocol v9.1)
// =========================================================

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <cstring>
#include <vector>
#include <atomic>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define CLOSE_SOCKET closesocket
    #define IS_VALIDSOCK(s) ((s) != INVALID_SOCKET)
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <ifaddrs.h> // REQUIRED FOR MAC IP
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET close
    #define IS_VALIDSOCK(s) ((s) >= 0)
#endif

using namespace std;

// --- CONFIG ---
const int WIN_W = 480;
const int WIN_H = 850;
const Color COL_BG = { 10, 10, 12, 255 };         
const Color COL_ACCENT = { 0, 255, 70, 255 };     
const Color COL_BUBBLE_ME = { 0, 80, 40, 255 };   
const Color COL_BUBBLE_YOU = { 20, 20, 20, 255 }; 
const Color COL_INPUT = { 25, 25, 30, 255 };      
const Color COL_LINK = { 0, 255, 70, 40 };        
const Color COL_DESTRUCT = { 200, 20, 20, 255 };  
const char* LOG_FILE = "ghost_log.txt";

Sound soundTing;
bool soundLoaded = false;

// --- IP DETECTION HELPER ---
string GetLocalIP() {
    string ip = "127.0.0.1";
    #ifdef _WIN32
        // Windows Logic
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) == 0) {
            char hostName[255];
            if (gethostname(hostName, sizeof(hostName)) == 0) {
                struct hostent* host = gethostbyname(hostName);
                if (host != nullptr) {
                    struct in_addr* addr = (struct in_addr*)host->h_addr_list[0];
                    if (addr != nullptr) ip = inet_ntoa(*addr);
                }
            }
            WSACleanup();
        }
    #else
        // Mac/Linux Logic
        struct ifaddrs *ifAddrStruct = NULL;
        struct ifaddrs *ifa = NULL;
        void *tmpAddrPtr = NULL;

        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) continue;
            if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                
                string ifName(ifa->ifa_name);
                string ipStr(addressBuffer);
                
                // en0 is typically Wi-Fi on Mac
                if (ifName == "en0" || ifName == "wlan0" || ifName == "eth0") {
                     ip = ipStr;
                     if (ip != "127.0.0.1") break; 
                }
            }
        }
        if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    #endif
    return ip;
}

string GetCurrentTimeStr() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M");
    return oss.str();
}

struct MessageNode {
    char name[32];
    char text[256];     
    char timestamp[16]; 
    bool isMine;
    float animScale; 
    float floatOffset; 
    float decryptTimer;
    
    bool isSelfDestruct;
    float lifeTime; 

    MessageNode* next;
    MessageNode* prev;
};

struct DataProjectile {
    Vector2 start, end;
    float progress;
    bool active;
};

struct MatrixRainDrop {
    float x, y;
    float speed;
    int len;
    char chars[20];
};

class ChatEngine {
private:
    std::mutex mtx;
public:
    MessageNode* head = nullptr;
    MessageNode* tail = nullptr;
    std::vector<DataProjectile> projectiles;
    std::vector<MatrixRainDrop> rain; 
    string remoteName = "WAITING...";
    bool triggerSound = false;
    float scrollOffset = 0.0f;
    bool needsScrollSnap = false;

    ChatEngine() {
        for(int i=0; i<30; i++) {
            MatrixRainDrop d;
            d.x = GetRandomValue(0, WIN_W);
            d.y = GetRandomValue(-100, WIN_H);
            d.speed = GetRandomValue(2, 6);
            d.len = GetRandomValue(5, 15);
            for(int j=0; j<20; j++) d.chars[j] = (char)GetRandomValue(33, 126);
            rain.push_back(d);
        }
    }
    
    void UpdateNodes(float dt) {
        std::lock_guard<std::mutex> lock(mtx);
        MessageNode* curr = head;
        while(curr) {
            MessageNode* nextNode = curr->next; 
            if (curr->isSelfDestruct) {
                curr->lifeTime -= dt;
                if (curr->lifeTime <= 0) {
                    if (curr->prev) curr->prev->next = curr->next;
                    if (curr->next) curr->next->prev = curr->prev;
                    if (curr == head) head = curr->next;
                    if (curr == tail) tail = curr->prev;
                    delete curr; 
                }
            }
            curr = nextNode;
        }
    }

    void ClearAll() {
        std::lock_guard<std::mutex> lock(mtx);
        MessageNode* curs = head;
        while(curs) { MessageNode* next=curs->next; delete curs; curs=next; }
        head=nullptr; tail=nullptr;
        remoteName = "WAITING...";
        std::ofstream ofs;
        ofs.open(LOG_FILE, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }

    void UpdateRain() {
        for(auto& r : rain) {
            r.y += r.speed;
            if(r.y > WIN_H) {
                r.y = -200;
                r.x = GetRandomValue(0, WIN_W);
            }
            if(GetRandomValue(0, 10) < 2) {
                 r.chars[GetRandomValue(0, r.len-1)] = (char)GetRandomValue(33, 126);
            }
        }
    }

    void SaveToLog(const string& user, const string& txt, bool destruct) {
        if(destruct) return;
        std::ofstream outfile;
        outfile.open(LOG_FILE, std::ios_base::app); 
        outfile << user << "|" << txt << "|" << GetCurrentTimeStr() << endl;
    }

    void InternalAdd(const string& user, const string& txt, const string& timeStr, bool mine, bool animate, bool destruct) {
        if (txt.length() == 0) return;
        MessageNode* n = new MessageNode();
        strncpy(n->name, user.c_str(), 31);
        strncpy(n->text, txt.c_str(), 255);
        strncpy(n->timestamp, timeStr.c_str(), 15);
        n->isMine = mine;
        n->animScale = animate ? 0.0f : 1.0f;       
        n->decryptTimer = animate ? 0.0f : 1.0f;    
        n->floatOffset = (float)(GetRandomValue(0, 360));
        n->isSelfDestruct = destruct;
        n->lifeTime = 10.0f; 
        
        n->next = nullptr;
        n->prev = tail;
        if (tail) tail->next = n;
        tail = n;
        if (!head) head = n;
        if (animate) {
             needsScrollSnap = true;
             if (!mine) triggerSound = true;
        }
    }

    void AddLog(const string& user, const string& txt, bool mine, bool destruct) {
        std::lock_guard<std::mutex> lock(mtx);
        if(!mine) remoteName = user; 
        SaveToLog(user, txt, destruct); 
        InternalAdd(user, txt, GetCurrentTimeStr(), mine, true, destruct);
    }
    
    void LoadHistory(const string& myName) {
        std::lock_guard<std::mutex> lock(mtx);
        if (head != nullptr) return; 

        std::ifstream infile(LOG_FILE);
        string line;
        while (std::getline(infile, line)) {
            size_t d1 = line.find('|');
            size_t d2 = line.rfind('|');
            if (d1!=string::npos && d2!=string::npos && d2 > d1) {
                string u = line.substr(0, d1);
                string m = line.substr(d1+1, d2-(d1+1));
                string t = line.substr(d2+1);
                bool mine = (u == myName); 
                InternalAdd(u, m, t, mine, false, false);
                if(!mine) remoteName = u; 
            }
        }
        needsScrollSnap = true;
    }
    
    void FireVisual(bool outgoing) {
        std::lock_guard<std::mutex> lock(mtx);
        Vector2 start = outgoing ? Vector2{(float)WIN_W/2, 120} : Vector2{(float)WIN_W/2, (float)WIN_H};
        Vector2 end = outgoing ? Vector2{(float)WIN_W/2, (float)WIN_H/2} : Vector2{(float)WIN_W/2, 120};
        projectiles.push_back({start, end, 0.0f, true});
    }

    std::mutex& GetLock() { return mtx; }
};

ChatEngine engine;
SocketType sock = INVALID_SOCKET;
std::atomic<bool> isConnected(false);
std::atomic<bool> isConnecting(false);
bool isHost = false;
bool isSandbox = false;
string myName = "Master AK"; 
bool selfDestructMode = false; 

void NetLoop() {
    char buf[4096]; 
    string pendingData = "";

    while (isConnected) {
        int bytes = recv(sock, buf, 4095, 0); 
        if (bytes > 0) {
            buf[bytes] = '\0';
            pendingData += string(buf);
            
            size_t pos = 0;
            while ((pos = pendingData.find('\n')) != string::npos) {
                string currentPacket = pendingData.substr(0, pos);
                pendingData.erase(0, pos + 1);

                size_t delim1 = currentPacket.find('|');
                if (delim1 != string::npos) {
                     string sender = currentPacket.substr(0, delim1);
                     string remainder = currentPacket.substr(delim1+1);
                     
                     size_t delim2 = remainder.find('|');
                     string msg = "";
                     bool isDestruct = false;

                     if (delim2 != string::npos) {
                         msg = remainder.substr(0, delim2);
                         string flag = remainder.substr(delim2+1);
                         if (flag == "1") isDestruct = true;
                     } else {
                         msg = remainder; 
                     }

                     if (msg.length() > 0) {
                         engine.AddLog(sender, msg, false, isDestruct);
                         engine.FireVisual(false);
                     }
                }
            }
        } else {
            isConnected = false; break;
        }
    }
}

bool InitSock() { 
    #ifdef _WIN32
    WSADATA w; return WSAStartup(MAKEWORD(2,2), &w) == 0;
    #else
    return true; 
    #endif
}
void KillSock() { if (IS_VALIDSOCK(sock)) CLOSE_SOCKET(sock); }

void HostAsync() {
    isConnecting = true; InitSock();
    SocketType l = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a = {0}; a.sin_family = AF_INET; a.sin_port = htons(9999); a.sin_addr.s_addr = INADDR_ANY;
    int o=1; setsockopt(l,SOL_SOCKET,SO_REUSEADDR,(char*)&o,sizeof(o));
    if(::bind(l,(sockaddr*)&a,sizeof(a))<0) { isConnecting=false; return; }
    listen(l,1); sock=accept(l,NULL,NULL); CLOSE_SOCKET(l);
    if(IS_VALIDSOCK(sock)) { isConnected=true; isHost=true; std::thread(NetLoop).detach(); }
    isConnecting=false;
}

void JoinAsync(string ip) {
    isConnecting=true; InitSock();
    sock=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in a={0}; a.sin_family=AF_INET; a.sin_port=htons(9999); inet_pton(AF_INET,ip.c_str(),&a.sin_addr);
    if(connect(sock,(sockaddr*)&a,sizeof(a))==0) { isConnected=true; isHost=false; std::thread(NetLoop).detach(); }
    isConnecting=false;
}

void Send(string s) {
    string flag = selfDestructMode ? "1" : "0";
    string raw = myName + "|" + s + "|" + flag + "\n";
    if(!isSandbox && isConnected) send(sock, raw.c_str(), raw.length(), 0);
    
    engine.AddLog(myName, s, true, selfDestructMode);
    engine.FireVisual(true);
    if(soundLoaded) PlaySound(soundTing);
}

Color LerpColor(Color a, Color b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

void DrawDottedLine(Vector2 start, Vector2 end, Color color) {
    float dist = Vector2Distance(start, end);
    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    for (float i = 0; i < dist; i += 15) {
        DrawCircleV(Vector2Add(start, Vector2Scale(dir, i)), 2, color);
    }
}

void DrawMatrixText(const char* realText, int x, int y, int fontSize, float progress, Color color) {
    if (progress >= 0.95f) { DrawText(realText, x, y, fontSize, color); return; }
    int len = strlen(realText);
    int revealCount = (int)(len * progress);
    string display = "";
    for(int i=0; i<len; i++) {
        if(i < revealCount) display += realText[i]; 
        else display += (char)GetRandomValue(33, 126); 
    }
    DrawText(display.c_str(), x, y, fontSize, color);
}

int main() {
    InitWindow(WIN_W, WIN_H, "Ghost-Talk // FINAL V11");
    InitAudioDevice();
    soundTing = LoadSound("assets/ting.wav");
    if(soundTing.frameCount > 0) soundLoaded = true;
    SetTargetFPS(60);
    
    remove(LOG_FILE); 
    
    // DETECT IP ON STARTUP
    string myIP = GetLocalIP();

    int screen = 0; 
    char ipBuf[32] = "127.0.0.1"; int ipLen = 9;
    char nameBuf[32] = "Master AK"; int nameLen = 9;
    char msgBuf[256] = {0}; int msgLen = 0;
    float time = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        time += dt;
        
        {
            engine.UpdateNodes(dt); 
            std::lock_guard<std::mutex> lock(engine.GetLock());
            if(engine.triggerSound) { if(soundLoaded) PlaySound(soundTing); engine.triggerSound=false; }
            for(auto& p : engine.projectiles) {
                if(p.active) { p.progress += 0.05f; if(p.progress >= 1.0f) p.active=false; }
            }
            if(screen < 2) engine.UpdateRain();
        }

        if(screen==0) { 
            int k=GetCharPressed();
            while(k>0) { if(k>=32&&k<=126&&nameLen<30){nameBuf[nameLen++]=(char)k;nameBuf[nameLen]='\0';} k=GetCharPressed(); }
            if(IsKeyPressed(KEY_BACKSPACE)&&nameLen>0) nameBuf[--nameLen]='\0';
            if(IsKeyPressed(KEY_ENTER) && nameLen>0) { 
                myName = string(nameBuf); 
                engine.LoadHistory(myName); 
                screen = 1; 
            }
        }
        else if(screen==1) { 
            if(isConnected) screen=2;
            if(!isConnecting) {
                int k=GetCharPressed();
                while(k>0) { if((k>=46&&k<=57)&&ipLen<15){ipBuf[ipLen++]=(char)k;ipBuf[ipLen]='\0';} k=GetCharPressed(); }
                if(IsKeyPressed(KEY_BACKSPACE)&&ipLen>0) ipBuf[--ipLen]='\0';
            }
        } 
        else if(screen==2) { 
            float wheel = GetMouseWheelMove();
            if (wheel != 0) engine.scrollOffset += wheel * 40.0f; 
            if (IsKeyDown(KEY_UP)) engine.scrollOffset += 10.0f;
            if (IsKeyDown(KEY_DOWN)) engine.scrollOffset -= 10.0f;
            
            if (IsKeyPressed(KEY_F5) || IsKeyPressed(KEY_DELETE)) engine.ClearAll();

            int k=GetCharPressed();
            while(k>0) { if(k>=32&&k<=126&&msgLen<250){msgBuf[msgLen++]=(char)k;msgBuf[msgLen]='\0';} k=GetCharPressed(); }
            if(IsKeyPressed(KEY_BACKSPACE)&&msgLen>0) msgBuf[--msgLen]='\0';
            if(IsKeyPressed(KEY_ENTER)&&msgLen>0) { Send(string(msgBuf)); msgLen=0; msgBuf[0]='\0'; }
        }

        BeginDrawing();
        ClearBackground(COL_BG);
        if(screen < 2) {
             for(auto& r : engine.rain) {
                 for(int i=0; i<r.len; i++) {
                     float a = 1.0f - ((float)i/r.len);
                     DrawTextCodepoint(GetFontDefault(), r.chars[i], {r.x, r.y - (i*15)}, 16, Fade(COL_ACCENT, a*0.3f));
                 }
             }
        }

        if(screen==0) {
            DrawText("GHOST-TALK", 40, 100, 40, COL_ACCENT);
            DrawText("IDENTITY PROTOCOL", 40, 300, 20, COL_ACCENT);
            DrawText("Enter Alias:", 40, 340, 20, GRAY);
            DrawRectangle(40, 370, 400, 50, COL_INPUT);
            DrawText(nameBuf, 50, 385, 20, WHITE);
            if ((int)(time*2)%2==0) DrawText("_", 50 + MeasureText(nameBuf, 20), 385, 20, COL_ACCENT);
        }
        else if(screen==1) {
            DrawText("POS: GHOST_MAIN", 40, 80, 10, GRAY);
            DrawText("GHOST-TALK", 40, 100, 40, COL_ACCENT);
            DrawText(("Agent: " + myName).c_str(), 40, 150, 20, WHITE);
            
            // DISPLAY IP HERE
            string ipDisp = "YOUR IP: " + myIP;
            DrawText(ipDisp.c_str(), 40, 275, 18, WHITE);
            
            Rectangle bHost = {40, 300, 400, 50};
            Color hCol = CheckCollisionPointRec(GetMousePosition(), bHost) ? WHITE : COL_ACCENT;
            DrawRectangleRounded(bHost, 0.2, 4, hCol);
            DrawText("HOST SECURE NET", 140, 315, 20, BLACK);
            if(!isConnecting && IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), bHost)) { isSandbox=false; std::thread(HostAsync).detach(); }

            Rectangle bJoin = {40, 400, 400, 50};
            Color jCol = CheckCollisionPointRec(GetMousePosition(), bJoin) ? GRAY : COL_INPUT;
            DrawRectangleRounded(bJoin, 0.2, 4, jCol);
            DrawText("JOIN SECURE NET", 140, 415, 20, WHITE);
            if(!isConnecting && IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), bJoin)) { isSandbox=false; std::thread(JoinAsync, string(ipBuf)).detach(); }
            DrawText(ipBuf, 50, 460, 20, WHITE);

            Rectangle bSand = {40, 550, 400, 50};
            DrawRectangleRoundedLines(bSand, 0.2, 4, 2, COL_ACCENT);
            DrawText("ENTER SANDBOX", 160, 565, 20, COL_ACCENT);
            if(!isConnecting && IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), bSand)) { isSandbox=true; isConnected=false; screen=2; }

            if(isConnecting) {
                float alpha = (sinf(time * 5.0f) + 1.0f) * 0.5f; 
                DrawText("ESTABLISHING UPLINK...", 60, 50, 28, Fade(COL_ACCENT, alpha));
                DrawRectangleLines(40, 40, 400, 50, Fade(COL_ACCENT, alpha));
            }
        } 
        else {
            DrawRectangle(0, 0, WIN_W, 60, {15, 15, 18, 255}); 
            string status = isSandbox ? "SANDBOX" : (myName + " <-> " + engine.remoteName);
            DrawText(status.c_str(), 60, 22, 18, COL_ACCENT);
            
            float blink = (sinf(time * 6.0f) + 1.0f) * 0.5f; 
            Color lightCol = (isConnected || isSandbox) ? GREEN : RED;
            DrawCircle(40, 30, 6, lightCol);
            DrawCircle(40, 30, 10 + (blink*5), Fade(lightCol, 0.3f)); 

            Rectangle bBack = {WIN_W-80, 10, 70, 40};
            DrawRectangleRounded(bBack, 0.3, 4, {200, 50, 50, 200});
            DrawText("EXIT", WIN_W-65, 20, 20, WHITE);
            if(IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), bBack)) {
                KillSock();
                isConnected = false; isConnecting = false; screen = 1;
            }
            
            Rectangle bClear = {WIN_W-160, 15, 60, 30};
            bool hoverClear = CheckCollisionPointRec(GetMousePosition(), bClear);
            DrawRectangleRounded(bClear, 0.3, 4, hoverClear ? RED : DARKGRAY);
            DrawText("CLEAR", WIN_W-152, 22, 16, WHITE);
            if(IsMouseButtonPressed(0) && hoverClear) engine.ClearAll();

            DrawRectangle(0, 60, WIN_W, 70, {18, 18, 22, 255});
            Rectangle inputRect = {20, 70, (float)WIN_W-40, 50};
            DrawRectangleRounded(inputRect, 0.3, 4, COL_INPUT);
            if (msgLen == 0) DrawText("Type encrypted message...", 35, 85, 20, GRAY);
            DrawText(msgBuf, 35, 85, 20, WHITE);
            if ((int)(time*2)%2==0) DrawText("_", 35 + MeasureText(msgBuf, 20), 85, 20, COL_ACCENT);

            Rectangle bDestruct = {(float)WIN_W-40-30, 80, 30, 30};
            Color dCol = selfDestructMode ? COL_DESTRUCT : (CheckCollisionPointRec(GetMousePosition(), bDestruct) ? GRAY : DARKGRAY);
            DrawRectangleRounded(bDestruct, 0.5, 4, dCol);
            DrawText("!", bDestruct.x+10, bDestruct.y+5, 20, WHITE);
            if(IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), bDestruct)) {
                selfDestructMode = !selfDestructMode;
            }

            BeginScissorMode(0, 130, WIN_W, WIN_H-130); 
            {
                std::lock_guard<std::mutex> lock(engine.GetLock());
                float totalH = 0;
                MessageNode* c = engine.head;
                while(c) { totalH += 80; c = c->next; }
                float viewH = WIN_H - 150;
                float minScroll = -(totalH - viewH);
                if (minScroll > 0) minScroll = 0;
                if (engine.needsScrollSnap) { engine.scrollOffset = minScroll; engine.needsScrollSnap = false; }
                if (engine.scrollOffset > 0) engine.scrollOffset = 0;
                if (engine.scrollOffset < minScroll) engine.scrollOffset = minScroll;
                
                c = engine.head;
                int yBase = 150 + (int)engine.scrollOffset;
                
                while(c) {
                    if(c->animScale < 1.0f) c->animScale += 0.15f;
                    if(c->animScale > 0.6f && c->decryptTimer < 1.0f) c->decryptTimer += 0.015f; 
                    float floatY = sinf(time * 2.0f + c->floatOffset) * 2.0f;
                    int y = yBase + floatY;
                    if (y > WIN_H || y < 100) { yBase += 80; c = c->next; continue; }
                    
                    if(c->next) {
                         int w1 = MeasureText(c->text, 18) + 40; int x1 = c->isMine ? (WIN_W - w1 - 20) : 20;
                         int w2 = MeasureText(c->next->text, 18) + 40; int x2 = c->next->isMine ? (WIN_W - w2 - 20) : 20;
                         DrawDottedLine({(float)x1+w1/2, (float)y+50}, {(float)x2+w2/2, (float)y+80+50}, COL_LINK);
                    }

                    int w = MeasureText(c->text, 18) + 40;
                    if (w < 120) w = 120;
                    int x = c->isMine ? (WIN_W - w - 20) : 20;
                    
                    Color bubbleBase = c->isMine ? COL_BUBBLE_ME : COL_BUBBLE_YOU;
                    if(c->isSelfDestruct) {
                        float flash = (sinf(time * 10.0f) + 1.0f) * 0.5f;
                        bubbleBase = LerpColor(COL_DESTRUCT, DARKGRAY, flash);
                    }

                    Rectangle r = {(float)x, (float)y, (float)w, 55};
                    DrawRectangleRounded({r.x, r.y+(27.5f-55*c->animScale/2), r.width, 55*c->animScale}, 0.3f, 4, bubbleBase);
                    
                    if(c->animScale > 0.8f) {
                        DrawText(c->name, x+15, y+5, 10, COL_ACCENT);
                        
                        if (c->isSelfDestruct) {
                            string timer = "[" + std::to_string((int)ceil(c->lifeTime)) + "s]";
                            DrawText(timer.c_str(), x+w-40, y+5, 10, RED);
                        } else {
                            DrawText(c->timestamp, x+w-40, y+5, 10, GRAY);
                        }

                        DrawMatrixText(c->text, x+15, y+25, 18, c->decryptTimer, WHITE);
                    }
                    yBase += 80;
                    c = c->next;
                }
                for(auto& p : engine.projectiles) {
                    if(p.active) {
                        Vector2 cur = Vector2Lerp(p.start, p.end, p.progress);
                        for(int i=1; i<=5; i++) {
                             Vector2 t = Vector2Lerp(p.start, p.end, p.progress - (i*0.02f));
                             DrawCircleV(t, 6-i, Fade(COL_ACCENT, 0.5f - (i*0.1f)));
                        }
                        DrawCircleV(cur, 6, WHITE);
                    }
                }
            }
            EndScissorMode();
        }
        EndDrawing();
    }
    if(soundLoaded) UnloadSound(soundTing);
    CloseAudioDevice(); KillSock(); CloseWindow();
    return 0;
}