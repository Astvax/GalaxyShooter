#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

                /*--------------------------------------------------------------------------+
                |                       Galaxy Shooter C++ Игра без ИИ                       |
                |                         Начало проекта: 09.09.2026                        |
                +--------------------------------------------------------------------------*/

const int WIDTH = 1000;
const int HEIGHT = 1080;

float DISTANCE = 0;
int BULLET_DAMAGE = 10;
const int MAX_DAMAGE = 30;
int lives = 3;

Color MyYellow = {229, 255, 0, 255};
Color MyOrange = {255, 167, 0, 255};
Color MyRed = {255, 0, 0, 255};

enum class GameState { Playing, GameOver, Victory};
GameState state = GameState::Playing;


class SpaceShip {
public:
    Texture2D texture;
    float x, y;
    const float width = 150, height = 150;

    SpaceShip() : texture(LoadTexture("assets/textures/galaxyship.png")),x(WIDTH / 2.0f), y(HEIGHT - 150.0f) {};

    ~SpaceShip() {
        UnloadTexture(texture);
    }

    void Update() {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            x -= 8;
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)){
            x += 8;
        }
        x = clamp(x, width / 2, WIDTH - width / 2);
    }

    void Draw() {
        DrawTexturePro(texture,
            {0, 0, (float)texture.width, (float)texture.height},
            {x - width / 2, y, width, height}, {0, 0}, 0, WHITE);
    }
};


class Bullet {
public:
    float x, y;
    bool active = true;
    bool levelSoundPlayed = false;
    void Update(float dt) {
        y -= 1000 * dt;
    }

void Draw(Sound& levelSound) {
    static int lastDamage = BULLET_DAMAGE;

    Color color = GREEN;

    if (BULLET_DAMAGE >= 20) {
        color = PURPLE;

        if (lastDamage < 20)
            PlaySound(levelSound);
    }

    if (BULLET_DAMAGE >= 30) {
        color = MyRed;

        if (lastDamage < 30)
            PlaySound(levelSound);
    }

    lastDamage = BULLET_DAMAGE;

    DrawRectangle((int)x - 2, (int)y - 10, 6, 20, color);
    }
};


class SpaceObject {
public:
    Texture2D texture;
    float x, y;
    float width, height;
    float speed;
    int health;

    SpaceObject(Texture2D texture, float x, float y,
            float width, float height, float speed, int health) :
            texture(texture), x(x),y(y),width(width),height(height),speed(speed),health(health) {}

    virtual void Update(float dt) {
        y += speed * dt;
    }

    virtual void Draw() {
        DrawTexturePro(texture,{0, 0, (float)texture.width, (float)texture.height},{x, y, width, height},{0, 0},0,WHITE);
    }

    Rectangle GetCollisionRect() const {
        return {x, y, width, height};
    }

    bool CheckHit(const Bullet& bullet) const {
        Rectangle bulletRect = {bullet.x - 2,bullet.y - 10,4,20};
        return CheckCollisionRecs(GetCollisionRect(),bulletRect);
    }

    void TakeDamage(int damage) {
        health -= damage;
    }

    bool IsDead() const {
        return health <= 0;
    }

    virtual ~SpaceObject() = default;
};

class Explosion {
public:
    Texture2D texture;
    float x, y;
    float width, height;
    float timer = 0;
    int currentFrame = 0;
    int frameCount = 4;
    float frameTime = 0.07f;
    bool finished = false;

    Explosion(Texture2D texture,float x,float y,float width,float height) : texture(texture),x(x),y(y),width(width),height(height) {}

    void Update(float dt) {
        timer += dt;
        if (timer >= frameTime) {
            timer -= frameTime;
            currentFrame++;
            if (currentFrame >= frameCount)
                finished = true;
        }
    }

    void Draw() {
        if (finished) return;
        float frameWidth = (float)texture.width / frameCount;
        Rectangle source = {currentFrame * frameWidth,0,frameWidth,(float)texture.height};
        Rectangle destination = {x,y,width,height};
        DrawTexturePro(texture,source,destination,{0,0},0,WHITE);
    }
};


void CreateExplosion(const SpaceObject& object,Texture2D explosionTexture,vector<Explosion>& explosions) {
    explosions.push_back(Explosion(explosionTexture,object.x,object.y,object.width,object.height));
}


class Meteor : public SpaceObject {
public:
    Meteor(Texture2D texture): Meteor(texture, GetRandomValue(80, 120)) {}

    Meteor(Texture2D texture, int size): SpaceObject(texture,
        GetRandomValue(0, WIDTH - size),-100,size,size,GetRandomValue(100, 170),60) {}
};


