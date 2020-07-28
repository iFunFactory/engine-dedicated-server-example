// Copyright (C) 2013-2017 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT EDIT.


#include "../dedi_server_manager_object.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <funapi.h>
#include <glog/logging.h>

#include <ctime>
#include <functional>
#include <map>


namespace dedi_server_manager {

using fun::ApiService;
using fun::string;


template<typename ObjectType, typename AttributeType>
bool CompareAttribute(const Ptr<ObjectType> &object, const AttributeType &value, const function<AttributeType(const Ptr<ObjectType> &)> &attribute_getter, MatchCondition cond) {
  if (not object) {
    return false;
  }

  if (cond == kEqual) {
    return value == attribute_getter(object);
  } else if (cond == kLess) {
    return value > attribute_getter(object);
  } else if (cond == kGreater) {
    return value < attribute_getter(object);
  }
  BOOST_ASSERT(false);
  return false;
}


template<typename ObjectType, typename AttributeType>
bool CompareAttribute2(const Ptr<ObjectType> &object, const function<bool(const AttributeType &)> &match, const function<AttributeType(const Ptr<ObjectType> &)> &attribute_getter) {
  if (not object) {
    return false;
  }
  return match(attribute_getter(object));
}


template <>
bool ArrayRef<bool>::GetAt(size_t index) const {
  return owner_->GetArrayElementBoolean(attribute_name_, index);
}


template <>
int8_t ArrayRef<int8_t>::GetAt(size_t index) const {
  return owner_->GetArrayElementInteger8(attribute_name_, index);
}


template <>
int16_t ArrayRef<int16_t>::GetAt(size_t index) const {
  return owner_->GetArrayElementInteger16(attribute_name_, index);
}


template <>
int32_t ArrayRef<int32_t>::GetAt(size_t index) const {
  return owner_->GetArrayElementInteger32(attribute_name_, index);
}


template <>
int64_t ArrayRef<int64_t>::GetAt(size_t index) const {
  return owner_->GetArrayElementInteger64(attribute_name_, index);
}


template <>
float ArrayRef<float>::GetAt(size_t index) const {
  return owner_->GetArrayElementFloat(attribute_name_, index);
}


template <>
double ArrayRef<double>::GetAt(size_t index) const {
  return owner_->GetArrayElementDouble(attribute_name_, index);
}


template <>
string ArrayRef<string>::GetAt(size_t index) const {
  return owner_->GetArrayElementString(attribute_name_, index);
}


template <>
Object::Id ArrayRef<Object::Id>::GetAt(size_t index) const {
  return owner_->GetArrayElementObject(attribute_name_, index);
}


template <>
void ArrayRef<bool>::SetAt(size_t index, const bool &value) {
  owner_->SetArrayElementBoolean(attribute_name_, index, value);
}


template <>
void ArrayRef<int8_t>::SetAt(size_t index, const int8_t &value) {
  owner_->SetArrayElementInteger8(attribute_name_, index, value);
}


template <>
void ArrayRef<int16_t>::SetAt(size_t index, const int16_t &value) {
  owner_->SetArrayElementInteger16(attribute_name_, index, value);
}


template <>
void ArrayRef<int32_t>::SetAt(size_t index, const int32_t &value) {
  owner_->SetArrayElementInteger32(attribute_name_, index, value);
}


template <>
void ArrayRef<int64_t>::SetAt(size_t index, const int64_t &value) {
  owner_->SetArrayElementInteger64(attribute_name_, index, value);
}


template <>
void ArrayRef<float>::SetAt(size_t index, const float &value) {
  owner_->SetArrayElementFloat(attribute_name_, index, value);
}


template <>
void ArrayRef<double>::SetAt(size_t index, const double &value) {
  owner_->SetArrayElementDouble(attribute_name_, index, value);
}


template <>
void ArrayRef<string>::SetAt(size_t index, const string &value) {
  owner_->SetArrayElementString(attribute_name_, index, value);
}


template <>
void ArrayRef<Object::Id>::SetAt(size_t index, const Object::Id &value) {
  owner_->SetArrayElementObject(attribute_name_, index, value);
}


template <>
void ArrayRef<bool>::InsertAt(size_t index, const bool &value) {
  owner_->InsertArrayElementBoolean(attribute_name_, index, value);
}


template <>
void ArrayRef<int8_t>::InsertAt(size_t index, const int8_t &value) {
  owner_->InsertArrayElementInteger8(attribute_name_, index, value);
}


template <>
void ArrayRef<int16_t>::InsertAt(size_t index, const int16_t &value) {
  owner_->InsertArrayElementInteger16(attribute_name_, index, value);
}


template <>
void ArrayRef<int32_t>::InsertAt(size_t index, const int32_t &value) {
  owner_->InsertArrayElementInteger32(attribute_name_, index, value);
}


template <>
void ArrayRef<int64_t>::InsertAt(size_t index, const int64_t &value) {
  owner_->InsertArrayElementInteger(attribute_name_, index, value);
}


template <>
void ArrayRef<float>::InsertAt(size_t index, const float &value) {
  owner_->InsertArrayElementFloat(attribute_name_, index, value);
}


template <>
void ArrayRef<double>::InsertAt(size_t index, const double &value) {
  owner_->InsertArrayElementDouble(attribute_name_, index, value);
}


template <>
void ArrayRef<string>::InsertAt(size_t index, const string &value) {
  owner_->InsertArrayElementString(attribute_name_, index, value);
}


template <>
void ArrayRef<Object::Id>::InsertAt(size_t index, const Object::Id &value) {
  owner_->InsertArrayElementObject(attribute_name_, index, value);
}


template <>
bool ArrayRef<bool>::Front() const {
  return GetAt(0);
}


template <>
int8_t ArrayRef<int8_t>::Front() const {
  return GetAt(0);
}


template <>
int16_t ArrayRef<int16_t>::Front() const {
  return GetAt(0);
}


template <>
int32_t ArrayRef<int32_t>::Front() const {
  return GetAt(0);
}


template <>
int64_t ArrayRef<int64_t>::Front() const {
  return GetAt(0);
}


template <>
float ArrayRef<float>::Front() const {
  return GetAt(0);
}


template <>
double ArrayRef<double>::Front() const {
  return GetAt(0);
}


template <>
string ArrayRef<string>::Front() const {
  return GetAt(0);
}


template <>
Object::Id ArrayRef<Object::Id>::Front() const {
  return GetAt(0);
}


template <>
bool ArrayRef<bool>::Back() const {
  return GetAt(Size() - 1);
}


template <>
int8_t ArrayRef<int8_t>::Back() const {
  return GetAt(Size() - 1);
}


template <>
int16_t ArrayRef<int16_t>::Back() const {
  return GetAt(Size() - 1);
}


template <>
int32_t ArrayRef<int32_t>::Back() const {
  return GetAt(Size() - 1);
}


template <>
int64_t ArrayRef<int64_t>::Back() const {
  return GetAt(Size() - 1);
}


template <>
float ArrayRef<float>::Back() const {
  return GetAt(Size() - 1);
}


template <>
double ArrayRef<double>::Back() const {
  return GetAt(Size() - 1);
}


template <>
string ArrayRef<string>::Back() const {
  return GetAt(Size() - 1);
}


template <>
Object::Id ArrayRef<Object::Id>::Back() const {
  return GetAt(Size() - 1);
}


template <>
void ArrayRef<bool>::PushFront(const bool &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<int8_t>::PushFront(const int8_t &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<int16_t>::PushFront(const int16_t &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<int32_t>::PushFront(const int32_t &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<int64_t>::PushFront(const int64_t &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<float>::PushFront(const float &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<double>::PushFront(const double &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<string>::PushFront(const string &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<Object::Id>::PushFront(const Object::Id &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<bool>::PushBack(const bool &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<int8_t>::PushBack(const int8_t &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<int16_t>::PushBack(const int16_t &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<int32_t>::PushBack(const int32_t &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<int64_t>::PushBack(const int64_t &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<float>::PushBack(const float &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<double>::PushBack(const double &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<string>::PushBack(const string &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<Object::Id>::PushBack(const Object::Id &value) {
  InsertAt(Size(), value);
}


namespace {

void ConvertArrayRefToVector(const ArrayRef<Object::Id> &array, std::vector<Object::Id> *out) {
  size_t size = array.Size();
  if (size == 0) {
    return;
  }
  out->reserve(size);

  for (size_t i = 0; i < size; ++i) {
    out->push_back(array.GetAt(i));
  }
}


template <typename ObjectType>
void FetchObjectFromVector(const std::vector<Object::Id> &object_ids, bool include_null_object, LockType lock_type, std::vector<Ptr<ObjectType> > *out) {
  if (object_ids.empty()) {
    return;
  }

  if (out) {
    std::vector<std::pair<Object::Id, Ptr<ObjectType> > > object_pairs;
    ObjectType::Fetch(object_ids, &object_pairs, lock_type);
    BOOST_ASSERT(object_ids.size() == object_pairs.size());

    for (size_t i = 0; i < object_pairs.size(); ++i) {
      if (include_null_object || object_pairs[i].second) {
        out->push_back(object_pairs[i].second);
      }
    }
  } else {
    ObjectType::Fetch(object_ids, NULL, lock_type);
  }
}


template <typename ObjectType>
void FetchObjectFromArray(const ArrayRef<Object::Id> &array, bool include_null_object, LockType lock_type, std::vector<Ptr<ObjectType> > *out) {
  std::vector<Object::Id> object_ids;
  ConvertArrayRefToVector(array, &object_ids);
  FetchObjectFromVector(object_ids, include_null_object, lock_type, out);
}


template <typename KeyType>
void ConvertMapRefToVector(const MapRef<KeyType, Object::Id> &map, std::vector<Object::Id> *out) {
  std::vector<KeyType> keys = map.Keys();
  if (keys.empty()) {
    return;
  }

  out->reserve(keys.size());

  for (size_t i = 0; i < keys.size(); ++i) {
    out->push_back(map.GetAt(keys[i]));
  }
}


template <typename KeyType, typename ObjectType>
void FetchObjectFromMap(const MapRef<KeyType, Object::Id> &map, bool include_null_object, LockType lock_type, std::vector<Ptr<ObjectType> > *out) {
  std::vector<Object::Id> object_ids;
  ConvertMapRefToVector(map, &object_ids);
  FetchObjectFromVector(object_ids, include_null_object, lock_type, out);
}


}  // unnamed namespace


void ObjectModelInit() {

  Object::ObjectModelInit();
}


#ifdef ENABLE_IFUN_DEPLOY_COMPATIBILITY
namespace cs_api {

using fun::Json;
using fun::http::Request;
using fun::http::Response;

typedef fun::ApiService::MatchResult Params;


namespace {
static CsApiHandler *g_handler = NULL;
}


CsApiHandler::CsApiHandler() : schemas_(boost::assign::map_list_of.convert_to_container<boost::unordered_map<string, string> >()),
  getters_(boost::assign::map_list_of.convert_to_container<CsApiHandler::getter_map>())
{
}


CsApiHandler::~CsApiHandler() {
}


bool CsApiHandler::GetSchemaList(std::vector<std::string> *result) {
  if (!result)
    return false;
  for (schema_map::const_iterator i = schemas_.begin(), ie = schemas_.end();
        i != ie; ++i)
    result->push_back(i->first);
  return true;
}


bool CsApiHandler::ShowSchema(const string &name, string *result) {
  if (!result)
    return false;
  schema_map::const_iterator it = schemas_.find(name);
  if (it != schemas_.end())
    result->assign(it->second);
  return true;
}


bool CsApiHandler::GetAccountTypes(std::vector<std::string>*) {
  return false;
}


bool CsApiHandler::GetAccount(const string&, const string&, Json*) {
  return false;
}


bool CsApiHandler::GetAccountCash(const string&, const string&, Json*) {
  return false;
}

bool CsApiHandler::UpdateAccountCash(
    const string&, const string&, const Json&, Json*) {
  return false;
}


bool CsApiHandler::GetAccountBillingHistory(const std::string &account_type,
      const std::string &uid, int64_t from_ts, int64_t until_ts,
      fun::Json *result) {
  return false;
}


const std::string &CsApiHandler::GetBillerUrl() const {
  static std::string _url;
  return _url;
}


bool CsApiHandler::GetHistoryFromBiller(const std::string &key, int64_t from_ts,
    int64_t until_ts, fun::Json *result) const {
  std::string url = GetBillerUrl();
  if (url.empty())
    return false;

  if (*url.rend() != '/')
    url += '/';

  url += key + "/?from=" + boost::lexical_cast<std::string>(from_ts)
      + "&until=" + boost::lexical_cast<std::string>(until_ts);

  // get response from iFunEngine Biller
  Json response;
  fun::HttpClient client;
  client.set_verbose(false);
  client.Get(url);
  const fun::http::Response &res = client.response();
  if (res.status_code != 200
      || !response.FromString(res.body)
      || !response.HasAttribute("receipts")
      || !response["receipts"].IsArray()) {
    result->SetNull();
    return true;
  }

  Json &receipts = response["receipts"];
  Json &out = (*result)["billing_history"];
  out.SetArray();
  const unsigned len = receipts.Size();
  for (unsigned i = 0; i < len; ++i) {
    out.PushBack();
    Json &dst = out[i];
    Json &src = receipts[i];

    if (src.HasAttribute("product_id"))
      dst["product_id"].SetString(src["product_id"].GetString());

    if (src.HasAttribute("quantity"))
      dst["quantity"].SetInteger(src["quantity"].GetInteger());
    else
      dst["quantity"].SetInteger(1);

    if (src.HasAttribute("purchase_timestamp"))
      dst["store_timestamp"].SetInteger(
          src["purchase_timestamp"].GetInteger());
    if (src.HasAttribute("insert_timestamp"))
      dst["server_timestamp"].SetInteger(src["insert_timestamp"].GetInteger());
  }

  return true;
}


bool CsApiHandler::GetData(const string& schema_type, const string &key,
    Json *result) {
  if (!result)
    return false;
  getter_map::const_iterator func = getters_.find(schema_type);
  BOOST_ASSERT(func != getters_.end());
  if (func == getters_.end())
    return false;

  if (!func->second(key, *result))
    result->SetNull();
  return true;
}





void HandleSchemaList(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/schema/";
  std::vector<std::string> schema_list;
  if (!g_handler->GetSchemaList(&schema_list)) {
    response->status_code = fun::http::kNotImplemented;
    response->body = "not implemented";
    return;
  }

  response->status_code = fun::http::kOk;
  Json msg;
  msg["schemas"].SetArray();
  Json &schemas = msg["schemas"];
  for (std::vector<string>::const_iterator i = schema_list.begin(),
        ie = schema_list.end(); i != ie; ++i)
    schemas.PushBack(*i);
  response->body = msg.ToString();
}


void HandleSchemaGet(Ptr<Response> response, const Request &request,
    const Params &params) {
  response->status_code = fun::http::kOk;
  DLOG(INFO) << "GET /v1/schema/{" << params[1] << "}";
  if (!g_handler->ShowSchema(params[1], &(response->body))) {
    response->status_code = fun::http::kNotImplemented;
    response->body = "not implemented";
    return;
  }

  if (response->body.empty()) {
    response->status_code = fun::http::kNotFound;
    response->body = "not found";
    return;
  }
}


void HandleAccountList(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/account";
  response->status_code = fun::http::kOk;
  Json msg;
  Json &accounts = msg["accounts"];
  accounts.SetArray();
  std::vector<string> _accounts;
  if (!g_handler->GetAccountTypes(&_accounts)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  for (std::vector<string>::const_iterator i = _accounts.begin(),
        ie = _accounts.end(); i != ie; ++i)
    accounts.PushBack(*i);
  response->body = msg.ToString();
}


void HandleAccountGet(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/account/{" << params[1] << "}/{" << params[2] << "}";

  Json msg;
  Json &out = msg["account"];
  out.SetObject();
  if (!g_handler->GetAccount(params[1], params[2], &out)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }
  if (out.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  msg["account_type"].SetString(params[1]);
  msg["account_id"].SetString(params[2]);
  response->status_code = fun::http::kOk;
  response->body = msg.ToString();
  return;
}


void HandleAccountGetCash(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/account/{" << params[1] << "}/{" << params[2]
      << "}/cash";

  Json obj;
  obj.SetObject();
  if (!g_handler->GetAccountCash(params[1], params[2], &obj)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (obj.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  response->status_code = fun::http::kOk;
  response->body = obj.ToString();
  return;
}


void HandleAccountGetBillingHistory(Ptr<Response> response,
    const Request &request, const Params &params) {
  DLOG(INFO) << "GET /v1/account/{" << params[1] << "}/{" << params[2]
      << "}/billing-history";

  int64_t until_ts = std::time(NULL);
  int64_t from_ts = until_ts - (86400 * 7);

  try {
    fun::http::GetParameter::const_iterator it;
    it = request.get_parameter.find("from");
    if (it != request.get_parameter.end()) {
      DLOG(INFO) << "from_ts: " << it->second;
      from_ts = boost::lexical_cast<int64_t>(it->second);
    }

    it = request.get_parameter.find("until");
    if (it != request.get_parameter.end()) {
      DLOG(INFO) << "until_ts: " << it->second;
      until_ts = boost::lexical_cast<int64_t>(it->second);
    }
  } catch (boost::bad_lexical_cast &) {
    LOG(ERROR) << "Invalid timestamp";
    response->status_code = fun::http::kBadRequest;
    response->body = "Invalid timestamp";
    return;
  }

  Json result;
  if (!g_handler->GetAccountBillingHistory(
        params[1], params[2], from_ts, until_ts, &result)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (result.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  response->status_code = fun::http::kOk;
  response->body = result.ToString();
  return;
}


void HandleAccountUpdateCash(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "PUT /v1/account/{" << params[1] << "}/{" << params[2]
      << "}/cash";

  Json body;
  if (!body.FromString(request.body)) {
    LOG(ERROR) << "Invalid body";
    response->status_code = fun::http::kBadRequest;
    return;
  }

  Json result;
  if (!g_handler->UpdateAccountCash(params[1], params[2], body, &result)) {
    LOG(ERROR) << "Update cash handler failed (" << params[1]
        << ", " << params[2] << ')';
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (result.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  response->status_code = fun::http::kOk;
  response->body = result.ToString();
  return;
}


void HandleDataGet(Ptr<Response> response, const Request &request,
    const Params &params, const std::string &schema) {
  const string key = params["key"];
  DLOG(INFO) << "GET /v1/data/{" << schema << "}/{" << key << "}";
  Json msg;
  Json &out = msg[schema];
  out.SetObject();

  if (!g_handler->GetData(schema, key, &out)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (out.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }
  response->status_code = fun::http::kOk;
  response->body = msg.ToString();
}


bool InitializeCustomerServiceAPI(CsApiHandler *handler) {
  DLOG(INFO) << "Registering CS API handlers (v1)";
  g_handler = handler;
  BOOST_ASSERT(g_handler);

  ApiService::RegisterHandler(http::kGet,
                              boost::regex("/v1/schema/"),
                              HandleSchemaList);
  ApiService::RegisterHandler(http::kGet,
                              boost::regex("/v1/schema/(?<name>[A-Za-z_]+)"),
                              HandleSchemaGet);

  ApiService::RegisterHandler(http::kGet,
                              boost::regex("/v1/account/"),
                              HandleAccountList);
  const char *pattern = "/v1/account/(?<name>[^/]+)/(?<key>[^/]+)/";
  ApiService::RegisterHandler(http::kGet,
                              boost::regex(pattern),
                              HandleAccountGet);

  const char *pattern_cash = "/v1/account/(?<name>[^/]+)/(?<key>[^/]+)/cash";
  ApiService::RegisterHandler(http::kGet,
                              boost::regex(pattern_cash),
                              HandleAccountGetCash);
  ApiService::RegisterHandler(http::kPost,
                              boost::regex(pattern_cash),
                              HandleAccountUpdateCash);

  const char *pattern_history
      = "/v1/account/(?<name>[^/]+)/(?<key>[^/]+)/billing-history";
  ApiService::RegisterHandler(http::kGet,
                              boost::regex(pattern_history),
                              HandleAccountGetBillingHistory);

  return true;
}

}  // namespace cs_api
#endif  // ENABLE_IFUN_DEPLOY_COMPATIBILITY

};  // namespace dedi_server_manager
