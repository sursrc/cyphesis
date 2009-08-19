// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000-2005 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

// $Id$

#include <Python.h>

#include "EntityBuilder.h"

#include "CorePropertyManager.h"
#include "EntityFactory.h"
#include "ScriptFactory.h"
#include "TaskFactory.h"
#include "Persistence.h"
#include "Player.h"

#include "rulesets/Thing.h"
#include "rulesets/MindFactory.h"
#include "rulesets/Character.h"
#include "rulesets/Creator.h"
#include "rulesets/Plant.h"
#include "rulesets/Stackable.h"
#include "rulesets/World.h"

#include "rulesets/Python_Script_Utils.h"

#include "common/id.h"
#include "common/log.h"
#include "common/debug.h"
#include "common/globals.h"
#include "common/const.h"
#include "common/Inheritance.h"
#include "common/AtlasFileLoader.h"
#include "common/random.h"
#include "common/compose.hpp"
#include "common/Monitors.h"
#include "common/Property.h"
#include "common/TypeNode.h"

#include <Atlas/Message/Element.h>
#include <Atlas/Objects/Entity.h>
#include <Atlas/Objects/RootOperation.h>

#include <sys/types.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif // HAS_DIRENT_H

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::RootEntity;

using String::compose;

typedef std::map<std::string, Atlas::Objects::Root> RootDict;

static const bool debug_flag = false;

EntityBuilder * EntityBuilder::m_instance = NULL;

EntityBuilder::EntityBuilder(BaseWorld & w) : m_world(w)
{
    installFactory("world", "game_entity", new EntityFactory<World>());
    EntityFactory<Thing> * tft = new EntityFactory<Thing>();
    installFactory("thing", "game_entity", tft);
    installFactory("character", "thing", new EntityFactory<Character>());
    installFactory("creator", "character", new EntityFactory<Creator>());
    installFactory("plant", "thing", new EntityFactory<Plant>());
    installFactory("stackable", "thing", new EntityFactory<Stackable>());

    // The property manager instance installs itself at construction time.
    new CorePropertyManager();
}

EntityBuilder::~EntityBuilder()
{
    delete PropertyManager::instance();
}

/// \brief Build and populate a new entity object.
///
/// A factory is found for the type of entity, and invoked to create the object
/// instance. If the type has a script factory, this is invoked to create the
/// associates script object which is attached to the entity instance.
/// The attribute values are then set on the instance, taking into account
/// the defaults for the class, and those inherited from parent classes, and
/// the values specified for this instance. The essential location data for
/// this instance is then set up. The final block of code relating to
/// persistence is legacy, and should never be invoked.
/// @param id The string identifier of the new entity.
/// @param intId The integer identifier of the new entity.
/// @param type The string specifying the type of entity.
/// @param attributes A mapping of attribute values to set on the entity.
Entity * EntityBuilder::newEntity(const std::string & id, long intId,
                                  const std::string & type,
                                  const RootEntity & attributes) const
{
    debug(std::cout << "EntityFactor::newEntity()" << std::endl << std::flush;);
    Entity * thing = 0;
    FactoryDict::const_iterator I = m_entityFactories.find(type);
    if (I == m_entityFactories.end()) {
        return 0;
    }
    EntityKit * factory = I->second;
    thing = factory->newEntity(id, intId);
    if (thing == 0) {
        return 0;
    }
    debug( std::cout << "[" << type << "]"
                     << std::endl << std::flush;);
    thing->setType(factory->m_type);
    // Sort out python object
    if (factory->m_scriptFactory != 0) {
        debug(std::cout << "Class " << type << " has a python class"
                        << std::endl << std::flush;);
        factory->m_scriptFactory->addScript(thing);
    }
    //
    factory->populate(*thing);

    MapType attrs = attributes->asMessage();
    // Apply the attribute values
    thing->merge(attrs);
    // Then set up the default class properties
    PropertyDict::const_iterator J = factory->m_type->defaults().begin();
    PropertyDict::const_iterator Jend = factory->m_type->defaults().end();
    for (; J != Jend; ++J) {
        PropertyBase * prop = J->second;
        // If a property is in the class it won't have been installed
        // as setAttr() checks
        prop->install(thing);
        // The property will have been applied if it has an overriden
        // value, so we only apply it the value is still default.
        if (attrs.find(J->first) == attrs.end()) {
            prop->apply(thing);
        }
    }
    // Get location from entity, if it is present
    // The default attributes cannot contain info on location
    if (attributes->hasAttrFlag(Atlas::Objects::Entity::LOC_FLAG)) {
        const std::string & loc_id = attributes->getLoc();
        thing->m_location.m_loc = m_world.getEntity(loc_id);
    }
    if (thing->m_location.m_loc == 0) {
        // If no info was provided, put the entity in the game world
        thing->m_location.m_loc = &m_world.m_gameWorld;
    }
    thing->m_location.readFromEntity(attributes);
    if (!thing->m_location.pos().isValid()) {
        // If no position coords were provided, put it somewhere near origin
        thing->m_location.m_pos = Point3D(uniform(-8,8), uniform(-8,8), 0);
    }
    if (thing->m_location.velocity().isValid()) {
        if (attributes->hasAttrFlag(Atlas::Objects::Entity::VELOCITY_FLAG)) {
            log(ERROR, compose("EntityBuilder::newEntity(%1, %2): "
                               "Entity has velocity set from the attributes "
                               "given by the creator", id, type));
        } else {
            log(ERROR, compose("EntityBuilder::newEntity(%1, %2): Entity has "
                               "velocity set from an unknown source",
                               id, type));
        }
        thing->m_location.m_velocity.setValid(false);
    }
    return thing;
}

