// Copyright (C) 2018-2019 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

#include "matchmaking_server_wrapper.h"

#include <funapi.h>
#include <glog/logging.h>

#include <src/dsm/dedicated_server_helper.h>
#include <src/dsm/matchmaking_type.h>


namespace dsm {

namespace {

const char *kBlueTeam = "blue_team";

const char *kRedTeam = "red_team";


bool CheckPlayerRequirements(const MatchmakingServer::Player &player,
                             const MatchmakingServer::Match &match) {
  //
  // 매치 조건 검사 핸들러 함수
  //

  // 플레이어가 이 매치에 참여해도 괜찮은 지 검사합니다.
  // 다음 링크를 참고해 단계 별 매칭, 그룹 매칭 등 여러 조건에 대한 예제를
  // 확인할 수 있습니다.
  // https://ifunfactory.com/engine/documents/reference/ko/contents-support-matchmaking.html#id17

  const string &account_id = player.id;

  //
  // 메치메이킹 서버는 StartMatchmaking2 에서 넣은 데이터를 다음 형태로 가공합니다.
  // request_time, elapsed_time 값은 엔진에서 자동으로 추가하고 관리합니다.
  //
  //
  // {
  //   "request_time":6447934119, // 요청 시각
  //   "elapsed_time":0,          // 요청 이후 지난 시간(단위: 초)
  //   "user_data": {
  //     // StartMatchmaking2() 안에 넣은 user_data
  //   }
  //}
  //
  const Json &user_context = player.context;
  const Json &user_data = player.context["user_data"];

  LOG_ASSERT(user_context.HasAttribute("request_time", Json::kInteger));
  LOG_ASSERT(user_context.HasAttribute("elapsed_time", Json::kInteger));

  LOG_ASSERT(user_data.HasAttribute(kMatchLevel, Json::kInteger))
      << ": user_data=" << user_data.ToString(false);
  LOG_ASSERT(user_data.HasAttribute(kMMRScore, Json::kInteger))
      << ": user_data=" << user_data.ToString(false);

  const std::vector<MatchmakingClient::Player> &players = match.players;

  LOG(INFO) << "Checking the first condition.";

  // 조건 1. 매치 상대가 없으면 (자신만 있다면) 바로 매치에 참여합니다.
  if (players.size() == 1) {
    LOG(INFO) << "[Condition 1] A new player is going to join the match"
              << ": match_id=" << match.match_id
              << ", account_id=" << player.id
              << ", user_context=" << player.context.ToString(false);
    return true;
  }

  LOG(INFO) << "Checking the second condition.";

  // 조건 2. 매치메이킹에 참여중인 플레이어 중 1명이 30초 이상 기다린 경우 바로 넣습니다.
  auto itr = players.begin();
  auto itr_end = players.end();
  for (; itr != itr_end; ++itr) {
    if (itr->id == account_id) {
      // 조건을 검사할 때 나 자신은 제외합니다.
      continue;
    }

    const Json &user_context2 = itr->context;
    const int64_t elapsed_sec = user_context2["elapsed_time"].GetInteger();
    if (elapsed_sec >= 30) {
      LOG(INFO) << "[Condition 2] A new player is going to join the match"
                << ": match_id=" << match.match_id
                << ", account_id=" << player.id
                << ", user_context=" << player.context.ToString(false);
      return true;
    }
  }

  LOG(INFO) << "Checking the third condition.";

  // 조건 3. 평균 레벨 차가 10 이상이거나, 랭킹 점수 차가 100점 이상인 경우
  // 매치에 참여시키지 않습니다.
  const int64_t my_match_level = user_data[kMatchLevel].GetInteger();
  const int64_t my_ranking_score = user_data[kMMRScore].GetInteger();

  int64_t avg_match_level = 0, avg_ranking_score = 0;

  itr = players.begin();
  for (; itr != itr_end; ++itr) {
    if (itr->id == account_id) {
      // 조건을 검사할 때 나 자신은 제외합니다.
      continue;
    }

    const Json &user_context2 = itr->context;
    const Json &user_data2 = user_context2["user_data"];
    avg_match_level += user_data2[kMatchLevel].GetInteger();
    avg_ranking_score += user_data2[kMMRScore].GetInteger();
  }

  avg_match_level /= (players.size());
  avg_ranking_score /= (players.size());

  LOG(INFO) << "my_match_level=" << my_match_level
            << ", my_ranking_score=" << my_ranking_score
            << ", avg_match_level=" << avg_match_level
            << ", avg_ranking_score=" << avg_ranking_score;

  if (labs(my_match_level - avg_match_level) >= 10 /* 10점 이상 */ ||
      labs(my_ranking_score - avg_ranking_score) >= 100 /* 100점 이상*/) {
    return false;
  }

  // 매치 조건에 만족하는 플레이어를 찾았습니다.
  LOG(INFO) << "[Condition 3] A new player is going to join the match"
            << ": match_id=" << match.match_id
            << ", account_id=" << player.id
            << ", user_context=" << player.context.ToString(false);
  return true;
}


MatchmakingServer::MatchState CheckMatchRequirements(
    const MatchmakingServer::Match &match) {
  //
  // 매치 완료 조건 검사 핸들러 함수입니다.
  //

  size_t total_players_for_match = GetNumberOfMaxPlayers(match.type);

  // 총 플레이어 수가 매치 완료 조건에 부합하는 지 검사합니다.
  if (match.players.size() != total_players_for_match) {
    // 아직 더 많은 플레이어가 필요합니다.
    LOG(INFO) << "Waiting for more players"
              << ": match_id=" << match.match_id
              << ", match_type=" << match.type
              << ", total_players_for_match=" << total_players_for_match
              << ", current players=" << match.players.size();

    return MatchmakingServer::kMatchNeedMorePlayer;
  }

  LOG(INFO) << "Matchmaking is done"
            << ": match_id=" << match.match_id
            << ", match_type=" << match.type
            << ", total_players_for_match=" << total_players_for_match
            << ", current players=" << match.players.size();

  // 매치메이킹이 끝났으니 이 정보를 토대로 데디케이티드 서버 생성을 요청합니다.
  DedicatedServerHelper::SpawnDedicatedServer(match);
  return MatchmakingServer::kMatchComplete;
}


void OnPlayerJoined(const MatchmakingServer::Player &player,
                    MatchmakingServer::Match *match) {
  //
  // 플레이어를 매치에 포함한 후 호출하는 핸들러 함수 입니다.
  //
  // CheckPlayerRequirements() 함수 안에서 true 를 반환하면 이 함수를 호출합니다.
  //
  // 이 예제에서는 레드 / 블루로 나눠진 팀에 각각 플레이어를 넣습니다.
  // 블루 팀 인원수가 레드 팀 인원 수보다 많지 않는 한 레드 팀에 우선적으로
  // 플레이어를 넣습니다.

  // 매치를 처음 생성할 때는 매치 컨텍스트가 비어있는 상태이므로 초기화가 필요합니다.
  if (match->context.IsNull()) {
    match->context.SetObject();
    match->context[kBlueTeam].SetArray();
    match->context[kRedTeam].SetArray();
  }

  LOG(INFO) << "OnPlayerJoined: player=" << player.id
            << ", user_data=" << player.context.ToString(false)
            << ", match_data=" << match->context.ToString(false);

  if (match->context[kBlueTeam].Size() > match->context[kRedTeam].Size()) {
    match->context[kRedTeam].PushBack(player.id);
  } else {
    match->context[kBlueTeam].PushBack(player.id);
  }
}


void OnPlayerLeft(const MatchmakingServer::Player &player,
                  MatchmakingServer::Match *match) {
  LOG_ASSERT(not match->context.IsNull());
  LOG_ASSERT(match->context[kBlueTeam].IsArray());
  LOG_ASSERT(match->context[kRedTeam].IsArray());

  LOG(INFO) << "OnPlayerLeft: player=" << player.id
            << ", user_data=" << player.context.ToString(false)
            << ", match_data=" << match->context.ToString(false);

  {
    // 매치메이킹 도중 나간 플레이어가 블루 팀에 있는지 확인합니다.
    Json::ValueIterator itr = match->context[kBlueTeam].Begin();
    Json::ConstValueIterator itr_end = match->context[kBlueTeam].End();
    for (; itr != itr_end; ++itr) {
      if (itr->GetString() == player.id) {
        match->context[kBlueTeam].RemoveElement(itr);
        return;
      }
    }
  }

  {
    // 매치메이킹 도중 나간 플레이어가 레드 팀에 있는지 확인합니다.
    Json::ValueIterator itr = match->context[kRedTeam].Begin();
    Json::ConstValueIterator itr_end = match->context[kRedTeam].End();
    for (; itr != itr_end; ++itr) {
      if (itr->GetString() == player.id) {
        match->context[kRedTeam].RemoveElement(itr);
        return;
      }
    }
  }
}

}  // unnamed namespace


void MatchmakingServerWrapper::Install() {
  MatchmakingServer::Start(
      CheckPlayerRequirements, CheckMatchRequirements,
      OnPlayerJoined, OnPlayerLeft);
}

}  // namespace dsm
