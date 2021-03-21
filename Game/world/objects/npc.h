#pragma once

#include "graphics/mesh/pose.h"
#include "graphics/mesh/animation.h"
#include "graphics/meshobjects.h"
#include "graphics/mdlvisual.h"
#include "game/gametime.h"
#include "game/movealgo.h"
#include "game/inventory.h"
#include "game/fightalgo.h"
#include "game/perceptionmsg.h"
#include "game/gamescript.h"
#include "physics/dynamicworld.h"
#include "world/aiqueue.h"
#include "world/fplock.h"
#include "world/waypath.h"

#include <cstdint>
#include <string>
#include <deque>

#include <daedalus/DaedalusVM.h>

class Interactive;
class WayPoint;

class Npc final {
  public:
    enum ProcessPolicy : uint8_t {
      Player,
      AiNormal,
      AiFar,
      AiFar2
      };

    using JumpStatus = MoveAlgo::JumpStatus;

    enum PercType : uint8_t {
      PERC_ASSESSPLAYER       = 1,
      PERC_ASSESSENEMY        = 2,
      PERC_ASSESSFIGHTER      = 3,
      PERC_ASSESSBODY         = 4,
      PERC_ASSESSITEM         = 5,
      PERC_ASSESSMURDER       = 6,
      PERC_ASSESSDEFEAT       = 7,
      PERC_ASSESSDAMAGE       = 8,
      PERC_ASSESSOTHERSDAMAGE = 9,
      PERC_ASSESSTHREAT       = 10,
      PERC_ASSESSREMOVEWEAPON = 11,
      PERC_OBSERVEINTRUDER    = 12,
      PERC_ASSESSFIGHTSOUND   = 13,
      PERC_ASSESSQUIETSOUND   = 14,
      PERC_ASSESSWARN         = 15,
      PERC_CATCHTHIEF         = 16,
      PERC_ASSESSTHEFT        = 17,
      PERC_ASSESSCALL         = 18,
      PERC_ASSESSTALK         = 19,
      PERC_ASSESSGIVENITEM    = 20,
      PERC_ASSESSFAKEGUILD    = 21,
      PERC_MOVEMOB            = 22,
      PERC_MOVENPC            = 23,
      PERC_DRAWWEAPON         = 24,
      PERC_OBSERVESUSPECT     = 25,
      PERC_NPCCOMMAND         = 26,
      PERC_ASSESSMAGIC        = 27,
      PERC_ASSESSSTOPMAGIC    = 28,
      PERC_ASSESSCASTER       = 29,
      PERC_ASSESSSURPRISE     = 30,
      PERC_ASSESSENTERROOM    = 31,
      PERC_ASSESSUSEMOB       = 32,
      PERC_Count
      };

    enum Talent : uint8_t {
      TALENT_UNKNOWN            = 0,
      TALENT_1H                 = 1,
      TALENT_2H                 = 2,
      TALENT_BOW                = 3,
      TALENT_CROSSBOW           = 4,
      TALENT_PICKLOCK           = 5,
      TALENT_MAGE               = 7,
      TALENT_SNEAK              = 8,
      TALENT_REGENERATE         = 9,
      TALENT_FIREMASTER         = 10,
      TALENT_ACROBAT            = 11,
      TALENT_PICKPOCKET         = 12,
      TALENT_SMITH              = 13,
      TALENT_RUNES              = 14,
      TALENT_ALCHEMY            = 15,
      TALENT_TAKEANIMALTROPHY   = 16,
      TALENT_FOREIGNLANGUAGE    = 17,
      TALENT_WISPDETECTOR       = 18,
      TALENT_C                  = 19,
      TALENT_D                  = 20,
      TALENT_E                  = 21,

      TALENT_MAX_G1             = 12,
      TALENT_MAX_G2             = 22
      };

    enum Attribute : uint8_t {
      ATR_HITPOINTS      = 0,
      ATR_HITPOINTSMAX   = 1,
      ATR_MANA           = 2,
      ATR_MANAMAX        = 3,
      ATR_STRENGTH       = 4,
      ATR_DEXTERITY      = 5,
      ATR_REGENERATEHP   = 6,
      ATR_REGENERATEMANA = 7,
      ATR_MAX
      };