/// \brief Build and populate a new task object.
///
/// @param name The name of the task type.
/// @param owner The character entity that owns the task.
Task * EntityBuilder::newTask(const std::string & name, Character & owner) const
{
    TaskFactoryDict::const_iterator I = m_taskFactories.find(name);
    if (I == m_taskFactories.end()) {
        return 0;
    }
    return I->second->newTask(owner);
}

void EntityBuilder::installTaskFactory(const std::string & class_name,
                                       TaskKit * factory)
{
    m_taskFactories.insert(std::make_pair(class_name, factory));
}

TaskKit * EntityBuilder::getTaskFactory(const std::string & class_name)
{
    TaskFactoryDict::const_iterator I = m_taskFactories.find(class_name);
    if (I == m_taskFactories.end()) {
        return 0;
    }
    return I->second;
}

void EntityBuilder::addTaskActivation(const std::string & tool,
                                      const std::string & op,
                                      TaskKit * factory)
{
    m_taskActivations[tool].insert(std::make_pair(op, factory));
}

/// \brief Build a new task object activated by the described event.
///
/// An event is described in terms of the tool type used to cause it,
/// the type of operation being performed using the tool and the type of
/// the target object the tool is being used on. If a match is found for
/// this event, a task object is instanced to track the progress of the
/// result of the event.
/// @param tool The type of tool activating the event.
/// @param op The type of operation being performed with the tool.
/// @param target The type of entity the operation is being performed on.
/// @param owner The character entity activating the task.
Task * EntityBuilder::activateTask(const std::string & tool,
                                   const std::string & op,
                                   const std::string & target,
                                   Character & owner) const
{
    TaskFactoryActivationDict::const_iterator I = m_taskActivations.find(tool);
    if (I == m_taskActivations.end()) {
        return 0;
    }
    const TaskFactoryMultimap & dict = I->second;
    TaskFactoryMultimap::const_iterator J = dict.lower_bound(op);
    if (J == dict.end()) {
        return 0;
    }
    TaskFactoryMultimap::const_iterator Jend = dict.upper_bound(op);
    for (; J != Jend; ++J) {
        if (!J->second->m_target.empty()) {
            if (!Inheritance::instance().isTypeOf(target, J->second->m_target)) {
                debug( std::cout << target << " is not a " << J->second->m_target
                                 << std::endl << std::flush; );
                continue;
            }
        }
        return J->second->newTask(owner);
    }
    return 0;
}

/// \brief Clear out all the factory objects owned by the entity builder.
void EntityBuilder::flushFactories()
{
    FactoryDict::const_iterator Iend = m_entityFactories.end();
    for (FactoryDict::const_iterator I = m_entityFactories.begin(); I != Iend; ++I) {
        delete I->second;
    }
    m_entityFactories.clear();
    TaskFactoryDict::const_iterator K = m_taskFactories.begin();
    TaskFactoryDict::const_iterator Kend = m_taskFactories.end();
    for (; K != Kend; ++K) {
        delete K->second;
    }
    m_taskFactories.clear();
}

