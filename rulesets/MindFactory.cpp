// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000,2001 Alistair Riddoch
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

#include "MindFactory.h"

#include "BaseMind.h"

#include "common/debug.h"
#include "common/ScriptKit.h"

static const bool debug_flag = false;

MindKit::MindKit() : m_scriptFactory(0)
{
}

MindKit::~MindKit()
{
    delete m_scriptFactory;
}

MindFactory::~MindFactory()
{
}

BaseMind * MindFactory::newMind(const std::string & id, long intId) const
{
    return new BaseMind(id, intId);
#if 0
    mind->setType(type);
    std::string mind_class("NPCMind"), mind_package("mind.NPCMind");
    MindFactory::mindmap_t::const_iterator I = m_mindTypes.find(type->name());
    if (I != m_mindTypes.end()) {
        mind_package = I->second;
        mind_class = type->name() + "Mind";
        debug(std::cout << "Got custom mind of type " << mind_package << " for "
                        << type << std::endl << std::flush;);
    }
    Create_PyMind(mind, mind_package, mind_class);
    return mind;
#endif
}
