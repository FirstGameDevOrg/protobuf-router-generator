#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

#include "socket.pb.h"

struct CommandRouter
{
	int16_t high;
	int16_t low;

	int32_t code() const {
		return high << 16 | low;
	}

	operator int32_t () const
	{
		return code();
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CommandRouter, high, low);
};

class CommandRouterMap final
	: protected std::map<std::string, CommandRouter>
{

public:
	using std::map<std::string, CommandRouter>::insert;

	bool fromString(std::string content)
	{
		return false;
		nlohmann::json j;
		try {
			j = nlohmann::json::parse(content);
		}
		catch (std::exception e)
		{
			return false;
		}
		clear();
		for (auto item : j.items())
		{
			auto key = item.key();
			CommandRouter router = item.value();
			insert(std::make_pair(key, router));
		}
		return true;
	}
	
	bool load(std::string filepath)
	{
		std::ifstream ifs{ filepath };
		if (!ifs)
		{
			return false;
		}
		nlohmann::json j;
		ifs >> j;
		ifs.close();
	
		clear();
		for (auto item : j.items())
		{
			auto key = item.key();
			CommandRouter router = item.value();
			insert(std::make_pair(key, router));
		}
		return true;
	}

	bool contains(std::string router_name) const
	{
		return find(router_name) != end();
	}

	CommandRouter router(std::string router_name) const
	{
		return at(router_name);
	}

	CommandRouter router(std::string router_name, CommandRouter default_router) const
	{
		if (contains(router_name))
			return at(router_name);
		else
			return default_router;
	}
public:
	static auto& instance() {
		static CommandRouterMap ins_;
		return ins_;
	}
protected:
	CommandRouterMap() = default;
	CommandRouterMap(const CommandRouterMap&) = delete;
	CommandRouterMap(CommandRouterMap&&) = delete;
	CommandRouterMap& operator = (const CommandRouterMap&&) = delete;
	CommandRouterMap& operator = (CommandRouterMap&&) = delete;
};

class MessageWrapper
{
public:
	using SocketMessage = pb::socket::ExternalMessage;

	auto setCmdCode(int32_t cmdcode) {
		return message_.set_cmdcode(cmdcode);
	}
	auto cmdCode() const {
		return message_.cmdcode();
	}

	auto setProtocolSwitch(int32_t protocolswitch) {
		return message_.set_protocolswitch(protocolswitch);
	}
	auto protocolSwitch() const {
		return message_.protocolswitch();
	}

	auto setCmdMerge(int32_t cmdmerge) {
		return message_.set_cmdmerge(cmdmerge);
	}
	auto cmdMerge() const {
		return message_.cmdmerge();
	}

	auto setResponseStatus(int32_t responsestatus) {
		return message_.set_responsestatus(responsestatus);
	}
	auto reponseStatus() const {
		return message_.responsestatus();
	}

	auto setValidMsg(std::string validmsg) {
		return message_.set_validmsg(validmsg);
	}
	auto validMsg() const {
		return message_.validmsg();
	}

	auto setData(std::string data) {
		return message_.set_data(data);
	}
	auto data() const {
		return message_.data();
	}

	auto setData(const google::protobuf::Message& data) {
		return message_.set_data(data.SerializeAsString());
	}
	
	auto setPayload(const google::protobuf::Message& payload) {
		setRouter(payload);
		return message_.set_data(payload.SerializeAsString());
	}

	template<typename T>
	typename std::enable_if<std::is_base_of<google::protobuf::Message, T>::value, T>::type object() const
	{
		T t;
		t.ParseFromString(data());
		return t;
	}

	auto toObject(google::protobuf::Message& msg) const
	{
		return msg.ParseFromString(data());
	}

public:
	void setRouter(const google::protobuf::Message& msg)
	{
		return setRouter(msg.GetTypeName());
	}

	void setRouter(std::string routerName)
	{
		return setRouter(CommandRouterMap::instance().router(routerName, CommandRouter{ 0, 0 }));
	}
	
	void setRouter(CommandRouter router)
	{
		return setCmdMerge(router);
	}

public:

	auto serializeAsString() const
	{
		return message_.SerializeAsString();
	}

	auto parseFromString(std::string str)
	{
		return message_.ParseFromString(str);
	}

	auto static FromString(std::string str)
	{
		MessageWrapper wrapper;
		wrapper.parseFromString(str);
		return std::move(wrapper);
	}

	SocketMessage& message() {
		return message_;
	}

	const SocketMessage& message() const{
		return message_;
	}

protected:
	SocketMessage message_{};
};

