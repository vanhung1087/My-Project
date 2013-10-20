/*
 
 Background music:
 8bit Dungeon Level by Kevin MacLeod (incompetech.com)
 
 */

#include "GameLayer.h"
#include "SimpleAudioEngine.h"
#include "Rocket.h"
#include "LineContainer.h"
#include "Bullet.h"


USING_NS_CC;

using namespace CocosDenshion;


GameLayer::~GameLayer () {
    
    CC_SAFE_RELEASE(_planets);
    CC_SAFE_RELEASE(_bullets);

}

CCScene* GameLayer::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    GameLayer *layer = GameLayer::create();
    
    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameLayer::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
	//init game values
	_screenSize = CCDirector::sharedDirector()->getWinSize();
    _drawing = false;
    _minLineLength = _screenSize.width * 0.07f;
    _state = kGameIntro;
    
    createGameScreen();
    
    createParticles();
    
    createStarGrid();
    
    //listen for touches
    this->setTouchEnabled(true);
    
    //create main loop
    this->schedule(schedule_selector(GameLayer::update));
    
    
    
    return true;
}
//Tran Van Hung
void GameLayer::update (float dt) {
    
    if (!_running) return;
    
    if (_lineContainer->getLineType() != LINE_NONE) {
        _lineContainer->setTip (_rocket->getPosition() );
    }
    //track collision with sides
    if (_rocket->collidedWithSides()) {
        _lineContainer->setLineType ( LINE_NONE );
    }
    _rocket->update(dt);
    
    scrollBackground(dt);
    
    //update jet particle so it follows rocket
    if (!_jet->isActive()) _jet->resetSystem();
    
    _jet->setRotation(_rocket->getRotation());
    _jet->setPosition(_rocket->getPosition());
    
     _cometTimer += dt;
    if (_bulletDeltaTimer > 0.1f)
    {
        startFighting();
        _bulletDeltaTimer= 0;
    }
    _bulletDeltaTimer += dt;
    
    float newY;
    if (_cometTimer > _cometInterval) {
        _cometTimer = 0;
        if (_comet->isVisible() == false) {
            _comet->setPositionX(0);
            newY =
            (float)rand()/((float)RAND_MAX/_screenSize.height *
                           0.6f) + _screenSize.height * 0.2f;
            if (newY > _screenSize.height * 0.9f)
                newY = _screenSize.height * 0.9f;
            _comet->setPositionY(newY);
            _comet->setVisible(true);
            _comet->resetSystem();
        }
    }
    if (_comet->isVisible()) {
        //collision with comet
        if (pow(_comet->getPositionX() - _rocket->getPositionX(), 2)
          + pow(_comet->getPositionY() - _rocket->getPositionY(), 2)
            <= pow (_rocket->getRadius() , 2)) {
            
            if (_rocket->isVisible()) killPlayer();
        }
        _comet->setPositionX(_comet->getPositionX() + 50 * dt);
        if (_comet->getPositionX() > _screenSize.width * 1.5f) {
            _comet->stopSystem();
            _comet->setVisible(false);
        }
    }
    
    _lineContainer->update(dt);
  //  _rocket->setOpacity(0);
    
//    int count = _planets->count();
//    GameSprite * planet;
//    for (int i = 0; i < count; i++) {
//        planet = (GameSprite *) _planets->objectAtIndex(i);
//        if (pow(planet->getPositionX() - _rocket->getPositionX(), 2)
//          + pow(planet->getPositionY() - _rocket->getPositionY(), 2)
//            <=   pow (_rocket->getRadius() * 0.8f + planet->getRadius()
//                      * 0.65f, 2)) {
//                
//                if (_rocket->isVisible()) killPlayer();
//                break; }
//    }
    
    if (pow(_star->getPositionX() - _rocket->getPositionX(), 2) + pow(_star->getPositionY() - _rocket->getPositionY(), 2)
        <= pow (_rocket->getRadius() * 1.2f, 2))
    {
        _pickup->setPosition(_star->getPosition() );
        _pickup->resetSystem();
        
        if (_lineContainer->getEnergy() + 0.25f < 1) {
            _lineContainer->setEnergy(_lineContainer->getEnergy() +
                                      0.25f);
        
        } else {
            _lineContainer->setEnergy(1.0);
        }
        _rocket->setSpeed(_rocket->getSpeed() + 2);
        _lineContainer->setEnergyDecrement(0.001f);
        
        SimpleAudioEngine::sharedEngine()->playEffect("pickup.wav");
        resetStar();
        int points = 100 - _timeBetweenPickups;
        if (points < 0) points = 0;
        _score += points;
        CCString * value = CCString::createWithFormat("%i",
                                                      _score);
        _scoreDisplay->setString(value->getCString());
        _timeBetweenPickups = 0;
    }
    
    
    _timeBetweenPickups += dt;
    if (_lineContainer->getEnergy() == 0) {
        if (_rocket->isVisible()) killPlayer();
    }
    
    _scoreDisplay->setPosition(ccp(_screenSize.width - _scoreDisplay->boundingBox().size.width , _screenSize.height * 0.96f));

}