    enum SpellCode : int32_t {
      SPL_DONTINVEST                  = 0,
      SPL_RECEIVEINVEST               = 1,
      SPL_SENDCAST                    = 2,
      SPL_SENDSTOP                    = 3,
      SPL_NEXTLEVEL                   = 4,
      SPL_STATUS_CANINVEST_NO_MANADEC = 8,
      SPL_FORCEINVEST		              = 1 << 16
      };

    enum Protection : uint8_t {
      PROT_BARRIER = 0,
      PROT_BLUNT   = 1,
      PROT_EDGE    = 2,
      PROT_FIRE    = 3,
      PROT_FLY     = 4,
      PROT_MAGIC   = 5,
      PROT_POINT   = 6,
      PROT_FALL    = 7,
      PROT_MAX     = 8
      };

    enum class State : uint32_t {
      INVALID     = 0,
      ANSWER      = 1,
      DEAD        = 2,
      UNCONSCIOUS = 3,
      FADEAWAY    = 4,
      FOLLOW      = 5,
      Count       = 6
      };

    enum GoToHint : uint8_t {
      GT_No,
      GT_Way,
      GT_NextFp,
      GT_Enemy,
      GT_Item,
      GT_Point,
      };

    enum HitSound : uint8_t {
      HS_NoSound = 0,
      HS_Dead    = 1
      };

    enum CastState : uint8_t {
      CS_NoCast      = 0,
      CS_Finalize    = 1,

      CS_Invest_0    = 16,
      CS_Invest_1    = 17,
      CS_Invest_2    = 19,
      CS_Invest_3    = 20,
      CS_Invest_4    = 21,
      CS_Invest_5    = 22,
      CS_Invest_6    = 23,
      CS_Invest_Last = 31,
      CS_Cast_0      = 32,
      CS_Cast_1      = 33,
      CS_Cast_2      = 34,
      CS_Cast_3      = 35,
      CS_Cast_Last   = 47,
      };

    using Anim = AnimationSolver::Anim;

    Npc(World &owner, size_t instance, const Daedalus::ZString& waypoint);
    Npc(World &owner, Serialize& fin);
    Npc(const Npc&)=delete;
    ~Npc();

    void       save(Serialize& fout);
    void       load(Serialize& fout);
    void       postValidate();

    bool       setPosition (float x,float y,float z);
    bool       setPosition (const Tempest::Vec3& pos);
    Tempest::Vec3 getPosition () const;
    void       setDirection(float x,float y,float z);
    void       setDirection(float rotation);
    void       setDirectionY(float rotation);
    void       setRunAngle  (float angle);

    auto       position()   const -> Tempest::Vec3;
    auto       cameraBone() const -> Tempest::Vec3;
    float      collisionRadius() const;
    float      rotation() const;
    float      rotationRad() const;
    float      rotationY() const;
    float      rotationYRad() const;
    float      runAngle() const { return runAng; }
    Bounds     bounds() const;

    void       stopDlgAnim();
    void       clearSpeed();
    bool       resetPositionToTA();

    void       setProcessPolicy(ProcessPolicy t);
    auto       processPolicy() const -> ProcessPolicy { return aiPolicy; }

    bool       isPlayer() const;
    void       setWalkMode(WalkBit m);
    auto       walkMode() const { return wlkMode; }
    void       tick(uint64_t dt);
    bool       startClimb(JumpStatus jump);

    auto       world() -> World&;

    float      translateY() const;
    float      centerY() const;
    Npc*       lookAtTarget() const;
    auto       portalName() -> const char*;

    float      qDistTo(float x,float y,float z) const;
    float      qDistTo(const WayPoint* p) const;
    float      qDistTo(const Npc& p) const;
    float      qDistTo(const Interactive& p) const;

    void       updateAnimation();
    void       updateTransform();

