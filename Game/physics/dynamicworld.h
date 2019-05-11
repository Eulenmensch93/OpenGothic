#pragma once

#include "physicmesh.h"

#include <zenload/zCMesh.h>
#include <zenload/zTypes.h>
#include <LinearMath/btScalar.h>
#include <memory>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <Tempest/Matrix4x4>

class btConstraintSolver;
class btCollisionConfiguration;
class btBroadphaseInterface;
class btDispatcher;
class btDynamicsWorld;
class btTriangleIndexVertexArray;
class btCollisionShape;
class btRigidBody;
class btGhostObject;
class btCollisionObject;
class btTriangleMesh;

class PhysicMeshShape;
class World;

class DynamicWorld final {
  public:
    DynamicWorld(World &world, const ZenLoad::PackedMesh &pkg);
    ~DynamicWorld();

    enum Category {
      C_Null      = 1,
      C_Landscape = 2,
      C_Ghost     = 3
      };

    struct Item {
      public:
        Item()=default;
        Item(DynamicWorld* owner,btCollisionObject* obj,float h,float r):owner(owner),obj(obj),height(h),r(r){}
        Item(Item&& it):owner(it.owner),obj(it.obj),height(it.height),r(it.r){it.obj=nullptr;}
        ~Item() { if(owner) owner->deleteObj(obj); }

        Item& operator = (Item&& it){
          std::swap(owner,it.owner);
          std::swap(obj,it.obj);
          std::swap(height,it.height);
          std::swap(r,it.r);
          return *this;
          }

        void setPosition(float x,float y,float z);
        void setObjMatrix(const Tempest::Matrix4x4& m);
        void setEnable(bool e);

        bool testMove(const std::array<float,3>& pos);
        bool testMove(const std::array<float,3>& pos, std::array<float,3> &fallback, float speed);
        bool tryMoveN(const std::array<float,3>& pos, std::array<float,3> &norm);
        bool tryMove (const std::array<float,3>& pos, std::array<float,3> &fallback, float speed);

        bool  hasCollision() const;
        float radius() const { return r; }

      private:
        DynamicWorld*       owner  = nullptr;
        btCollisionObject*  obj    = nullptr;
        float               height = 0.f;
        float               r      = 0.f;
        void implSetPosition(float x,float y,float z);

      friend class DynamicWorld;
      };

    struct RayResult final {
      std::array<float,3> v={};
      uint8_t             mat=0;
      bool                hasCol=0;

      float               x() const { return v[0]; }
      float               y() const { return v[1]; }
      float               z() const { return v[2]; }
      };

    RayResult dropRay(float x, float y, float z,bool& hasCol) const;
    RayResult dropRay(float x, float y, float z) const;

    RayResult ray(float x0, float y0, float z0, float x1, float y1, float z1) const;
    float     soundOclusion(float x0, float y0, float z0, float x1, float y1, float z1) const;

    std::array<float,3> landNormal(float x, float y, float z) const;

    Item ghostObj (const ZMath::float3& min,const ZMath::float3& max);
    Item staticObj(const PhysicMeshShape *src, const Tempest::Matrix4x4& m);

    void tick(uint64_t dt);

  private:
    void deleteObj(btCollisionObject* obj);
    bool hasCollision(const Item &it,std::array<float,3>& normal);
    void rayTest(const btVector3& rayFromWorld, const btVector3& rayToWorld, btCollisionWorld::RayResultCallback& resultCallback) const;    
    std::unique_ptr<btRigidBody> landObj();

    void updateSingleAabb(btCollisionObject* obj);

    struct HumShape;

    std::unique_ptr<btCollisionConfiguration>   conf;
    std::unique_ptr<btDispatcher>               dispatcher;
    std::unique_ptr<btBroadphaseInterface>      broadphase;
    std::unique_ptr<btCollisionWorld>           world;

    std::unique_ptr<PhysicMesh>                 landMesh;
    std::unique_ptr<btCollisionShape>           landShape;
    std::unique_ptr<btRigidBody>                landBody;

    mutable RayResult                           lastRayDrop;
    mutable float                               lastRayDropXyz[3]={};

    static const float                          ghostPadding;
    static const float                          ghostHeight;
    static const float                          worldHeight;
  };