class Moon : public SpaceObject {
public:
    Moon(Texture2D texture): Moon(texture, GetRandomValue(140,180)) {}

    Moon(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH - size), -100, size, size, GetRandomValue(100, 170), 150) {}
};


class Earth : public SpaceObject {
public:
    Earth(Texture2D texture) : Earth(texture, GetRandomValue(250, 300)) {}

    Earth(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH - size), -100, size, size, GetRandomValue(100, 160), 240) {}
};


class Saturn : public SpaceObject {
public:
    Saturn(Texture2D texture) : Saturn(texture, GetRandomValue(340,400)) {}

    Saturn(Texture2D texture, int size) : SpaceObject(texture,
         GetRandomValue(0, WIDTH - size), -100, size, size, GetRandomValue(100,150),400) {}
};


class Jupiter : public SpaceObject {
public:
    Jupiter(Texture2D texture) : Jupiter(texture, GetRandomValue(340, 400)) {}

    Jupiter(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH-size), -100, size, size, GetRandomValue(90,140),600) {}
};


class Sun : public SpaceObject {
public:
    Sun(Texture2D texture) : Sun(texture, GetRandomValue(380,440)) {}

    Sun(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH-size), -100, size, size, GetRandomValue(80,130), 690) {}
};


class BlackHole : public SpaceObject {
public:
    BlackHole(Texture2D texture) : BlackHole(texture, GetRandomValue(450,500)) {}

    BlackHole(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH-size), -100, size, size, GetRandomValue(70,120), 750) {}
};


template<typename T>
void CleanupObjects(vector<T>& objects, Sound& ouch) {
    for (auto& obj : objects) {
        if (!obj.IsDead() && obj.y > HEIGHT) {
            lives--;
            PlaySound(ouch);
        }
    }
    objects.erase(remove_if(objects.begin(), objects.end(),[](const T& obj) {
        return obj.IsDead() || obj.y > HEIGHT;
    }), objects.end());
}