    const char*displayName() const;
    auto       displayPosition() const -> Tempest::Vec3;
    void       setVisual    (const char *visual);
    void       setVisual    (const Skeleton *visual);
    bool       hasOverlay   (const char*     sk) const;
    bool       hasOverlay   (const Skeleton* sk) const;
    void       addOverlay   (const char*     sk, uint64_t time);
    void       addOverlay   (const Skeleton *sk, uint64_t time);
    void       delOverlay   (const char*     sk);
    void       delOverlay   (const Skeleton *sk);

    bool       toogleTorch();

    void       setVisualBody (int32_t headTexNr,int32_t teethTexNr,
                              int32_t bodyVer,int32_t bodyColor,
                              const std::string& body,const std::string& head);
    void       updateArmour  ();
    void       setSword      (MeshObjects::Mesh&& sword);
    void       setRangeWeapon(MeshObjects::Mesh&& bow);
    void       setMagicWeapon(Effect&& spell);
    void       setSlotItem   (MeshObjects::Mesh&& itm,const char* slot);
    void       setStateItem  (MeshObjects::Mesh&& itm,const char* slot);
    void       setAmmoItem   (MeshObjects::Mesh&& itm,const char* slot);
    void       clearSlotItem (const char* slot);
    void       setPhysic     (DynamicWorld::NpcItem&& item);
    void       setFatness    (float f);
    void       setScale      (float x,float y,float z);

    bool       setAnim(Anim a);
    auto       setAnimAngGet(Anim a,bool noInterupt) -> const Animation::Sequence*;
    auto       setAnimAngGet(Anim a, bool noInterupt, uint8_t comb) -> const Animation::Sequence*;
    void       setAnimRotate(int rot);
    bool       setAnimItem(const char* scheme, int state);
    void       stopAnim(const std::string& ani);
    bool       stopItemStateAnim();
    bool       isFinishingMove() const;

    auto       animMoveSpeed(uint64_t dt) const -> Tempest::Vec3;

    bool       isJumpAnim() const;
    bool       isFlyAnim() const;
    bool       isFaling() const;
    bool       isSlide() const;
    bool       isInAir() const;
    bool       isStanding() const;
    bool       isSwim() const;
    bool       isDive() const;
    bool       isCasting() const;

    void       setTalentSkill(Talent t,int32_t lvl);
    int32_t    talentSkill(Talent t) const;

    void       setTalentValue(Talent t,int32_t lvl);
    int32_t    talentValue(Talent t) const;
    int32_t    hitChanse(Talent t) const;

    void       setRefuseTalk(uint64_t milis);
    bool       isRefuseTalk() const;

    int32_t    mageCycle() const;
    bool       canSneak() const;
    int32_t    attribute (Attribute a) const;
    void       changeAttribute(Attribute a, int32_t val, bool allowUnconscious);
    int32_t    protection(Protection p) const;
    void       changeProtection(Protection p, int32_t val);

    uint32_t   instanceSymbol() const;
    uint32_t   guild() const;
    bool       isMonster() const;
    void       setTrueGuild(int32_t g);
    int32_t    trueGuild() const;
    int32_t    magicCyrcle() const;
    int32_t    level() const;
    int32_t    experience() const;
    int32_t    experienceNext() const;
    int32_t    learningPoints() const;
    int32_t    diveTime() const;

    void      setAttitude(Attitude att);
    Attitude  attitude() const { return permAttitude; }

    bool      isFriend() const;

    void      setTempAttitude(Attitude att);
    Attitude  tempAttitude() const { return tmpAttitude; }

    void      startDialog(Npc& other);
    bool      startState(ScriptFn id, const Daedalus::ZString& wp);
    bool      startState(ScriptFn id, const Daedalus::ZString& wp, gtime endTime, bool noFinalize);
    void      clearState(bool noFinalize);
    BodyState bodyState() const;
    BodyState bodyStateMasked() const;

    void      setToFightMode(const size_t item);
    void      setToFistMode();

    void      aiPush(AiQueue::AiAction&& a);

