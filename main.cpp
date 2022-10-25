#include "ProtobufRouterGenerator.h"
#include <QtWidgets/QApplication>
#include "MessageWrapper.hpp"

namespace ohtoai::protobuf
{
    template<typename ...OtherTypes>
    decltype(auto) getMessageNames()
    {
        return std::apply(
            [](auto&&... elems)
            {
                std::vector<std::string> result;
                result.reserve(sizeof...(elems));
                (result.push_back(std::forward<decltype(elems)>(elems)), ...);
                return result;
            }
            , std::forward<decltype(
                std::make_tuple(OtherTypes::descriptor()->full_name()...))>(
                    std::make_tuple(OtherTypes::descriptor()->full_name()...)));
    }
}

int main(int argc, char *argv[])
{
    auto names = ohtoai::protobuf::getMessageNames<
        pb::socket::UserLogin
        , pb::socket::UserRegister
        , pb::socket::UserInfo
        , pb::socket::ExternalMessage
    >();

    QApplication a(argc, argv);
    ProtobufRouterGenerator w;
    w.show();
    return a.exec();
}
