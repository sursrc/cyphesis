// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000,2001 Alistair Riddoch

#ifndef RULESETS_CHARACTER_H
#define RULESETS_CHARACTER_H

#include "Thing.h"

class Movement;
class BaseMind;

class Character : public Thing {
  protected:
    Movement & movement;
    double drunkness;
    std::string sex;
    double food;
    double maxMass;
    bool isAlive;

    static const double energyConsumption = 0.001;
    static const double foodConsumption = 0.1;
    static const double weightConsumption = 1.0;
    static const double energyGain = 0.5;
    static const double energyLoss = 0.1;
    static const double weightGain = 0.5;

    OpVector metabolise(double ammount = 1); 

    friend class Movement;
  protected:
    OpNoDict opMindLookup;
    OpNoDict opW2mLookup;

    void mindSubscribe(const std::string& op, OpNo no) {
        opMindLookup[op] = no;
    }

    void w2mSubscribe(const std::string& op, OpNo no) {
        opW2mLookup[op] = no;
    }
  public:
    BaseMind * mind;
    BaseMind * externalMind;

    explicit Character(const std::string & id);
    virtual ~Character();

    const double getDrunkness() const { return drunkness; }
    const std::string & getSex() const { return sex; }
    const double getFood() const { return food; }

    virtual bool get(const std::string &, Atlas::Message::Object &) const;
    virtual void set(const std::string &, const Atlas::Message::Object &);

    virtual void addToObject(Atlas::Message::Object::MapType &) const;

    virtual OpVector operation(const RootOperation & op);
    virtual OpVector externalMessage(const RootOperation & op);
    virtual OpVector externalOperation(const RootOperation & op);

    virtual OpVector ImaginaryOperation(const Imaginary & op);
    virtual OpVector SetupOperation(const Setup & op);
    virtual OpVector TickOperation(const Tick & op);
    virtual OpVector TalkOperation(const Talk & op);
    virtual OpVector EatOperation(const Eat & op);
    virtual OpVector NourishOperation(const Nourish & op);

    virtual OpVector mindLoginOperation(const Login & op);
    virtual OpVector mindLogoutOperation(const Logout & op);
    virtual OpVector mindCreateOperation(const Create & op);
    virtual OpVector mindActionOperation(const Action & op);
    virtual OpVector mindChopOperation(const Chop & op);
    virtual OpVector mindCombineOperation(const Combine & op);
    virtual OpVector mindCutOperation(const Cut & op);
    virtual OpVector mindDeleteOperation(const Delete & op);
    virtual OpVector mindDivideOperation(const Divide & op);
    virtual OpVector mindEatOperation(const Eat & op);
    virtual OpVector mindBurnOperation(const Burn & op);
    virtual OpVector mindGetOperation(const Get & op);
    virtual OpVector mindImaginaryOperation(const Imaginary & op);
    virtual OpVector mindInfoOperation(const Info & op);
    virtual OpVector mindMoveOperation(const Move & op);
    virtual OpVector mindNourishOperation(const Nourish & op);
    virtual OpVector mindSetOperation(const Set & op);
    virtual OpVector mindSightOperation(const Sight & op);
    virtual OpVector mindSoundOperation(const Sound & op);
    virtual OpVector mindTalkOperation(const Talk & op);
    virtual OpVector mindTickOperation(const Tick & op);
    virtual OpVector mindTouchOperation(const Touch & op);
    virtual OpVector mindLookOperation(const Look & op);
    virtual OpVector mindSetupOperation(const Setup & op);
    virtual OpVector mindAppearanceOperation(const Appearance & op);
    virtual OpVector mindDisappearanceOperation(const Disappearance & op);
    virtual OpVector mindErrorOperation(const Error & op);
    virtual OpVector mindOtherOperation(const RootOperation & op);

    bool w2mLoginOperation(const Login & op);
    bool w2mLogoutOperation(const Logout & op);
    bool w2mActionOperation(const Action & op);
    bool w2mChopOperation(const Chop & op);
    bool w2mCombineOperation(const Combine & op);
    bool w2mCreateOperation(const Create & op);
    bool w2mCutOperation(const Cut & op);
    bool w2mDeleteOperation(const Delete & op);
    bool w2mDivideOperation(const Divide & op);
    bool w2mEatOperation(const Eat & op);
    bool w2mBurnOperation(const Burn & op);
    bool w2mGetOperation(const Get & op);
    bool w2mImaginaryOperation(const Imaginary & op);
    bool w2mInfoOperation(const Info & op);
    bool w2mMoveOperation(const Move & op);
    bool w2mNourishOperation(const Nourish & op);
    bool w2mSetOperation(const Set & op);
    bool w2mSightOperation(const Sight & op);
    bool w2mSoundOperation(const Sound & op);
    bool w2mTouchOperation(const Touch & op);
    bool w2mTickOperation(const Tick & op);
    bool w2mLookOperation(const Look & op);
    bool w2mSetupOperation(const Setup & op);
    bool w2mTalkOperation(const Talk & op);
    bool w2mAppearanceOperation(const Appearance & op);
    bool w2mDisappearanceOperation(const Disappearance & op);
    bool w2mErrorOperation(const Error & op);
    bool w2mOtherOperation(const RootOperation & op);

    OpVector sendMind(const RootOperation & op);
    OpVector mind2body(const RootOperation & op);
    OpVector world2body(const RootOperation & op);
    bool world2mind(const RootOperation & op);
};

#endif // RULESETS_CHARACTER_H