void GameLayer::startFighting(void)
{
    _indexBullet++;
    if (_indexBullet == _bullets->count()) _indexBullet=0;
    CCParticleSystem *b = (CCParticleSystem *) _bullets->objectAtIndex(_indexBullet);
    
    int bulletX = _rocket->getPositionX() - 40;
    int bulletY = _rocket->getPositionY();
    
    int bulletTargetX = _rocket->getPositionX() - 40;
    int bulletTargetY = _screenSize.height + b->boundingBox().size.height * 0.5f;
    
       
       
    b->stopAllActions();
    b->setPosition(ccp(bulletX, bulletY));
    if (!b->isActive()) b->resetSystem();
    int s = bulletTargetY - bulletY ;
    float time = (s * 1.0f) / 1000;


    CCActionInterval*  _bulletFighting = CCMoveTo::create(time, ccp(bulletTargetX, bulletTargetY));
    CCFiniteTimeAction*  sequence = CCSequence::create(_bulletFighting,
                                                       CCCallFuncN::create
                                                       (this, callfuncN_selector(GameLayer::fightingDone)),NULL);
    b->setVisible(true);
    b->runAction(sequence);
    
}
void GameLayer::fightingDone(CCNode* pSender)
{
    pSender->setVisible(false);
    if (pSender->getTag() == kSpriteBullet)
    {
        CCParticleSystem *b = (CCParticleSystem *) pSender;
        b->stopSystem();
    }
    
}

//Tran Van Hung
void GameLayer::ccTouchesBegan(CCSet* pTouches, CCEvent* event) {
	
    if (!_running) return;
    
	CCTouch *touch = (CCTouch *)pTouches->anyObject();
    if (touch) {
	    
	    CCPoint tap = touch->getLocation();
        
    	//track if tapping on ship
		float dx = _rocket->getPositionX() - tap.x;
		float dy = _rocket->getPositionY() - tap.y;
        
		if (dx * dx + dy * dy <= pow(_rocket->getRadius(), 2) ) {
			_lineContainer->setLineType ( LINE_NONE );
			_rocket->setRotationOrientation ( ROTATE_NONE );
			_drawing = true;
		}
	}
    
}
//Tran Van Hung
void GameLayer::ccTouchesMoved(CCSet* pTouches, CCEvent* event) {
    
    if (!_running) return;
	
	if (_drawing) {
        
        CCTouch *touch = (CCTouch *)pTouches->anyObject();
        if(touch ) {
            CCPoint tap = touch->getLocation();
            _rocket ->setPositionX(tap.x);
            _rocket ->setPositionY(tap.y);
//            float dx = _rocket->getPositionX() - tap.x;
//            float dy = _rocket->getPositionY() - tap.y;
//            if (dx * dx + dy * dy > pow (_minLineLength, 2)) {
//                _rocket->select(true);
//                _lineContainer->setPivot ( tap );
//                _lineContainer->setLineType ( LINE_TEMP );
//            } else {
//                _rocket->select(false);
//                _lineContainer->setLineType ( LINE_NONE );
//            }
        }
    }
    
	
}
//Tran Van Hung
void GameLayer::ccTouchesEnded(CCSet* pTouches, CCEvent* event) {
    
    if (_state == kGameIntro) {
        
        _intro->setVisible(false);
        _pauseBtn->setVisible(true);
        _state = kGamePlay;
        resetGame();
        return;
        
    } else if (_state == kGamePaused) {
        
        _pauseBtn->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName ("btn_pause_off.png"));
        _paused->setVisible(false);
        _state = kGamePlay;
        _running = true;
        return;
        
    } else if (_state == kGameOver) {
        
        _gameOver->setVisible(false);
        _pauseBtn->setVisible(true);
        _state = kGamePlay;
        resetGame();
        return;
        
    }
    
    if (!_running) return;
    
    CCTouch *touch = (CCTouch *)pTouches->anyObject();
    
    if(touch) {
        
        CCPoint tap = touch->getLocation();
        
        if (_pauseBtn->boundingBox().containsPoint(tap)) {
            _paused->setVisible(true);
            _state = kGamePaused;
            _pauseBtn->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName ("btn_pause_on.png"));
            _running = false;
            return;
        }
        
        //track if tapping on ship
        _drawing = false;
        _rocket->select(false);
                            
                
    }

}

