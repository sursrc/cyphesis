#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "TestBase.h"

#include "rulesets/entityfilter/Filter.h"
#include "rulesets/entityfilter/ParserDefinitions.h"
#include "rulesets/entityfilter/Providers.h"

#include "rulesets/EntityProperty.h"
#include "rulesets/Domain.h"
#include "rulesets/AtlasProperties.h"
#include "rulesets/OutfitProperty.h"
#include "rulesets/BBoxProperty.h"
#include "common/Property.h"
#include "common/BaseWorld.h"
#include "common/log.h"
#include "common/Inheritance.h"

#include "rulesets/Entity.h"
#include "common/TypeNode.h"

#include <wfmath/point.h>
#include <Atlas/Objects/Anonymous.h>

#include <cassert>

using namespace EntityFilter;
using Atlas::Message::Element;

static std::map<std::string, TypeNode*> types;

class ProvidersTest : public Cyphesis::TestBase {
    private:
        ProviderFactory m_factory;

        //Entities for testing
        Entity *m_b1;
        LocatedEntitySet *m_b1_container; //Container property for b1
        Entity *m_b2;

        Entity *m_ch1; //Character type entity
        Entity *m_glovesEntity; //Gloves for the character entity's outfit
        Entity *m_cloth; //Cloth for gloves' outfit

        //Types for testing
        TypeNode *m_barrelType;
        TypeNode *m_characterType;
        TypeNode *m_clothType;

        ///\A helper to create providers. Accepts a list of tokens and assumes that
        ///the delimiter for all but first token is "." (a dot)
        /// for example, to make entity.type provider, use {"Entity", "type"} argument
        Consumer<QueryContext>* CreateProvider(std::initializer_list<std::string> tokens);
    public:
        ProvidersTest();

        ///\Initialize private variables for testing before each test.
        void setup();
        ///\Free allocated space after every test
        void teardown();
        ///\Test basic property providers (soft properties, type, id)
        void test_EntityProperty();
        ///\Test BBox providers (bbox volume, height, area etc)
        void test_BBoxProviders();
        ///\Test Outfit providers
        void test_OutfitProviders();
        ///\Test comparator and logical predicates
        void test_ComparePredicates();

};

