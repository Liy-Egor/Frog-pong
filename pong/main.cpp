//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include "ctime"
#include "vector"
int currenttime = 0;
void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false);

POINT mouse;
struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������

HBITMAP ballBitmap;

// ������ ������ ����  
HBITMAP frogHbm;
float frogWidth;
float frogHeight;
int oldtime = 0;


const int slots_count = 6;

void loadFrog()
{
    frogHbm = (HBITMAP)LoadImageA(NULL, "frog.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    BITMAP bm;
    GetObject(frogHbm, (int)sizeof bm, &bm);
    frogWidth = bm.bmWidth;
    frogHeight = bm.bmHeight;
}


struct sprite {
    float x, y, width, height, rad, dx, dy, speed, time;
    HBITMAP hBitmap;//����� � ������� ������ 

    void loadBitmapWithNativeSize(const char* filename)
    {
        hBitmap = (HBITMAP)LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        BITMAP bm;
        GetObject(hBitmap, (int)sizeof bm, &bm);
        width = bm.bmWidth;
        height = bm.bmHeight;
    }

    void show()
    {
        ShowBitmap(window.context, x, y, width, height, hBitmap,true);
    }
};
sprite racket;//������� ������

struct enemy {

    sprite enemy_sprite;
    std::vector<sprite> bullet;
    int bullettime = 0;
    bool dead = true;
    int spawnTime = 0;
    int HPfrog = 3;           //rand()% 3-5

    void showBullet()
    {
        for (int i = 0; i < bullet.size(); i++)
        {
            ShowBitmap(window.context, bullet[i].x - bullet[i].rad, bullet[i].y - bullet[i].rad, 2 * bullet[i].rad, 2 * bullet[i].rad, bullet[i].hBitmap, true);
        }
    }

    void processBullet()
    {
        if (currenttime > bullettime + 5000)
        {
            //for (int i = 0; i < bullet.size(); i++)
            {
                sprite b;
                b.height = 40;
                b.width = 40;
                b.speed = rand()% (20-1)+4;
                b.x = enemy_sprite.x;
                b.y = enemy_sprite.y;
                b.dx = racket.x - b.x;
                b.dy = racket.y - b.y;

                float dvector = sqrt(b.dx * b.dx + b.dy * b.dy);
                b.dx = b.dx / dvector;
                b.dy = b.dy / dvector;
                b.rad = 20;
                b.hBitmap = ballBitmap;

                //game.action = true;

                bullet.push_back(b);
                bullettime = currenttime;
            }
        }

        for (int i = 0; i < bullet.size(); i++)
        {
            float margin = 0;

            if ((bullet[i].x > window.width - margin) || (bullet[i].x < margin) ||
                (bullet[i].y < margin))
            {
                bullet[i].speed = 0;
            }

             bullet[i].x += bullet[i].dx * bullet[i].speed;
            bullet[i].y += bullet[i].dy * bullet[i].speed;
        }

        for (int i = 0; i < bullet.size(); i++)
        {
            if (bullet[i].speed < .1)
            {
                bullet.erase(bullet.begin() + i);
            }
        }
    }
};


int location_number = 0;

struct portal_ 
{
    int x;
    int y;
    int w;
    int h;
    int destination;
    bool active;
};

struct location 
{

    std::vector<portal_> portal;
    
    
    HBITMAP back;
};

location map[10];
enemy frog[slots_count];

sprite ball;//�����
//�������

std::vector<sprite> bullet;


void spawnEnemy()
{
        for (int i = 0;i < slots_count;i++)
        {
            if (frog[i].dead)
            {

                if (currenttime > frog[i].spawnTime + 1000)
                {
                    sprite f;

                    f.x = (window.width / slots_count) * i;
                    f.y = 0;
                    f.width = frogWidth;
                    f.height = frogHeight;
                    f.hBitmap = frogHbm;

                    enemy e;
                    e.enemy_sprite = f;
                    e.dead = false;
                    e.HPfrog = rand() % 3 + 1;
                    frog[i] = e;
                  

                }
                break;

            }
        }

    
}





//c����� ����

void loadBitmap(const char* filename, HBITMAP& hbm)
{
    hbm = (HBITMAP)LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void ProcessMapsSwap() 
{
    auto portaL_count = map[location_number].portal.size();
    for (int i = 0; i < portaL_count; i++)
    {
        if (map[location_number].portal[i].x < racket.x and
            map[location_number].portal[i].y < racket.y and
            map[location_number].portal[i].y + map[location_number].portal[i].h > racket.y and
            map[location_number].portal[i].x + map[location_number].portal[i].w > racket.x)
        {
            location_number = map[location_number].portal[i].destination;
            racket.x = window.width / 4.;
            racket.y = window.height - (window.height - racket.y);
            return;
        }

    }
}

void InitGame()
{
    for (int i = 0;i < slots_count;i++)
    {
        frog[i].dead = true;
    }
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    //ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //frog.hBitmap = (HBITMAP)LoadImageA(NULL, "frog.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //frog.loadBitmapWithNativeSize("frog.bmp");
    loadFrog();


    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    //------------------------------------------------------

    racket.width = 30;
    racket.height = 30;
    racket.speed = 30;//�������� ����������� �������
    racket.x = window.width / 2.;//������� ���������� ����
    racket.y = window.height - racket.height;//���� ���� ���� ������ - �� ������ �������

    //enemy.x = racket.x;//� ���������� �������� ������ � �� �� ����� ��� � ������

    loadBitmap("ball.bmp", ballBitmap);

    game.score = 0;
    game.balls = 9;


    map[0].back = (HBITMAP)LoadImageA(NULL, "background_0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    map[0].portal.push_back({ 0,window.height - window.height / 4,window.width / 50, window.height / 4,1,true });

    map[1].back = (HBITMAP)LoadImageA(NULL, "background_1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    map[1].portal.push_back({ window.width - window.width / 50,window.height - window.height / 4,window.width / 50, window.height / 4,0,true });
    /*if (location_number == 1)
    {
        racket.x = window.width - (window.width / 50) - racket.width;
        racket.y = window.height - racket.height;
    }*/
    
    


}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

void ShowScore()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(165, 180, 130));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(bullet.size(), txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    //    _itoa_s(game.balls, txt, 10);
      //  TextOutA(window.context, 10, 100, "Balls", 5);
       // TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));

    DeleteObject(hFont);
}


float clickTimeOut = 100;
float clickTime = 0;

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;
    //if (GetAsyncKeyState(VK_UP)) racket.y -= racket.speed;
    //if (GetAsyncKeyState(VK_DOWN)) racket.y += racket.speed;
    clickTime = timeGetTime();
    
    if (GetAsyncKeyState(VK_LBUTTON) && clickTime > clickTimeOut)
    {
        sprite b;
        b.speed = 10;
        b.x = racket.x;
        b.y = racket.y;

        b.dx = mouse.x - b.x;
        b.dy = mouse.y - b.y;
        float dvector = sqrt(b.dx * b.dx + b.dy * b.dy);
        b.dx = b.dx / dvector;
        b.dy = b.dy / dvector;
        b.rad = 20;
        b.hBitmap = ballBitmap;

        game.action = true;

        bullet.push_back(b);

        ProcessSound("bounce1.wav");

        clickTimeOut = clickTime + 1000;


    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}





void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, map[location_number].back);//������ ���
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ������� ������

    

    
    
    for (int i = 0; i < slots_count; i++)
    {
        if (!frog[i].dead)
        {
            frog[i].enemy_sprite.show();
            frog[i].processBullet();
            frog[i].showBullet();
        }
    
    }


    for (int i = 0; i < bullet.size(); i++)
    {
        ShowBitmap(window.context, bullet[i].x - bullet[i].rad, bullet[i].y - bullet[i].rad, 2 * bullet[i].rad, 2 * bullet[i].rad, bullet[i].hBitmap, true);// �����
    }

    for (int i = 0; i < 20; i++)
    {
        //    ShowBitmap(window.context, ball[i].x - ball[i].rad, ball[i].y - ball[i].rad, 2 * ball[i].rad, 2 * ball[i].rad, ball[i].hBitmap, true);// �����
    }


}

void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
    racket.x = min(racket.x, window.width - racket.width / 2.);//���������� ��� ������� ����
}