    bool      canSwitchWeapon() const;
    bool      closeWeapon(bool noAnim);
    bool      drawWeaponFist();
    bool      drawWeaponMele();
    bool      drawWeaponBow();
    bool      drawMage(uint8_t slot);
    bool      drawSpell(int32_t spell);
    auto      weaponState() const -> WeaponState;

    bool      canFinish(Npc &oth);
    void      fistShoot();
    void      blockFist();
    bool      finishingMove();
    void      swingSword();
    void      swingSwordL();
    void      swingSwordR();
    void      blockSword();
    bool      beginCastSpell();
    void      endCastSpell();
    void      setActiveSpellInfo(int32_t info);
    int32_t   activeSpellLevel() const;
    bool      castSpell();
    bool      aimBow();
    bool      shootBow(Interactive* focOverride = nullptr);
    bool      hasAmunition() const;

    bool      isEnemy(const Npc& other) const;
    bool      isDead() const;
    bool      isUnconscious() const;
    bool      isDown() const;
    bool      isAtack() const;
    bool      isTalk() const;
    bool      isPrehit() const;
    bool      isImmortal() const;

    void      setPerceptionTime   (uint64_t time);
    void      setPerceptionEnable (PercType t, size_t fn);
    void      setPerceptionDisable(PercType t);

    bool      perceptionProcess(Npc& pl);
    bool      perceptionProcess(Npc& pl, Npc *victum, float quadDist, PercType perc);
    bool      hasPerc(PercType perc) const;
    uint64_t  percNextTime() const;

    auto      interactive() const -> Interactive* { return currentInteract; }
    auto      detectedMob() const -> Interactive*;
    bool      setInteraction(Interactive* id, bool quick=false);
    void      quitIneraction();

    bool      isState(size_t stateFn) const;
    bool      wasInState(size_t stateFn) const;
    uint64_t  stateTime() const;
    void      setStateTime(int64_t time);

    void      addRoutine(gtime s, gtime e, uint32_t callback, const WayPoint* point);
    void      excRoutine(size_t callback);
    void      multSpeed(float s);

    bool      testMove    (const Tempest::Vec3& pos);
    bool      tryMove     (const Tempest::Vec3& dp);
    bool      tryTranslate(const Tempest::Vec3& pos);

    JumpStatus tryJump();
    bool      hasCollision() const { return physic.hasCollision(); }

    void      startDive();
    void      transformBack();

    auto      dialogChoises(Npc &player, const std::vector<uint32_t> &except, bool includeImp) -> std::vector<GameScript::DlgChoise>;

    auto      handle() -> Daedalus::GEngineClasses::C_Npc* { return  &hnpc; }

    auto      inventory() const -> const Inventory& { return invent; }
    size_t    hasItem    (size_t id) const;
    Item*     getItem    (size_t id);
    Item*     addItem    (size_t id, uint32_t amount);
    Item*     addItem    (std::unique_ptr<Item>&& i);
    Item*     takeItem   (Item& i);
    void      delItem    (size_t id, uint32_t amount);
    void      useItem    (size_t item, bool force=false);
    void      setCurrentItem(size_t item);
    void      unequipItem(size_t item);
    void      addItem    (size_t id, Interactive& chest,uint32_t count=1);
    void      addItem    (size_t id, Npc& from, uint32_t count=1);
    void      moveItem   (size_t id, Interactive& to,uint32_t count=1);
    void      sellItem   (size_t id, Npc& to,uint32_t count=1);
    void      buyItem    (size_t id, Npc& from,uint32_t count=1);
    void      dropItem   (size_t id);
    void      clearInventory();
    Item*     currentArmour();
    Item*     currentMeleWeapon();
    Item*     currentRangeWeapon();
    auto      mapWeaponBone() const -> Tempest::Vec3;

    bool      lookAt  (float dx, float dz, bool anim, uint64_t dt);
    bool      rotateTo(float dx, float dz, float speed, bool anim, uint64_t dt);
    auto      playAnimByName(const Daedalus::ZString& name, bool forceAnim, BodyState bs) -> const Animation::Sequence*;

    bool      checkGoToNpcdistance(const Npc& other);


