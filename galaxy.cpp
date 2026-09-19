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

Color MyYellow = {229, 255, 0, 255};
Color MyOrange = {255, 167, 0, 255};
Color MyRed = {255, 0, 0, 255};


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

    void Update(float dt) {
        y -= 1000 * dt;
    }

    void Draw() {
        Color color = MyYellow;

        if (BULLET_DAMAGE >= 20)
            color = MyOrange;

        if (BULLET_DAMAGE == 30)
            color = MyRed;

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
        GetRandomValue(0, WIDTH - size),-100,size,size,GetRandomValue(100, 200),60) {}
};


class Moon : public SpaceObject {
public:
    Moon(Texture2D texture): Moon(texture, GetRandomValue(140,180)) {}

    Moon(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH - size), -100, size, size, GetRandomValue(100, 200), 150) {}
};


class Earth : public SpaceObject {
public:
    Earth(Texture2D texture) : Earth(texture, GetRandomValue(250, 300)) {}

    Earth(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH - size), -100, size, size, GetRandomValue(100, 200), 240) {}
};


class Saturn : public SpaceObject {
public:
    Saturn(Texture2D texture) : Saturn(texture, GetRandomValue(340,400)) {}

    Saturn(Texture2D texture, int size) : SpaceObject(texture,
         GetRandomValue(0, WIDTH - size), -100, size, size, GetRandomValue(100,180),400) {}
};


class Jupiter : public SpaceObject {
public:
    Jupiter(Texture2D texture) : Jupiter(texture, GetRandomValue(340, 400)) {}

    Jupiter(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH-size), -100, size, size, GetRandomValue(90,150),600) {}
};


class Sun : public SpaceObject {
public:
    Sun(Texture2D texture) : Sun(texture, GetRandomValue(380,440)) {}

    Sun(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH-size), -100, size, size, GetRandomValue(80,140), 690) {}
};


class BlackHole : public SpaceObject {
public:
    BlackHole(Texture2D texture) : BlackHole(texture, GetRandomValue(450,500)) {}

    BlackHole(Texture2D texture, int size) : SpaceObject(texture,
        GetRandomValue(0, WIDTH-size), -100, size, size, GetRandomValue(70,120), 750) {}
};


int main() {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(WIDTH, HEIGHT, "Galaxy Shooter");
    SetTargetFPS(120);

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

    Texture2D map = LoadTexture("assets/textures/galaxy3.png");
    Texture2D meteorTexture = LoadTexture("assets/textures/greymeteor.png");
    Texture2D moonTexture = LoadTexture("assets/textures/moon.png");
    Texture2D earthTexture = LoadTexture("assets/textures/earth.png");
    Texture2D saturnTexture = LoadTexture("assets/textures/saturn.png");
    Texture2D jupiterTexture = LoadTexture("assets/textures/jupiter.png");
    Texture2D sunTexture = LoadTexture("assets/textures/sun.png");
    Texture2D blackholeTexture = LoadTexture("assets/textures/blackhole.png");
    Texture2D explosionTexture = LoadTexture("assets/textures/explosionssprite.png");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
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
            bulletTimer = 0.13f;
        }

        if (moonTimer <= 0) {
            moons.push_back(Moon(moonTexture));
            moonTimer = 10.0f;
        }

        if (meteorTimer <= 0) {
            meteors.push_back(Meteor(meteorTexture));
            meteorTimer = 2.0f;
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
                        int missing = 30 - BULLET_DAMAGE;
                        BULLET_DAMAGE += std::min(6, missing);
                    }

                    break;
                }
            }
        }

        // destroying
        meteors.erase(
            remove_if(meteors.begin(), meteors.end(),[](const Meteor& meteor) {
                return meteor.IsDead();
            }),
            meteors.end()
        );

        moons.erase(
            remove_if(moons.begin(), moons.end(),[](const Moon& moon) {
                return moon.IsDead();
            }),
            moons.end()
        );

        earths.erase(
            remove_if(earths.begin(), earths.end(),[](const Earth& earth) {
                return earth.IsDead();
            }),
            earths.end()
        );

        saturns.erase(
            remove_if(saturns.begin(), saturns.end(),[](const Saturn& saturn) {
                return saturn.IsDead();
            }),
            saturns.end()
        );

        jupiters.erase(
            remove_if(jupiters.begin(), jupiters.end(),[](const Jupiter& jupiter) {
                return jupiter.IsDead();
            }),
            jupiters.end()
        );

        suns.erase(
            remove_if(suns.begin(), suns.end(),[](const Sun& sun) {
                return sun.IsDead();
            }),
            suns.end()
        );

        blackholes.erase(
            remove_if(blackholes.begin(), blackholes.end(),[](const BlackHole& blackhole) {
                return blackhole.IsDead();
            }),
            blackholes.end()
        );

        explosions.erase(
            remove_if(explosions.begin(), explosions.end(),[](const Explosion& explosion) {
                return explosion.finished;
            }),
            explosions.end()
        );

        BeginDrawing();

            ClearBackground(BLACK);
            DrawTexture(map,0,0,WHITE);
            DISTANCE += 333 * dt;

            ship.Draw();
            for (auto& moon : moons) moon.Draw();
            for (auto& bullet : bullets) bullet.Draw();
            for (auto& meteor : meteors) meteor.Draw();
            for (auto& earth : earths) earth.Draw();
            for (auto& saturn : saturns) saturn.Draw();
            for (auto& jupiter : jupiters) jupiter.Draw();
            for (auto& sun : suns) sun.Draw();
            for (auto& blackhole : blackholes) blackhole.Draw();

            for (auto& explosion : explosions) explosion.Draw();

            DrawText(TextFormat("Distance: %d meters",(int)DISTANCE),20,20,30,RED);
            DrawText(TextFormat("DMG: %d",BULLET_DAMAGE),20,60,30, RED);

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

    CloseWindow();
    return 0;
}