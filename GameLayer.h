#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__


#include "cocos2d.h"
#include "SimpleAudioEngine.h"



USING_NS_CC;
using namespace CocosDenshion;

enum {
    kBackground,
    kMiddleground,
    kForeground
};

enum {
    kSpriteRocket,
    kSpritePlanet,
    kSpriteBoost,
    kSpriteStar,
    kSpriteBullet
};

typedef enum gamestates {
    kGameIntro,
    kGamePaused,
    kGamePlay,
    kGameOver
    
}GameState;

class Rocket;
class LineContainer;
class GameSprite;
class Bullet;

class GameLayer : public CCLayer {
    
	
    Rocket * _rocket;
    LineContainer * _lineContainer;
    
    CCSpriteBatchNode *_gameBatchNode;
    CCSpriteBatchNode *_gameBatchNode3;
    
    CCLabelBMFont * _scoreDisplay;
   
    
    GameSprite * _pauseBtn;
    CCSprite * _intro;
    CCSprite *_gameOver;
    CCSprite *_paused;
    CCSprite *_gun;
    
    CCParticleSystem * _star;
    CCParticleSystem * _jet;
    CCParticleSystem * _boom;
    CCParticleSystem * _comet;
    CCParticleSystem * _pickup;
    CCParticleSystem * _warp;
    
    CCArray * _planets;
    CCSize _screenSize;
    CCSize _backGroundSize;
    
    CCSprite * bgF ;
    CCSprite * bgS ;
    
    Bullet *_bullet;
    CCArray *_bullets;
    
    
    CCPoint _positionBgFirst;
    CCPoint _positionBgSec;

    GameState _state;
    
    bool _drawing;
    bool _running;

    std::vector<CCPoint> _grid;
    int _gridIndex;
    
    int _minLineLength;
    float _cometInterval;
    float _cometTimer;
    float _bulletDeltaTimer;
    
    int _indexBullet;
   
    
    void resetGame(void);
    void resetStar(void);
    void resetBullets(void);
    
    void startFighting(void);
    void fightingDone(CCNode* pSender);
    
    
    void killPlayer(void);
    
    void createGameScreen(void);
    void createParticles(void);
    void createStarGrid(void);
    void createActions(void);
    void scrollBackground(float dt);
	int _score;
	float _timeBetweenPickups;
    
public:
    ~GameLayer(void);
    
    // Method 'init' in cocos2d-x returns bool, instead of 'id' in cocos2d-iphone (an object pointer)
    virtual bool init();
    
    // there's no 'id' in cpp, so we recommend to return the class instance pointer
    static cocos2d::CCScene* scene();
    
    void update (float dt);
	
	virtual void ccTouchesBegan(CCSet* pTouches, CCEvent* event);
	virtual void ccTouchesMoved(CCSet* pTouches, CCEvent* event);
	virtual void ccTouchesEnded(CCSet* pTouches, CCEvent* event);
    
    // preprocessor macro for "static create()" constructor ( node() deprecated )
    CREATE_FUNC(GameLayer);
	

};

#endif // __GAMELAYER_H__