    bool      isAiQueueEmpty() const;
    bool      isAiBusy() const;
    void      clearAiQueue();

    bool      isInState(ScriptFn fn) const;

    auto      currentWayPoint() const -> const WayPoint* { return currentFp; }
    void      attachToPoint(const WayPoint* p);
    GoToHint  moveHint() const { return go2.flag; }
    void      clearGoTo();

    bool      canSeeNpc(const Npc& oth,bool freeLos) const;
    bool      canSeeNpc(float x,float y,float z,bool freeLos) const;
    auto      canSenseNpc(const Npc& oth,bool freeLos, float extRange=0.f) const -> SensesBit;
    auto      canSenseNpc(float x,float y,float z,bool freeLos,bool isNoisy,float extRange=0.f) const -> SensesBit;

    void      setTarget(Npc* t);
    Npc*      target() const;

    void      clearNearestEnemy();
    int32_t   lastHitSpellId() const { return lastHitSpell; }

    void      setOther(Npc* ot);
    void      setVictum(Npc* ot);

    bool      haveOutput() const;
    void      setAiOutputBarrier(uint64_t dt);

    bool      doAttack(Anim anim);
    void      takeDamage(Npc& other,const Bullet* b);
    void      emitDlgSound(const char* sound);
    void      emitSoundEffect(const char* sound, float range, bool freeSlot);
    void      emitSoundGround(const char* sound, float range, bool freeSlot);
    void      emitSoundSVM   (const char* sound);

    void      startEffect(Npc& to, const VisualFx& vfx);
    void      stopEffect(const VisualFx& vfx);

    void      commitSpell();

  private:
    struct Routine final {
      gtime           start;
      gtime           end;
      ScriptFn        callback;
      const WayPoint* point=nullptr;
      };

    enum TransformBit : uint8_t {
      TR_Pos  =1,
      TR_Rot  =1<<1,
      TR_Scale=1<<2,
      };

    struct AiState final {
      ScriptFn funcIni;
      ScriptFn funcLoop;
      ScriptFn funcEnd;
      uint64_t sTime   =0;
      gtime    eTime   =gtime::endOfTime();
      bool     started =false;
      uint64_t loopNextTime=0;
      const char* hint="";
      };

    struct Perc final {
      ScriptFn func;
      };

    struct GoTo final {
      GoToHint         flag = GoToHint::GT_No;
      Npc*             npc  = nullptr;
      const WayPoint*  wp   = nullptr;
      Tempest::Vec3    pos  = {};

      void                         save(Serialize& fout) const;
      void                         load(Serialize&  fin);
      Tempest::Vec3                target() const;

      bool                         empty() const;
      void                         clear();
      void                         set(Npc* to, GoToHint hnt = GoToHint::GT_Way);
      void                         set(const WayPoint* to, GoToHint hnt = GoToHint::GT_Way);
      void                         set(const Item* to);
      void                         set(const Tempest::Vec3& to);
      };

    void      updateWeaponSkeleton();
    void      tickTimedEvt(Animation::EvCount &ev);
    void      tickRegen(int32_t& v,const int32_t max,const int32_t chg, const uint64_t dt);
    void      updatePos();
    void      setViewPosition(const Tempest::Vec3& pos);
    bool      tickCast();

    int       aiOutputOrderId() const;
    bool      performOutput(const AiQueue::AiAction &ai);

    auto      currentRoutine() const -> const Routine&;
    gtime     endTime(const Routine& r) const;

    bool      implLookAt (uint64_t dt);
    bool      implLookAt (const Npc& oth, uint64_t dt);
    bool      implLookAt (const Npc& oth, bool noAnim, uint64_t dt);
    bool      implLookAt (float dx, float dz, bool noAnim, uint64_t dt);
    bool      implGoTo   (uint64_t dt);
    bool      implGoTo   (uint64_t dt, float destDist);
    bool      implAtack  (uint64_t dt);
    void      adjustAtackRotation(uint64_t dt);
    bool      implAiTick (uint64_t dt);
    void      implAiWait (uint64_t dt);
    void      implAniWait(uint64_t dt);
    void      implFaiWait(uint64_t dt);
    void      implSetFightMode(const Animation::EvCount& ev);
    void      tickRoutine();
    void      nextAiAction(uint64_t dt);
    void      commitDamage();
    void      takeDamage(Npc& other);
    Npc*      updateNearestEnemy();
    Npc*      updateNearestBody();
    bool      checkHealth(bool onChange, bool forceKill);
    void      onNoHealth(bool death, HitSound sndMask);
    bool      hasAutoroll() const;
    void      stopWalkAnimation();

