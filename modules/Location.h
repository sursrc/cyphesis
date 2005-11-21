// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000-2005 Alistair Riddoch

#ifndef MODULES_LOCATION_H
#define MODULES_LOCATION_H

#include <physics/Vector3D.h>
#include <physics/BBox.h>
#include <physics/Quaternion.h>

#include <Atlas/Message/Element.h>
#include <Atlas/Objects/ObjectsFwd.h>

#include <sigc++/object.h>

class Entity;

class Location : virtual public SigC::Object {
  private:
    bool m_simple;
    bool m_solid;

    double m_timeStamp;
  public:
    Entity * m_loc;
    Point3D m_pos;   // Coords relative to m_loc entity
    Vector3D m_velocity; // Veclociy vector, relative to m_loc entity.
    Vector3D m_acceleration; // Acceleration vector, relative to m_loc entity.
    Quaternion m_orientation;
    Vector3D m_angular; // Angular velocity vector.

    BBox m_bBox;

    Location();
    explicit Location(Entity * rf);
    explicit Location(Entity * rf, const Point3D & crds);
    explicit Location(Entity * rf, const Point3D & crds, const Vector3D & vel);

    const Point3D & pos() const { return m_pos; }
    const Vector3D & velocity() const { return m_velocity; }
    const Vector3D & acceleration() const { return m_acceleration; }
    const Quaternion & orientation() const { return m_orientation; }
    const Vector3D & angular() const { return m_angular; }
    const BBox & bBox() const { return m_bBox; }

    bool isValid() const {
        return ((m_loc != NULL) && m_pos.isValid());
    }

    bool isSimple() const {
        return m_simple;
    }

    void setSimple(bool c = true) {
        m_simple = c;
    }

    bool isSolid() const {
        return m_solid;
    }

    void setSolid(bool s = true) {
        m_solid = s;
    }

    const double & timeStamp() const {
        return m_timeStamp;
    }

    void update(const double & time) {
        m_timeStamp = time;
    }

    void setBBox(const BBox & b) {
        m_bBox = b;
        modifyBBox();
    }

    void addToMessage(Atlas::Message::MapType & ent) const;
    void addToEntity(const Atlas::Objects::Entity::RootEntity & ent) const;
    const Atlas::Objects::Root asEntity() const;

    int readFromEntity(const Atlas::Objects::Entity::RootEntity & ent);
    void modifyBBox();

    friend std::ostream & operator<<(std::ostream& s, Location& v);
};

const Vector3D distanceTo(const Location & self, const Location & other);

const Point3D relativePos(const Location & self, const Location & other);

const float squareDistance(const Location & self, const Location & other);
const float squareHorizontalDistance(const Location & self,
                                     const Location & other);

inline const float distance(const Location & self, const Location & other)
{
    return sqrt(squareDistance(self, other));
}

#endif // MODULES_LOCATION_H
