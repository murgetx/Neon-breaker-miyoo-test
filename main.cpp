#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>

namespace {
constexpr int SCREEN_W = 640, SCREEN_H = 480;
constexpr float FIELD_LEFT = 24.0f, FIELD_RIGHT = 616.0f, FIELD_TOP = 52.0f, FIELD_BOTTOM = 456.0f;
struct SaveData { int magic = 0x4E425431; int bestScore = 0; };
struct Game {
    float paddleX = SCREEN_W / 2.0f, ballX = SCREEN_W / 2.0f, ballY = 330.0f;
    float ballVX = 170.0f, ballVY = -210.0f;
    bool launched = true, paused = false;
    int score = 0, bricksLeft = 24;
};
void color(SDL_Renderer* r, Uint8 a, Uint8 b, Uint8 c) { SDL_SetRenderDrawColor(r, a, b, c, 255); }
void bar(SDL_Renderer* r, int x, int y, int w, int h, Uint8 a, Uint8 b, Uint8 c) {
    SDL_Rect q{x,y,w,h}; color(r,a,b,c); SDL_RenderFillRect(r,&q);
}
void saveScore(const std::string& path, int score) {
    SaveData d; d.bestScore = score; std::ofstream f(path,std::ios::binary|std::ios::trunc);
    if (f) f.write(reinterpret_cast<const char*>(&d),sizeof(d));
}
int loadScore(const std::string& path) {
    SaveData d; std::ifstream f(path,std::ios::binary);
    return f && f.read(reinterpret_cast<char*>(&d),sizeof(d)) && d.magic==0x4E425431 ? std::max(0,d.bestScore) : 0;
}
}
int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER)!=0) {
        std::fprintf(stderr,"SDL_Init: %s\n",SDL_GetError()); return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
    SDL_Window* w=SDL_CreateWindow("NEON BREAKER SDL2 TEST",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_W,SCREEN_H,SDL_WINDOW_SHOWN);
    if(!w){std::fprintf(stderr,"SDL window: %s\n",SDL_GetError());SDL_Quit();return 1;}
    SDL_Renderer* r=SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED);
    if(!r) r=SDL_CreateRenderer(w,-1,SDL_RENDERER_SOFTWARE);
    if(!r){std::fprintf(stderr,"SDL renderer: %s\n",SDL_GetError());SDL_DestroyWindow(w);SDL_Quit();return 1;}
    SDL_Joystick* joy=nullptr;
    if(SDL_NumJoysticks()>0) joy=SDL_JoystickOpen(0);
    char* pref=SDL_GetPrefPath("NEON_BREAKER","MiyooPortTest");
    std::string savePath=pref?std::string(pref)+"test_save.dat":"test_save.dat";
    if(pref) SDL_free(pref);
    int best=loadScore(savePath); Game g; bool run=true;
    Uint32 last=SDL_GetTicks(); float fpsTime=0; int fpsCount=0,fps=0;
    while(run){
        Uint32 now=SDL_GetTicks(); float dt=std::min((now-last)/1000.0f,0.05f); last=now;
        bool left=false,right=false,action=false; SDL_Event e;
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) run=false;
            if(e.type==SDL_KEYDOWN&&!e.key.repeat){
                if(e.key.keysym.sym==SDLK_ESCAPE||e.key.keysym.sym==SDLK_p) g.paused=!g.paused;
                if(e.key.keysym.sym==SDLK_SPACE||e.key.keysym.sym==SDLK_RETURN) action=true;
                if(e.key.keysym.sym==SDLK_q) run=false;
            }
            if(e.type==SDL_CONTROLLERBUTTONDOWN){
                if(e.cbutton.button==SDL_CONTROLLER_BUTTON_START) g.paused=!g.paused;
                if(e.cbutton.button==SDL_CONTROLLER_BUTTON_A) action=true;
                if(e.cbutton.button==SDL_CONTROLLER_BUTTON_BACK) run=false;
            }
            if(e.type==SDL_JOYBUTTONDOWN&&!SDL_IsGameController(0)){
                if(e.jbutton.button==0) action=true;
                if(e.jbutton.button==7) g.paused=!g.paused;
                if(e.jbutton.button==6) run=false;
            }
        }
        const Uint8* k=SDL_GetKeyboardState(nullptr);
        left=k[SDL_SCANCODE_LEFT]||k[SDL_SCANCODE_A]; right=k[SDL_SCANCODE_RIGHT]||k[SDL_SCANCODE_D];
        if(joy&&SDL_JoystickGetNumAxes(joy)>0){Sint16 a=SDL_JoystickGetAxis(joy,0);if(a< -9000)left=true;if(a>9000)right=true;}
        if(joy&&SDL_JoystickGetNumHats(joy)>0){Uint8 h=SDL_JoystickGetHat(joy,0);if(h&SDL_HAT_LEFT)left=true;if(h&SDL_HAT_RIGHT)right=true;}
        if(action&&!g.paused)g.launched=true;
        if(!g.paused){
            if(left)g.paddleX-=390*dt;if(right)g.paddleX+=390*dt;
            g.paddleX=std::clamp(g.paddleX,FIELD_LEFT+42,FIELD_RIGHT-42);
            if(g.launched){
                g.ballX+=g.ballVX*dt;g.ballY+=g.ballVY*dt;
                if(g.ballX<FIELD_LEFT+7||g.ballX>FIELD_RIGHT-7)g.ballVX*=-1;
                if(g.ballY<FIELD_TOP+7)g.ballVY=std::abs(g.ballVY);
                SDL_Rect p{int(g.paddleX-42),424,84,12},b{int(g.ballX-7),int(g.ballY-7),14,14};
                if(g.ballVY>0&&SDL_HasIntersection(&p,&b)){
                    float off=(g.ballX-g.paddleX)/42;g.ballVX=off*250;g.ballVY=-std::max(170.0f,std::abs(g.ballVY));g.ballY=410;g.score+=10;
                    if(g.score>best){best=g.score;saveScore(savePath,best);}
                }
                if(g.ballY>FIELD_BOTTOM){g.ballX=g.paddleX;g.ballY=400;g.ballVX=170;g.ballVY=-210;g.launched=false;}
            }else{g.ballX=g.paddleX;g.ballY=400;}
        }
        color(r,8,10,22);SDL_RenderClear(r);bar(r,0,0,SCREEN_W,34,18,22,42);bar(r,14,42,SCREEN_W-28,SCREEN_H-56,18,22,42);bar(r,18,46,SCREEN_W-36,2,80,220,255);
        bar(r,28,12,120,8,255,80,180);bar(r,190,12,std::min(180,g.score/2+2),8,80,220,255);bar(r,450,12,150,8,110,255,140);
        for(int i=0;i<g.bricksLeft;i++){int c=i%8,row=i/8;SDL_Rect b{42+c*70,82+row*28,62,18};color(r,Uint8(80+row*45),Uint8(210-c*12),255);SDL_RenderFillRect(r,&b);color(r,220,245,255);SDL_RenderDrawRect(r,&b);}
        SDL_Rect p{int(g.paddleX-42),424,84,12};color(r,255,80,180);SDL_RenderFillRect(r,&p);
        SDL_Rect b{int(g.ballX-7),int(g.ballY-7),14,14};color(r,255,245,190);SDL_RenderFillRect(r,&b);
        if(!g.launched||g.paused){bar(r,170,250,300,3,255,80,180);bar(r,220,266,200,3,80,220,255);bar(r,170,282,300,3,255,80,180);}
        fpsTime+=dt;if(++fpsCount&&fpsTime>=1){fps=fpsCount;fpsCount=0;fpsTime=0;}
        for(int i=0;i<std::min(12,fps/10);i++)bar(r,520+i*7,452,5,10,110,255,140);
        SDL_RenderPresent(r);SDL_Delay(1);
    }
    if(joy)SDL_JoystickClose(joy);SDL_DestroyRenderer(r);SDL_DestroyWindow(w);SDL_Quit();return 0;
}
