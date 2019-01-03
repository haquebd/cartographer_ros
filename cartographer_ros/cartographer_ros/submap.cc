/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cartographer_ros/submap.h"

#include "cartographer/common/make_unique.h"
#include "cartographer/common/port.h"
#include "cartographer/transform/transform.h"
#include "cartographer_ros/msg_conversion.h"
#include "cartographer_ros_msgs/msg/status_code.hpp"
#include "cartographer_ros_msgs/srv/submap_query.hpp"

namespace cartographer_ros {

std::unique_ptr<::cartographer::io::SubmapTextures> FetchSubmapTextures(
    const ::cartographer::mapping::SubmapId& submap_id,
    ::rclcpp::Client<::cartographer_ros_msgs::srv::SubmapQuery>::SharedPtr client){
  auto srv_request = std::make_shared<cartographer_ros_msgs::srv::SubmapQuery::Request>();
  srv_request->trajectory_id = submap_id.trajectory_id;
  srv_request->submap_index = submap_id.submap_index;

  auto result = client->async_send_request(srv_request);
  auto srv_response = result.get();

  auto response = ::cartographer::common::make_unique<::cartographer::io::SubmapTextures>();
  response->version = srv_response->submap_version;
  for (const auto& texture : srv_response->textures) {
    const std::string compressed_cells(texture.cells.begin(),
                                       texture.cells.end());
    response->textures.emplace_back(::cartographer::io::SubmapTexture{
        ::cartographer::io::UnpackTextureData(compressed_cells, texture.width,
                                              texture.height),
        texture.width, texture.height, texture.resolution,
        ToRigid3d(texture.slice_pose)});
  }
  return response;
}

}  // namespace cartographer_ros