void ProcessBall()
{
    if (game.action)
    {

        for (int i = 0; i < bullet.size(); i++)
        {
            float margin = 0;

            if ((bullet[i].x > window.width - margin) || (bullet[i].x < margin) ||
                (bullet[i].y < margin))
            {
                bullet[i].speed = 0;
            }

            //���� ���� � �������� ������ - ���������� �����
            bullet[i].x += bullet[i].dx * bullet[i].speed;
            bullet[i].y += bullet[i].dy * bullet[i].speed;
        }

       for (int j = 0; j < slots_count; j++)
       {
           for (int i = 0; i < bullet.size(); i++)
           {
               if (!bullet.empty())
               {
                   if (!frog[j].dead)
                   {

                       if (bullet[i].x < frog[j].enemy_sprite.x + frog[j].enemy_sprite.width and
                           bullet[i].y < frog[j].enemy_sprite.y + frog[j].enemy_sprite.height and
                           bullet[i].x > frog[j].enemy_sprite.x and
                           bullet[i].y > frog[j].enemy_sprite.y)
                       {
                           bullet.erase(bullet.begin() + i);
                           
                           frog[j].HPfrog -= 1;
                           if (frog[j].HPfrog < 0)
                           {
                               frog[j].dead = true;
                               frog[j].spawnTime = currenttime;
                           }

                           i--;
                           if (i < 0) break;

                       }
                   }
               }
               else
               {
                   break;
               }
           }
           
       }
        

         for (int i = 0; i < bullet.size(); i++)
         {
             if (bullet[i].speed < .1)
             {
                 bullet.erase(bullet.begin() + i);
             }
         }
    }
    else
    {
        for (int i = 0; i < bullet.size(); i++)
        {
            if (bullet[i].speed < .1)
            {
                bullet[i].x = racket.x;
            }
        }
    }
    if (game.action)
    {
        //std::vector<int> b = (ball[i])
        {
            //    ball[i].time
        }
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.hWnd, &r);

}

                            //float clickTime = 0;
