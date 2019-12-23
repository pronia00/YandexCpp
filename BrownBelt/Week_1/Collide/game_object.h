#pragma once

class Unit;
class Building;
class Tower;
class Fence;

struct GameObject {
  virtual ~GameObject() = default;

  virtual bool Collide(const GameObject& that) const = 0;
  virtual bool CollideWith(const Unit& that) const = 0;
  virtual bool CollideWith(const Building& that) const = 0;
  virtual bool CollideWith(const Tower& that) const = 0;
  virtual bool CollideWith(const Fence& that) const = 0;
};


template <typename T> 
struct Collider : GameObject {
  bool Collide(const GameObject& that) const final {
    return that.CollideWith(static_cast<const T&>(*this));
  }
};
bool Collide(const GameObject& first, const GameObject& second);