void ProvidersTest::test_EntityProperty()
{

    Atlas::Message::Element value;

    auto provider = CreateProvider( { "entity" });

    provider->value(value, QueryContext { *m_b1 });
    assert(value.Ptr() == m_b1);

    //entity.type
    provider = CreateProvider( { "entity", "type" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Ptr() == m_barrelType);

    //entity.id
    provider = CreateProvider( { "entity", "id" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Int() == 1);

    //entity.mass
    provider = CreateProvider( { "entity", "mass" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Int() == 30);

    //entity.burn_speed
    provider = CreateProvider( { "entity", "burn_speed" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Float() == 0.3);
}

void ProvidersTest::test_BBoxProviders()
{
    Atlas::Message::Element value;
    //entity.bbox.volume
    auto provider = CreateProvider( { "entity", "bbox", "volume" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Float() == 48.0);

    //entity.bbox.height
    provider = CreateProvider( { "entity", "bbox", "height" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Float() == 6.0);

    //entity.bbox.width
    provider = CreateProvider( { "entity", "bbox", "width" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Float() == 2.0);

    //entity.bbox.depth
    provider = CreateProvider( { "entity", "bbox", "depth" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Float() == 4.0);

    //entity.bbox.area
    provider = CreateProvider( { "entity", "bbox", "area" });
    provider->value(value, QueryContext { *m_b1 });
    assert(value.Float() == 8.0);
}

void ProvidersTest::test_OutfitProviders()
{
    Atlas::Message::Element value;
    //Check if we get the right entity in outfit query
    auto provider = CreateProvider({"entity", "outfit", "hands"});
    provider->value(value, QueryContext{*m_ch1});
    ASSERT_EQUAL(*(Entity**)value.Ptr(), m_glovesEntity);

    //Check for outfit's property query
    provider = CreateProvider({"entity", "outfit", "hands", "color"});
    provider->value(value, QueryContext{*m_ch1});
    assert(value.String() == "brown");

    //Check if we get the right entity in nested outfit query
    provider = CreateProvider({"entity", "outfit", "hands", "outfit", "thumb"});
    provider->value(value, QueryContext{*m_ch1});
    assert(*(Entity**)value.Ptr() == m_cloth);

    //Check for nested outfit's property
    provider = CreateProvider({"entity", "outfit", "hands", "outfit", "thumb", "color"});
    provider->value(value, QueryContext{*m_ch1});
    assert(value.String() == "green");
}

void ProvidersTest::test_ComparePredicates()
{
    //entity.type = types.barrel
    auto lhs_provider1 = CreateProvider( { "entity", "type" });
    auto rhs_provider1 = CreateProvider( { "types", "barrel" });

    ComparePredicate compPred1(lhs_provider1, rhs_provider1,
                               ComparePredicate::Comparator::INSTANCE_OF);
    assert(compPred1.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume
    auto lhs_provider2 = CreateProvider( { "entity", "bbox", "volume" });

    //entity.bbox.volume = 48
    ComparePredicate compPred2(lhs_provider2, new FixedElementProvider(48.0f),
                               ComparePredicate::Comparator::EQUALS);
    assert(compPred2.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume = 1
    ComparePredicate compPred3(lhs_provider2, new FixedElementProvider(1.0f),
                               ComparePredicate::Comparator::EQUALS);
    assert(!compPred3.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume != 1
    ComparePredicate compPred4(lhs_provider2, new FixedElementProvider(1.0f),
                               ComparePredicate::Comparator::NOT_EQUALS);
    assert(compPred4.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume > 0
    ComparePredicate compPred5(lhs_provider2, new FixedElementProvider(0.0f),
                               ComparePredicate::Comparator::GREATER);
    assert(compPred5.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume >= 1
    ComparePredicate compPred6(lhs_provider2, new FixedElementProvider(1.0f),
                               ComparePredicate::Comparator::GREATER_EQUAL);
    assert(compPred6.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume < 5
    ComparePredicate compPred7(lhs_provider2, new FixedElementProvider(5.0f),
                               ComparePredicate::Comparator::LESS);
    assert(!compPred7.isMatch(QueryContext { *m_b1 }));

    //entity.bbox.volume <= 48
    ComparePredicate compPred8(lhs_provider2, new FixedElementProvider(48.0f),
                               ComparePredicate::Comparator::LESS_EQUAL);
    assert(compPred8.isMatch(QueryContext { *m_b1 }));

    //entity.type = types.barrel && entity.bbox.volume = 48
    AndPredicate andPred1(&compPred1, &compPred2);
    assert(andPred1.isMatch(QueryContext { *m_b1 }));

    //entity.type = types.barrel && entity.bbox.volume = 1
    AndPredicate andPred2(&compPred1, &compPred3);
    assert(!andPred2.isMatch(QueryContext { *m_b1 }));

    //entity.type = types.barrel || entity.bbox.volume = 1
    OrPredicate orPred1(&compPred1, &compPred3);
    assert(orPred1.isMatch(QueryContext { *m_b1 }));
}

ProvidersTest::ProvidersTest()
{
    ADD_TEST(ProvidersTest::test_EntityProperty);
    ADD_TEST(ProvidersTest::test_BBoxProviders);
    ADD_TEST(ProvidersTest::test_OutfitProviders);
    ADD_TEST(ProvidersTest::test_ComparePredicates);
}

void ProvidersTest::setup()
{
    //Make a barrel with mass and burn speed properties
    m_b1 = new Entity("1", 1);
    m_barrelType = new TypeNode("barrel");
    types["barrel"] = m_barrelType;
    m_b1->setType(m_barrelType);
    m_b1->setProperty("mass", new SoftProperty(Element(30)));
    m_b1->setProperty("burn_speed", new SoftProperty(Element(0.3)));
    m_b1->setProperty("isVisible", new SoftProperty(Element(true)));

    //Make a second barrel
    m_b2 = new Entity("2", 2);
    m_b2->setProperty("mass", new SoftProperty(Element(20)));
    m_b2->setProperty("burn_speed", new SoftProperty(0.25));
    m_b2->setType(m_barrelType);
    m_b2->setProperty("isVisible", new SoftProperty(Element(false)));

    //Make first barrel contain the second barrel
    m_b1_container = new LocatedEntitySet;
    m_b1_container->insert(m_b2);
    m_b1->m_contains = m_b1_container;

    //Set bounding box properties for barrels
    BBoxProperty* bbox1 = new BBoxProperty;
    //Specify two corners of bbox in form of x, y, z coordinates
    bbox1->set((std::vector<Element> { -1, -2, -3, 1, 2, 3 }));
    m_b1->setProperty("bbox", bbox1);

    BBoxProperty* bbox2 = new BBoxProperty;
    bbox2->set(std::vector<Element> { -3, -1, -2, 1, 2, 3 });
    m_b2->setProperty("bbox", bbox2);

    ///Set up outfit testing

    //Green Cloth serves as outfit for gloves
    m_clothType = new TypeNode("cloth");
    types["cloth"] = m_clothType;

    m_cloth = new Entity("3", 3);
    m_cloth->setType(m_clothType);
    m_cloth->setProperty("color", new SoftProperty("green"));

    //Create outfit map where "thumb" outfit contains cloth
    std::map<std::string, Element> outfitMap1;
    outfitMap1.insert(std::make_pair("thumb", Element(m_cloth)));
    OutfitProperty* outfit2 = new OutfitProperty;
    outfit2->set(outfitMap1);
    m_glovesEntity = new Entity("4", 4);
    m_glovesEntity->setProperty("color", new SoftProperty("brown"));
    m_glovesEntity->setProperty("outfit", outfit2);

    //Create outfit map where hands of character contain brown gloves
    std::map<std::string, Element> outfitMap;
    outfitMap.insert(std::make_pair("hands", Element(m_glovesEntity)));
    OutfitProperty* outfit1 = new OutfitProperty;
    outfit1->set(outfitMap);

    //Create the character for testing
    m_characterType = new TypeNode("character");
    types["character"] = m_characterType;
    m_ch1 = new Entity("5", 5);
    m_ch1->setType(m_characterType);
    m_ch1->setProperty("outfit", outfit1);
}

void ProvidersTest::teardown()
{
    delete m_b1;
    delete m_b2;
    delete m_ch1;
    delete m_glovesEntity;
    delete m_barrelType;
    delete m_characterType;
    delete m_clothType;
}

Consumer<QueryContext>* ProvidersTest::CreateProvider(std::initializer_list<
        std::string> tokens)
{
    ProviderFactory::SegmentsList segments;
    auto iter = tokens.begin();

    //First token doesn't have a delimiter, so just add it.
    segments.push_back(ProviderFactory::Segment { "", *iter++ });

    //Starting from the second token, add them to the list of segments with "." delimiter
    for (; iter != tokens.end(); iter++) {
        segments.push_back(ProviderFactory::Segment { ".", *iter });
    }
    return m_factory.createProviders(segments);
}

int main()
{
    ProvidersTest t;

    t.run();
}

//Stubs

#include "stubs/common/stubVariable.h"
#include "stubs/common/stubMonitors.h"
#include "stubs/rulesets/stubDomainProperty.h"
#include "stubs/rulesets/stubIdProperty.h"

ContainsProperty::ContainsProperty(LocatedEntitySet & data) :
        PropertyBase(per_ephem), m_data(data)
{
}

int ContainsProperty::get(Atlas::Message::Element & e) const
{
    return 0;
}

void ContainsProperty::set(const Atlas::Message::Element & e)
{
}

void ContainsProperty::add(const std::string & s,
                           const Atlas::Objects::Entity::RootEntity & ent) const
{
}

ContainsProperty * ContainsProperty::copy() const
{
    return 0;
}

namespace Atlas
{
namespace Objects
{
namespace Operation
{
int ACTUATE_NO = -1;
int ATTACK_NO = -1;
int EAT_NO = -1;
int NOURISH_NO = -1;
int SETUP_NO = -1;
int TICK_NO = -1;
int UPDATE_NO = -1;
int RELAY_NO = -1;
}
}
}
Router::Router(const std::string & id, long intId) :
        m_id(id), m_intId(intId)
{
}

Router::~Router()
{
}

void Router::addToMessage(Atlas::Message::MapType & omap) const
{
}

void Router::addToEntity(const Atlas::Objects::Entity::RootEntity & ent) const
{
}
BaseWorld*BaseWorld::m_instance = 0;
BaseWorld::BaseWorld(LocatedEntity & gw) :
        m_gameWorld(gw)
{
    m_instance = this;
}

BaseWorld::~BaseWorld()
{
    m_instance = 0;
}

LocatedEntity * BaseWorld::getEntity(const std::string & id) const
{
    return 0;
}

void Location::addToMessage(MapType & omap) const
{
}

Location::Location() :
        m_loc(0)
{
}

void Location::addToEntity(const Atlas::Objects::Entity::RootEntity & ent) const
{
}
void Location::modifyBBox()
{
}

Inheritance::Inheritance()
{
}

Inheritance & Inheritance::instance()
{
    return *(new Inheritance());
}

const TypeNode * Inheritance::getType(const std::string & parent)
{
    auto I = types.find(parent);
    if (I == types.end()) {
        return 0;
    }
    return I->second;
}

void log(LogLevel lvl, const std::string & msg)
{
}