void GameLayer::resetGame () {

    _rocket->setPosition(ccp(_screenSize.width * 0.5f, _screenSize.height * 0.1f));
    _rocket->setOpacity(255);
    _rocket->setVisible(true);
    _rocket->reset();
    
    _cometInterval = 4;
    _cometTimer = 0;
    _timeBetweenPickups = 0.0;
    
    _score = 0;
    char szValue[100] = {0};
    sprintf(szValue, "%i", _score);
    _scoreDisplay->setString(szValue);
    
    _lineContainer->reset();
    
    //shuffle grid cells
    //std::random_shuffle(_grid.begin(), _grid.end());
    _gridIndex = 0;
    
    resetStar();
    
    _warp->stopSystem();
    
    _running = true;
    _indexBullet = -1;
    
    SimpleAudioEngine::sharedEngine()->playBackgroundMusic("background.mp3", true);
    SimpleAudioEngine::sharedEngine()->stopAllEffects();
    //SimpleAudioEngine::sharedEngine()->playEffect("rocket.wav", true);
    
       
}
void GameLayer::resetBullets(void)
{
    _bullet->setPosition( ccp(_rocket->getPositionX() - 40, _rocket->getPositionY() ) );
    
}
void GameLayer::resetStar() {
    CCPoint position = _grid[_gridIndex];
    _gridIndex++;
    if (_gridIndex == _grid.size()) _gridIndex = 0;
    //reset star particles
    _star->setPosition(position);
    _star->setVisible(true);
    _star->resetSystem();
}


//Tran Van Hung
void GameLayer::killPlayer() {

    SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->stopAllEffects();
    SimpleAudioEngine::sharedEngine()->playEffect("shipBoom.wav");
    //update our particle systems, _boom and _jet
    _boom->setPosition(_rocket->getPosition());
    _boom->resetSystem();
    _rocket->setVisible(false);
    _jet->stopSystem();
    _lineContainer->setLineType ( LINE_NONE );
    _running = false;
    _state = kGameOver;
    _gameOver->setVisible(true);
    _pauseBtn->setVisible(false);
    
    int count = _bullets->count();
    
    for (int i = 0 ; i < count; i++) {
        
        Bullet * b = (Bullet *) _bullets ->objectAtIndex(i);
        b->setPosition( ccp(_rocket->getPositionX() - 40, _rocket->getPositionY() ) );
        b->setFighting(false);
        b->setVisible(false);

    }

}

void GameLayer::scrollBackground(float dt)
{
//    if (bgF == NULL || bgS == NULL) return;
    float delta = dt * 50;
    _positionBgFirst.y -= delta;
    _positionBgSec.y -= delta;

   
    
    if (_positionBgFirst.y < -_backGroundSize.height * 0.5f)
    {
        _positionBgFirst.y = _screenSize.height + _screenSize.height * 0.5f;
    }

    if (_positionBgSec.y < -_backGroundSize.height * 0.5f)
    {
        _positionBgSec.y = _screenSize.height + _screenSize.height * 0.5f;
    }
//
    bgF->setPositionY(_positionBgFirst.y);
    bgS->setPositionY(_positionBgSec.y);

}