float gravity = 30;      //float clickTimeOut = 100;
float jump = 0;
<<<<<<< HEAD
float maxjump = 20;
//bool doubleJump = false;

void ProcessHero()
{
    if (GetAsyncKeyState(VK_SPACE) && racket.y > (window.height - racket.height - 1))
=======
int isJump = false;
int doubleJump = false;

void ProcessHero()
{

    if (GetAsyncKeyState(VK_SPACE) && racket.y > window.height * 0.8 && isJump)
>>>>>>> 7f7521025322de2f54baabec411a0e6f8b9f81d9
    {

        jump += 10;
        isJump = true;
    }
   
    

<<<<<<< HEAD
    racket.y += gravity - jump;
    racket.y = min(window.height - racket.height, racket.y);
    
    jump *= .9;
    jump = max(jump, 0);
    //doubleJump = false;
    
}


const float dashDistance = 150;
float dash = 0;
bool wasShiftPressed = false; 

void ProcessDash() //�����
{
    bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (isShiftPressed && !wasShiftPressed) 
    {
        wasShiftPressed = true; 
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) 
        {
            racket.x -= dashDistance; 
        }
        else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) 
        {
            racket.x += dashDistance; 
        }
    }
    else if (!isShiftPressed)
    {
        wasShiftPressed = false;
    }
}


//float dash = 0;
//
//
//void ProcessDash()
//{
//    if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_LEFT))
//    {
//        dash = 50;
//        racket.x -= dash;
//        
//    }
//
//    if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_RIGHT))
//    {
//        dash = 50;
//        racket.x += dash;
//        
//    }
//}




=======
    if (GetAsyncKeyState(VK_SPACE) && racket.y > window.height / 2 && !isJump)
    {

        jump += 10;
        doubleJump = true;
    }

    if (jump > 0)
    {
        racket.y += gravity - jump;
        racket.y = min(window.height - racket.height, racket.y);
        jump *= 0.9;
        jump = max(jump, 0);
    }

    if (!isJump && !doubleJump)
    {
        isJump = false;
        doubleJump = false;
    }
}




//void ProcessHero()
//{
//float maxjump = window.height / 2;
//int jumpCounter = 1;
//    if (GetAsyncKeyState(VK_SPACE) && racket.y > maxjump && isJump && jumpCounter > 0)
//    {
//  
//        jump += 10;
//   
//    }   
//        else {
//            if (racket.y > 200) {
//            racket.y += gravity - jump;
//            racket.y = min(window.height - racket.height, racket.y);
//            jump *= 0.9;
//            jump = max(jump, 0);
//            }
//
//        }
//
//        isJump = true;
//    if (racket.y < window.height - racket.height) {
//      isJump = false;
//      jumpCounter = 0;
//    }
//    
//
//    
//
//
//}
>>>>>>> 7f7521025322de2f54baabec411a0e6f8b9f81d9
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    //mciSendString(TEXT("play ..\\Debug\\music1.mp3 repeat"), NULL, 0, NULL);
    //ShowCursor(NULL);
    oldtime = timeGetTime();
    

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        currenttime = timeGetTime();
        GetCursorPos(&mouse);
        ScreenToClient(window.hWnd, &mouse);

        ShowRacketAndBall();//������ ���, ������� � �����
        ShowScore();//������ ���� � �����
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

        ProcessInput();//����� ����������
        ProcessDash();//�����
        ProcessHero();//������ 
        LimitRacket();//���������, ����� ������� �� ������� �� �����
        ProcessBall();//���������� �����
        spawnEnemy();
        ProcessMapsSwap();
        // ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������


    }

}