int main() {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(WIDTH, HEIGHT, "Galaxy Shooter");
    InitAudioDevice();  
    SetTargetFPS(165);

    SpaceShip ship;
    vector<Moon> moons;
    vector<Bullet> bullets;
    vector<Meteor> meteors;
    vector<Earth> earths;
    vector<Saturn> saturns;
    vector<Jupiter> jupiters;
    vector<Sun> suns;
    vector<BlackHole> blackholes;
    vector<Explosion> explosions;

    float moonTimer = 10;
    float meteorTimer = 0;
    float bulletTimer = 0;
    float earthTimer = 25;
    float saturnTimer = 40;
    float jupiterTimer = 60;
    float sunTimer = 78;
    float blackholeTimer = 100;

    Texture2D map = LoadTexture("assets/textures/galaxy4.png");
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(map,0,0,WHITE);
    EndDrawing();
    Texture2D meteorTexture = LoadTexture("assets/textures/greymeteor.png");
    Texture2D moonTexture = LoadTexture("assets/textures/moon.png");
    Texture2D earthTexture = LoadTexture("assets/textures/earth.png");
    Texture2D saturnTexture = LoadTexture("assets/textures/saturn.png");
    Texture2D jupiterTexture = LoadTexture("assets/textures/jupiter.png");
    Texture2D sunTexture = LoadTexture("assets/textures/sun.png");
    Texture2D blackholeTexture = LoadTexture("assets/textures/blackhole.png");
    Texture2D explosionTexture = LoadTexture("assets/textures/explosionssprite.png");
    Texture2D heartTexture = LoadTexture("assets/textures/heart.png");
    Texture2D greyHeartTexture = LoadTexture("assets/textures/greyheart.png");

    Sound shootSound = LoadSound("assets/audio/shoot.wav");
    Sound explosionSound = LoadSound("assets/audio/explosion.wav");
    Sound levelSound = LoadSound("assets/audio/levelup.wav");
    Sound cosmoSound = LoadSound("assets/audio/background.mp3");
    Sound victorySound = LoadSound("assets/audio/victory.wav");
    Sound loseSound = LoadSound("assets/audio/lose.wav");
    Sound ouchSound = LoadSound("assets/audio/ouch.wav");

    PlaySound(cosmoSound);

    SetSoundVolume(shootSound, 0.02f);
    SetSoundVolume(explosionSound, 0.3f);
    SetSoundVolume(cosmoSound, 0.3f);

    auto ResetGame = [&]() {
        lives = 3;
        DISTANCE = 0;
        BULLET_DAMAGE = 10;
        ship.x = WIDTH / 2.0f;
        bullets.clear();
        meteors.clear();
        moons.clear();
        earths.clear();
        saturns.clear();
        jupiters.clear();
        suns.clear();
        blackholes.clear();
        explosions.clear();
        moonTimer = 10;
        meteorTimer = 0;
        bulletTimer = 0;
        earthTimer = 25;
        saturnTimer = 40;
        jupiterTimer = 60;
        sunTimer = 78;
        blackholeTimer = 100;
        state = GameState::Playing;
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (state == GameState::GameOver || state == GameState::Victory) {
            if (IsKeyPressed(KEY_SPACE)) ResetGame();
        }

        if (state == GameState::Playing) {
        ship.Update();

            // timers
            moonTimer -= dt;
            meteorTimer -= dt;
            bulletTimer -= dt;
            earthTimer -= dt;
            saturnTimer -= dt;
            jupiterTimer -= dt;
            sunTimer -= dt;
            blackholeTimer -= dt;

            if (bulletTimer <= 0) {
                bullets.push_back({ship.x, ship.y});
                PlaySound(shootSound);
                bulletTimer = 0.13f;
            }

            if (moonTimer <= 0) {
                moons.push_back(Moon(moonTexture));
                moonTimer = 10.0f;
            }

            if (meteorTimer <= 0) {
                meteors.push_back(Meteor(meteorTexture));
                meteorTimer = 2.5f;
            }

            if (earthTimer <= 0) {
                earths.push_back(Earth(earthTexture));
                earthTimer = 25.0f;
            }

            if (saturnTimer <= 0) {
                saturns.push_back(Saturn(saturnTexture));
                saturnTimer = 40.0f;
            }

            if (jupiterTimer <= 0) {
                jupiters.push_back(Jupiter(jupiterTexture));
                jupiterTimer = 60.0f;
            }

            if (sunTimer <= 0) {
                suns.push_back(Sun(sunTexture));
                sunTimer = 78.0f;
            }

            if (blackholeTimer <= 0) {
                blackholes.push_back(BlackHole(blackholeTexture));
                blackholeTimer = 100.0f;
            }

            // initialization
            for (auto& bullet : bullets) bullet.Update(dt);
            bullets.erase(remove_if(bullets.begin(), bullets.end(),[](const Bullet& bullet) {
                return !bullet.active || bullet.y < -20;
            }),bullets.end());

            for (auto& meteor : meteors) {
                meteor.Update(dt);
            }

            for (auto& moon : moons) {
                moon.Update(dt);
            }

            for (auto& earth : earths) {
                earth.Update(dt);
            }

            for (auto& saturn : saturns) {
                saturn.Update(dt);
            }

            for (auto& jupiter : jupiters) {
                jupiter.Update(dt);
            }

            for (auto& sun : suns) {
                sun.Update(dt);
            }

            for (auto& blackhole : blackholes) {
                blackhole.Update(dt);
            }

            for (auto& explosion : explosions) {
                explosion.Update(dt);
            }

            // collision
            for (auto& bullet : bullets) {

                if (!bullet.active) continue;

                for (auto& meteor : meteors) {
                    if (meteor.CheckHit(bullet)) {
                        meteor.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (meteor.IsDead()) {
                            CreateExplosion(meteor,explosionTexture,explosions);
                            PlaySound(explosionSound);
                        }

                        break;
                    }
                }

                if (!bullet.active) continue;

                for (auto& moon : moons) {
                    if (moon.CheckHit(bullet)) {
                        moon.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (moon.IsDead()) {
                            CreateExplosion(moon,explosionTexture,explosions);
                            PlaySound(explosionSound);
                            int missing = 30 - BULLET_DAMAGE;
                            BULLET_DAMAGE += std::min(1, missing);
                        }

                        break;
                    }
                }

                if (!bullet.active) continue;

                for (auto& earth : earths) {
                    if (earth.CheckHit(bullet)) {
                        earth.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (earth.IsDead()) {
                            CreateExplosion(earth,explosionTexture,explosions);
                            PlaySound(explosionSound);
                            int missing = 30 - BULLET_DAMAGE;
                            BULLET_DAMAGE += std::min(2, missing);
                        }

                        break;
                    }
                }

                if (!bullet.active) continue;

                for (auto& saturn : saturns) {
                    if (saturn.CheckHit(bullet)) {
                        saturn.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (saturn.IsDead()) {
                            CreateExplosion(saturn,explosionTexture,explosions);
                            PlaySound(explosionSound);
                            int missing = 30 - BULLET_DAMAGE;
                            BULLET_DAMAGE += std::min(3, missing);
                        }

                        break;
                    }
                }

                if (!bullet.active) continue;

                for (auto& jupiter : jupiters) {
                    if (jupiter.CheckHit(bullet)) {
                        jupiter.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (jupiter.IsDead()) {
                            CreateExplosion(jupiter,explosionTexture,explosions);
                            PlaySound(explosionSound);
                            int missing = 30 - BULLET_DAMAGE;
                            BULLET_DAMAGE += std::min(4, missing);
                        }

                        break;
                    }
                }

                if (!bullet.active) continue;

                for (auto& sun : suns) {
                    if (sun.CheckHit(bullet)) {
                        sun.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (sun.IsDead()) {
                            CreateExplosion(sun,explosionTexture,explosions);
                            PlaySound(explosionSound);
                            int missing = 30 - BULLET_DAMAGE;
                            BULLET_DAMAGE += std::min(5, missing);
                        }

                        break;
                    }
                }

                if (!bullet.active) continue;

                for (auto& blackhole : blackholes) {
                    if (blackhole.CheckHit(bullet)) {
                        blackhole.TakeDamage(BULLET_DAMAGE);
                        bullet.active = false;

                        if (blackhole.IsDead()) {
                            CreateExplosion(blackhole,explosionTexture,explosions);
                            PlaySound(explosionSound);
                            int missing = 30 - BULLET_DAMAGE;
                            BULLET_DAMAGE += std::min(6, missing);
                        }

                        break;
                    }
                }
            }

            // destroying
            CleanupObjects(meteors,ouchSound);
            CleanupObjects(moons,ouchSound);
            CleanupObjects(earths,ouchSound);
            CleanupObjects(saturns,ouchSound);
            CleanupObjects(jupiters,ouchSound);
            CleanupObjects(suns,ouchSound);
            CleanupObjects(blackholes,ouchSound);

            explosions.erase(
                remove_if(explosions.begin(), explosions.end(),[](const Explosion& explosion) {
                    return explosion.finished;
                }),
                explosions.end()
            );

            if (lives <= 0) {
                PlaySound(loseSound);
                state = GameState::GameOver;
            }
            
            if (state == GameState::Playing && DISTANCE >= 100000) {
                state = GameState::Victory;
                PlaySound(victorySound);
            }
        }
        

        BeginDrawing();

            ClearBackground(BLACK);
            DrawTexture(map,0,0,WHITE);
            if (state == GameState::Playing) DISTANCE += 333 * dt;

            ship.Draw();
            for (auto& moon : moons) moon.Draw();
            for (auto& bullet : bullets) bullet.Draw(levelSound);
            for (auto& meteor : meteors) meteor.Draw();
            for (auto& earth : earths) earth.Draw();
            for (auto& saturn : saturns) saturn.Draw();
            for (auto& jupiter : jupiters) jupiter.Draw();
            for (auto& sun : suns) sun.Draw();
            for (auto& blackhole : blackholes) blackhole.Draw();
            for (auto& explosion : explosions) explosion.Draw();

            DrawText(TextFormat("Distance: %d meters",(int)DISTANCE),20,20,30,RED);
            DrawText(TextFormat("DMG: %d",BULLET_DAMAGE),20,60,30, RED);

            for (int i = 0; i < 3; i++) {
                Texture2D tex = (i < lives) ? heartTexture : greyHeartTexture;
                DrawTextureEx(tex, {-57.0f + i * 50.0f, 20.0f}, 0, 0.15f, WHITE);
            }

            if (state == GameState::GameOver) {
                DrawRectangle(0, 0, WIDTH, HEIGHT, {0, 0, 0, 180});
                DrawText("GAME OVER", WIDTH/2 - 150, HEIGHT/2 - 50, 50, RED);
                DrawText("Press SPACE to restart", WIDTH/2 - 150, HEIGHT/2 + 20, 25, WHITE);
            }

            if (state == GameState::Victory) {
                DrawRectangle(0, 0, WIDTH, HEIGHT, {0, 0, 0, 180});
                DrawText("VICTORY!", WIDTH/2 - 150, HEIGHT/2 - 50, 50, GREEN);
                DrawText("Press SPACE to restart", WIDTH/2 - 150, HEIGHT/2 + 20, 25, WHITE);
            }

        EndDrawing();
    }
    
    UnloadTexture(map);
    UnloadTexture(meteorTexture);
    UnloadTexture(moonTexture);
    UnloadTexture(earthTexture);
    UnloadTexture(saturnTexture);
    UnloadTexture(jupiterTexture);
    UnloadTexture(sunTexture);
    UnloadTexture(blackholeTexture);
    UnloadTexture(explosionTexture);
    UnloadTexture(heartTexture);
    UnloadTexture(greyHeartTexture);

    UnloadSound(shootSound);
    UnloadSound(explosionSound);
    UnloadSound(levelSound);
    UnloadSound(cosmoSound);

    CloseWindow();
    return 0;
}