void GameLayer::createGameScreen () {
    
    _positionBgFirst = ccp(_screenSize.width * 0.5f, _screenSize.height * 0.5f);
    _positionBgSec = ccp(_screenSize.width * 0.5f , _screenSize.height + _screenSize.height * 0.5f);

    bgF = CCSprite::create("bg.png");
    bgS = CCSprite::create("bg.png");
    
    _backGroundSize = bgF->boundingBox().size;

    bgF->setPosition(_positionBgFirst);
    bgS->setPosition(_positionBgSec);
    
   
    
    this->addChild(bgF, kBackground);
    this->addChild(bgS, kBackground);
    
   
    
    
    
       CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("spaceship.plist");
    _gameBatchNode3 = CCSpriteBatchNode::create("spaceship.png", 100);
    this->addChild(_gameBatchNode3, kForeground);

    
    
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("sprite_sheet.plist");
    _gameBatchNode = CCSpriteBatchNode::create("sprite_sheet.png", 100);
    this->addChild(_gameBatchNode, kForeground);
    
    _rocket = Rocket::create();
    _rocket->setPosition(ccp(_screenSize.width * 0.5f, _screenSize.height * 0.1f));
   

    _bullet = Bullet::create();
    _bullet->setPosition( ccp(_rocket->getPositionX() - 40, _rocket->getPositionY() ) );
    _gameBatchNode3->addChild(_bullet, kForeground, kSpriteBullet);
   
    
    //add planets
   // GameSprite * planet;
    _planets = CCArray::create();
    _planets->retain();
    
    _bullets = CCArray::createWithCapacity(20);
    _bullets ->retain();
    CCParticleSystem *b ;
    for (int i = 0; i < 20; i++) {
        b = CCParticleSystemQuad::create("jet.plist");
        //b->setSourcePosition( ccp(0,-_rocket->getRadius() * 0.5f) );
        b->setPosition( ccp(_rocket->getPositionX() - 40, _rocket->getPositionY() ) );
        b->setAngle(-90);
        b->stopSystem();

       
       // _gameBatchNode3->addChild(b, kForeground, kSpriteBullet);
       this->addChild(b, kForeground, kSpriteBullet);
        _bullets->addObject(b);
    }
    _indexBullet = -1;
     _gameBatchNode3->addChild(_rocket, kForeground, kSpriteRocket);
    
//    planet = GameSprite::createWithFrameName("planet_1.png");
//    planet->setPosition(ccp(_screenSize.width * 0.25f,
//                            _screenSize.height * 0.8f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
//    
//    planet = GameSprite::createWithFrameName("planet_2.png");
//    planet->setPosition(ccp(_screenSize.width * 0.8f,
//                            _screenSize.height * 0.45f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
//    
//    planet = GameSprite::createWithFrameName("planet_3.png");
//    planet->setPosition(ccp(_screenSize.width * 0.75f,
//                            _screenSize.height * 0.8f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
//    
//    planet = GameSprite::createWithFrameName("planet_4.png");
//    planet->setPosition(ccp(_screenSize.width * 0.5f,
//                            _screenSize.height * 0.5f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
//    
//    planet = GameSprite::createWithFrameName("planet_5.png");
//    planet->setPosition(ccp(_screenSize.width * 0.18f,
//                            _screenSize.height * 0.45f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
//    
//    planet = GameSprite::createWithFrameName("planet_6.png");
//    planet->setPosition(ccp(_screenSize.width * 0.8f,
//                            _screenSize.height * 0.15f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
//    
//    planet = GameSprite::createWithFrameName("planet_7.png");
//    planet->setPosition(ccp(_screenSize.width * 0.18f,
//                            _screenSize.height * 0.1f));
//    _gameBatchNode->addChild(planet, kBackground, kSpritePlanet);
//    _planets->addObject(planet);
    
    CCSprite * scoreLabel = CCSprite::createWithSpriteFrameName("label_score.png");
    scoreLabel->setPosition(ccp(_screenSize.width * 0.4f, _screenSize.height * 0.96));
    _gameBatchNode->addChild(scoreLabel, kBackground);
    
    _scoreDisplay = CCLabelBMFont::create("0", "font.fnt", _screenSize.width * 0.5f, kCCTextAlignmentLeft);
    _scoreDisplay->setAnchorPoint(ccp(0,0.5));
    _scoreDisplay->setPosition(ccp(_screenSize.width - _scoreDisplay->boundingBox().size.width, _screenSize.height * 0.96f));
    this->addChild(_scoreDisplay, kBackground);
    
        
    _pauseBtn = GameSprite::createWithFrameName("btn_pause_off.png");
    _pauseBtn->setVisible(false);
    _pauseBtn->setPosition(ccp(_screenSize.width * 0.05f, _screenSize.height * 0.96f));
    _gameBatchNode->addChild(_pauseBtn, kBackground);
    
    
    _intro = CCSprite::createWithSpriteFrameName("logo.png");
    _intro->setPosition(ccp(_screenSize.width * 0.5f, _screenSize.height * 0.55f));
    CCSprite *play = CCSprite::createWithSpriteFrameName("label_play.png");
    play->setPosition(ccp(_intro->boundingBox().size.width * 0.5f, -_intro->boundingBox().size.height * 0.5f));
    _intro->addChild(play);
    _gameBatchNode->addChild(_intro, kForeground);
    
    _gameOver = CCSprite::createWithSpriteFrameName("gameOver.png");
    _gameOver->setPosition(ccp(_screenSize.width * 0.5f, _screenSize.height * 0.55f));
    _gameOver->setVisible(false);
    _gameBatchNode->addChild(_gameOver, kForeground);
    
    _paused = CCSprite::createWithSpriteFrameName("label_paused.png");
    _paused->setPosition(ccp(_screenSize.width * 0.5f, _screenSize.height * 0.55f));
    _paused->setVisible(false);
    _gameBatchNode->addChild(_paused, kForeground);
    
    _lineContainer = LineContainer::create();
    this->addChild( _lineContainer, kForeground );
}