    void      dropTorch();

    void      saveAiState(Serialize& fout) const;
    void      loadAiState(Serialize& fin);
    static float angleDir(float x,float z);

    uint8_t   calcAniComb() const;

    bool               isAlignedToGround() const;
    Tempest::Vec3      groundNormal() const;
    Tempest::Matrix4x4 mkPositionMatrix() const;

    World&                         owner;
    Daedalus::GEngineClasses::C_Npc hnpc={};
    float                          x=0.f;
    float                          y=0.f;
    float                          z=0.f;
    float                          angle    = 0.f;
    float                          angleY   = 0.f;
    float                          runAng   = 0.f;
    float                          sz[3]={1.f,1.f,1.f};

    // visual props (cache)
    uint8_t                        durtyTranform=0;
    Tempest::Vec3                  lastGroundNormal;

    // visual props
    std::string                    body,head;
    int32_t                        vHead=0, vTeeth=0, vColor=0;
    int32_t                        bdColor=0;
    MdlVisual                      visual;

    DynamicWorld::NpcItem          physic;

    WalkBit                        wlkMode                 =WalkBit::WM_Run;
    int32_t                        trGuild                 =GIL_NONE;
    int32_t                        talentsSk[TALENT_MAX_G2]={};
    int32_t                        talentsVl[TALENT_MAX_G2]={};
    uint64_t                       refuseTalkMilis         =0;

    // attitude
    Attitude                       permAttitude=ATT_NULL;
    Attitude                       tmpAttitude =ATT_NULL;

    // perception
    uint64_t                       perceptionTime    =0;
    uint64_t                       perceptionNextTime=0;
    Perc                           perception[PERC_Count];

    // inventory
    Inventory                      invent;

    // last hit
    Npc*                           lastHit          = nullptr;
    char                           lastHitType      = 'A';
    int32_t                        lastHitSpell     = 0;

    // spell cast
    CastState                      castLevel        = CS_NoCast;
    size_t                         currentSpellCast = size_t(-1);
    uint64_t                       castNextTime     = 0;
    int32_t                        spellInfo        = 0;

    // transform-backshape
    struct TransformBack;
    std::unique_ptr<TransformBack> transform;

    // ai state
    uint64_t                       aniWaitTime=0;
    uint64_t                       waitTime=0;
    uint64_t                       faiWaitTime=0;
    uint64_t                       aiOutputBarrier=0;
    ProcessPolicy                  aiPolicy=ProcessPolicy::AiNormal;
    AiState                        aiState;
    ScriptFn                       aiPrevState;
    AiQueue                        aiQueue;
    std::vector<Routine>           routines;

    Interactive*                   currentInteract=nullptr;
    Npc*                           currentOther   =nullptr;
    Npc*                           currentVictum  =nullptr;

    Npc*                           currentLookAt  =nullptr;
    Npc*                           currentTarget  =nullptr;
    Npc*                           nearestEnemy   =nullptr;
    AiOuputPipe*                   outputPipe     =nullptr;

    Tempest::Vec3                  moveMobCacheKey={std::numeric_limits<float>::infinity(),0.f,0.f};
    Interactive*                   moveMob        =nullptr;

    GoTo                           go2;
    const WayPoint*                currentFp      =nullptr;
    FpLock                         currentFpLock;
    WayPath                        wayPath;

    MoveAlgo                       mvAlgo;
    FightAlgo                      fghAlgo;
    uint64_t                       lastEventTime=0;

    Sound                          sfxWeapon;

  friend class MoveAlgo;
  };
