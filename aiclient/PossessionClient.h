/*
 Copyright (C) 2013 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef POSSESSIONCLIENT_H_
#define POSSESSIONCLIENT_H_

#include "BaseClient.h"
#include "MindClient.h"
#include <map>
#include <unordered_map>
#include <memory>

class MindFactory;

/**
 * Manages possession requests from the server and spawns new AI clients.
 */
class PossessionClient: public BaseClient
{
    public:
        PossessionClient(MindFactory& mindFactory);
        virtual ~PossessionClient();

        virtual void idle();

        /**
         * Notify the server that we are able to possess minds.
         */
        void enablePossession();

    protected:

        virtual void operation(const Operation & op, OpVector & res);

        void PossessOperation(const Operation & op, OpVector & res);

        MindFactory& m_mindFactory;

        /**
         * All active minds.
         */
        std::unordered_map<std::string, std::shared_ptr<MindClient>> m_minds;

        /**
         * Keeps track of any operations with refs which has been sent from any MindClient instances.
         *
         * This is currently only used to route the response to taking a character to the correct client.
         */
        std::map<long, std::string> m_refNoOperations;

};

#endif /* POSSESSIONCLIENT_H_ */