//Tran Van Hung
void GameLayer::createParticles() {

    _jet = CCParticleSystemQuad::create("jet.plist");
    _jet->setSourcePosition( ccp(0,-_rocket->getRadius() * 0.5f) );
    _jet->setAngle(-90);
    _jet->stopSystem();
    
    this->addChild(_jet, kBackground);
    _boom = CCParticleSystemQuad::create("boom.plist");
    _boom->stopSystem();
    this->addChild(_boom, kForeground);
    
    _comet = CCParticleSystemQuad::create("comet.plist");
    _comet->stopSystem();
    _comet->setPosition(ccp(0, _screenSize.height * 0.6f));
    _comet->setVisible(false);
    this->addChild(_comet, kForeground);
    
    _pickup = CCParticleSystemQuad::create("plink.plist");
    _pickup->stopSystem();
    this->addChild(_pickup, kMiddleground);
    
    _warp = CCParticleSystemQuad::create("warp.plist");
    _warp->setPosition(_rocket->getPosition());
    this->addChild(_warp, kBackground);
    
    _star = CCParticleSystemQuad::create("star.plist");
    _star->stopSystem();
    _star->setVisible(false);
    this->addChild(_star, kBackground, kSpriteStar);

}

void GameLayer::createStarGrid() {
    //create grid
    float gridFrame = _screenSize.width * 0.1f;
    int tile = 32;
    int rows = (_screenSize.height - 4 * gridFrame)/tile;
    int cols = (_screenSize.width  - 2 * gridFrame)/tile;
    
    int count = _planets->count();
    GameSprite * planet;
    CCPoint cell;
    bool overlaps;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            cell = ccp(gridFrame + c * tile, 2 * gridFrame + r * tile);
            overlaps = false;
            for (int j = 0; j < count; j++) {
                planet = (GameSprite *) _planets->objectAtIndex(j);
                if (pow(planet->getPositionX() - cell.x, 2) + pow(planet->getPositionY() - cell.y, 2) <= pow(planet->getRadius() * 1.2f, 2)) {
                    overlaps = true;
                }
            }
            if (!overlaps) _grid.push_back(cell);
        }
    }
    CCLOG("POSSIBLE STARS: %i", _grid.size());
    
}