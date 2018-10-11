#pragma once

#include <crampl/NonTypeList.h>
#include <crampl/MemberComparator.h>

namespace crampl {

template<auto... M>
struct ForEachMember;

template<>
struct ForEachMember<> {
    template<typename Action, typename ObjType>
    static void call(const ObjType& obj, const Action& action) {
    }
};

template<auto M1, auto... M>
struct ForEachMember<M1, M...> {

    template<typename Action>
    static void call(const detail::ObjectType<M1, M...>& obj, const Action& action) {
        action(obj.*M1);
        ForEachMember<M...>::template call<Action>(obj, action);
    }

};

template<typename T>
struct ForEachMemberInList;
template<auto... M>
struct ForEachMemberInList<NonTypeList < M...>> : ForEachMember<M...> {

};

}