bool EntityBuilder::isTask(const std::string & class_name)
{
    if (class_name == "task") {
        return true;
    }
    return (m_taskFactories.find(class_name) != m_taskFactories.end());
}

bool EntityBuilder::hasTask(const std::string & class_name)
{
    return (m_taskFactories.find(class_name) != m_taskFactories.end());
}

static void updateChildren(EntityKit * factory)
{
    std::set<EntityKit *>::const_iterator I = factory->m_children.begin();
    std::set<EntityKit *>::const_iterator Iend = factory->m_children.end();
    for (; I != Iend; ++I) {
        EntityKit * child_factory = *I;
        child_factory->m_attributes = factory->m_attributes;
        MapType::const_iterator J = child_factory->m_classAttributes.begin();
        MapType::const_iterator Jend = child_factory->m_classAttributes.end();
        for (; J != Jend; ++J) {
            child_factory->m_attributes[J->first] = J->second;
        }
        updateChildren(child_factory);
    }
}

static void updateChildrenProperties(EntityKit * factory)
{
    // Discover the default attributes which are no longer
    // present after the update.
    std::set<std::string> removed_properties;
    PropertyDict & defaults = factory->m_type->defaults();
    PropertyDict::const_iterator I = defaults.begin();
    PropertyDict::const_iterator Iend = defaults.end();
    MapType::const_iterator Jend = factory->m_attributes.end();
    for (; I != Iend; ++I) {
        if (factory->m_attributes.find(I->first) == Jend) {
            debug( std::cout << I->first << " removed" << std::endl; );
            removed_properties.insert(I->first);
        }
    }

    // Remove the class properties for the default attributes that
    // no longer exist
    std::set<std::string>::const_iterator L = removed_properties.begin();
    std::set<std::string>::const_iterator Lend = removed_properties.end();
    for (; L != Lend; ++L) {
        PropertyDict::iterator M = defaults.find(*L);
        delete M->second;
        defaults.erase(M);
    }

    // Update the values of existing class properties, and add new class
    // properties for added default attributes.
    MapType::const_iterator J = factory->m_attributes.begin();
    PropertyBase * p;
    for (; J != Jend; ++J) {
        PropertyDict::const_iterator I = defaults.find(J->first);
        if (I == Iend) {
            p = PropertyManager::instance()->addProperty(J->first,
                                                         J->second.getType());
            assert(p != 0);
            p->setFlags(flag_class);
            defaults[J->first] = p;
        } else {
            p = I->second;
        }
        p->set(J->second);
    }

    // Propagate the changes to all child factories
    std::set<EntityKit *>::const_iterator K = factory->m_children.begin();
    std::set<EntityKit *>::const_iterator Kend = factory->m_children.end();
    for (; K != Kend; ++K) {
        EntityKit * child_factory = *K;
        updateChildrenProperties(child_factory);
    }
}

void EntityBuilder::installFactory(const std::string & class_name,
                                   const std::string & parent,
                                   EntityKit * factory,
                                   Root class_desc)
{
    assert(factory != 0);

    m_entityFactories[class_name] = factory;
    Monitors::instance()->watch(compose("created_count{type=%1}", class_name),
                                new Monitor<int>(factory->m_createdCount));

    Inheritance & i = Inheritance::instance();

    if (class_desc.isValid()) {
        assert(class_desc->getId() == class_name);
        assert(class_desc->getParents().front() == parent);
        factory->m_type = i.addChild(class_desc);
    } else {
        factory->m_type = i.addChild(atlasClass(class_name, parent));
    }
}

EntityKit * EntityBuilder::getClassFactory(const std::string & class_name)
{
    FactoryDict::const_iterator I = m_entityFactories.find(class_name);
    if (I == m_entityFactories.end()) {
        return 0;
    }
    return I->second;
}

EntityKit * EntityBuilder::getNewFactory(const std::string & parent)
{
    FactoryDict::const_iterator I = m_entityFactories.find(parent);
    if (I == m_entityFactories.end()) {
        return 0;
    }
    return I->second->duplicateFactory();
}
