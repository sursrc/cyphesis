#include <Atlas/Message/Object.h>
#include <Atlas/Objects/Operation/Login.h>
#include <Atlas/Objects/Operation/Look.h>

using Atlas::Objects::Operation::Look;

#include "Thing.h"
#include "MemMap.h"


Thing * MemMap::add_object(Thing * object)
{
    cout << "MemMap::add_object " << object << " " << object->fullid
         << endl << flush;
    if (object != NULL) {
        things[object->fullid] = object;
    }

    cout << things[object->fullid] << endl << flush;
    cout << this << endl << flush;
    //for (/*hook in MemMap::add_hooks*/) {
        //hook(object);
    //}
    return object;
}

RootOperation * MemMap::look_id()
{
    cout << "MemMap::look_id" << endl << flush;
    if (additions_by_id.size() != 0) {
        string id = additions_by_id.front();
        additions_by_id.pop_front();
        Look * l = new Look();
        *l = Look::Instantiate();
        Object::MapType m;
        m["id"] = Object(id);
        l->SetArgs(Object::ListType(1, Object(m)));
        return l;
    }
    return(NULL);
}

Thing * MemMap::add_id(const string & id)
{
    cout << "MemMap::add_id" << endl << flush;
    additions_by_id.push_back(id);
    Object::MapType m;
    m["id"] = Object(string(id));
    Object obj(m);
    return add(obj);
}

Thing * MemMap::add(const Object & entity)
{
    cout << "MemMap::add" << endl << flush;
    if (!entity.IsMap()) {
        return NULL;
    }
    Object::MapType entmap = entity.AsMap();
    if ((entmap.find("id") == entmap.end()) ||
        (entmap["id"].AsString().size() == 0)) {
        return NULL;
    }
    if (get(entmap["id"].AsString())) {
        return update(entity);
    }
    Thing * thing = new Thing;
    thing->fullid = entmap["id"].AsString();
    if (entmap.find("name") != entmap.end() && entmap["name"].IsString()) {
        thing->name = entmap["name"].AsString();
    }
    thing->merge(entmap);
    return add_object(thing);
}

void MemMap::_delete(const string & id)
{
    cout << "MemMap::delete" << endl << flush;
    if (things.find(id) != things.end()) {
        Thing * obj = things[id];
        things.erase(id);
        //for (/*hook in MemMap::delete_hooks*/) {
            //hook(obj);
        //}
    }
}

Thing * MemMap::get(const string & id)
{
    cout << "MemMap::get" << endl << flush;
    if (things.find(id) != things.end()) {
        return things[id];
    }
    return(NULL);
}

#if 0 
bad_type MemMap::__getitem__(bad_type id)
{
    return MemMap::things[id];
}
#endif

Thing * MemMap::get_add(const string & id)
{
    cout << "MemMap::get_add" << endl << flush;
    Thing * obj = MemMap::get(id);
    if (obj != NULL) {
        return obj;
    }
    return add_id(id);
}

Thing * MemMap::update(const Object & entity)
{
    cout << "MemMap::update" << endl << flush;
    if (!entity.IsMap()) {
        return NULL;
    }
    Object::MapType entmap = entity.AsMap();
    if (entmap.find("id") == entmap.end()) {
        return NULL;
    }
    string & id = entmap["id"].AsString();
    if (id.size() == 0) {
        return NULL;
    }
    cout << " updating " << id << endl << flush;
    if (things.find(id) == things.end()) {
        return add(entity);
    }
    cout << " " << id << " has already been spotted" << endl << flush;
    Thing * thing = things[id];
    cout << " got " << thing << endl << flush;
    // I am not sure what the deal is with all the "needTrueValue stuff
    // below yet. FIXME find out exactly what is required.
    if (entmap.find("name") != entmap.end() && entmap["name"].IsString()) {
        thing->name = entmap["name"].AsString();
    }
    cout << " got " << thing << endl << flush;
    thing->merge(entmap);
    //needTrueValue=["type","contains","instance","id","location","stamp"];
    //for (/*(key,value) in entity.__dict__.items()*/) {
        //if (value or not key in needTrueValue) {
            //setattr(obj,key,value);
        //}
    //}
    //for (/*hook in MemMap::update_hooks*/) {
        //hook(obj);
    //}
    return thing;
}

#if 0 
bad_type MemMap::find_by_location(bad_type location, bad_type radius=0.0)
{
    res=[];
    for (/*p in MemMap::things.values()*/) {
        if (p.location and location.parent==p.location.parent) {
            d=location.coordinates.distance(p.location.coordinates);
            if (d<=radius) {
                res.append(d,p);
            }
        }
    }
    res.sort();
    return res;
}

bad_type MemMap::find_by_type(bad_type what)
{
    res=[];
    for (/*thing in MemMap::things.values()*/) {
        if (thing.type!=[]) {
            if (thing.type[0]==what) {
                res.append(thing);
            }
        }
    }
    return res;
}
#endif
