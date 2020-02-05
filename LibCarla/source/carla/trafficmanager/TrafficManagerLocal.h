// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGERLOCAL__
#define __TRAFFICMANAGERLOCAL__

#include <algorithm>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

#include "carla/StringUtil.h"
#include "carla/geom/Transform.h"
#include "carla/Logging.h"
#include "carla/Memory.h"

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"

#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/EpisodeProxy.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/AtomicMap.h"
#include "carla/trafficmanager/BatchControlStage.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/MotionPlannerStage.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/TrafficLightStage.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerServer.h"

namespace carla {
namespace traffic_manager {

  using ActorPtr = carla::SharedPtr<carla::client::Actor>;

  using TLS = carla::rpc::TrafficLightState;
  using TLGroup = std::vector<carla::SharedPtr<carla::client::TrafficLight>>;

  /// The function of this class is to integrate all the various stages of
  /// the traffic manager appropriately using messengers.
  class TrafficManagerLocal : public TrafficManagerBase {

  private:

    /// PID controller parameters.
    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    std::vector<float> lateral_highway_PID_parameters;

    /// Set of all actors registered with traffic manager.
    AtomicActorSet registered_actors;

    /// Pointer to local map cache.
    std::shared_ptr<InMemoryMap> local_map;

    /// Carla's client connection object.
    carla::client::detail::EpisodeProxy episodeProxyTM;

    /// Carla's debug helper object.
    carla::client::DebugHelper debug_helper;

    /// Pointers to messenger objects connecting stage pairs.
    std::shared_ptr<CollisionToPlannerMessenger> collision_planner_messenger;
    std::shared_ptr<LocalizationToCollisionMessenger> localization_collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_traffic_light_messenger;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_planner_messenger;
    std::shared_ptr<PlannerToControlMessenger> planner_control_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_planner_messenger;

    /// Pointers to the stage objects of traffic manager.
    std::unique_ptr<CollisionStage> collision_stage;
    std::unique_ptr<BatchControlStage> control_stage;
    std::unique_ptr<LocalizationStage> localization_stage;
    std::unique_ptr<MotionPlannerStage> planner_stage;
    std::unique_ptr<TrafficLightStage> traffic_light_stage;

    /// Parameterization object.
    Parameters parameters;

    /// Method to check if traffic lights are frozen.
    bool CheckAllFrozen(TLGroup tl_to_freeze);

  protected:

    /// To start the TrafficManager.
    void Start();

    /// To stop the TrafficManager.
    void Stop();

  public:

    /// Private constructor for singleton lifecycle management.
    TrafficManagerLocal
		( std::vector<float> longitudinal_PID_parameters
		, std::vector<float> longitudinal_highway_PID_parameters
		, std::vector<float> lateral_PID_parameters
		, std::vector<float> lateral_highway_PID_parameters
		, float perc_decrease_from_limit
		, carla::client::detail::EpisodeProxy episodeProxy);

    /// Destructor.
    virtual ~TrafficManagerLocal();

    /// This method registers a vehicle with the traffic manager.
    void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

    /// This method unregisters a vehicle from traffic manager.
    void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

    /// This method kills a vehicle. (Not working right now)
    /// void DestroyVehicle(const ActorPtr &actor);

    /// Set target velocity specific to a vehicle.
    void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

    /// Set global target velocity.
    void SetGlobalPercentageSpeedDifference(float const percentage);

    /// Set collision detection rules between vehicles.
    void SetCollisionDetection(
        const ActorPtr &reference_actor,
        const ActorPtr &other_actor,
        const bool detect_collision);

    /// Method to force lane change on a vehicle.
    /// Direction flag can be set to true for left and false for right.
    void SetForceLaneChange(const ActorPtr &actor, const bool direction);

    /// Enable / disable automatic lane change on a vehicle.
    void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

    /// Method to specify how much distance a vehicle should maintain to
    /// the leading vehicle.
    void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

    /// Method to specify the % chance of ignoring collisions with other actors
    void SetPercentageIgnoreActors(const ActorPtr &actor, const float perc);

    /// Method to specify the % chance of running a red light
    void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

    /// Method to reset all traffic lights.
    void ResetAllTrafficLights();

    /// Method to switch traffic manager into synchronous execution.
    void SetSynchronousMode(bool mode);

    /// Method to provide synchronous tick
    bool SynchronousTick();

    /// Get carla episode information
    carla::client::detail::EpisodeProxy& GetEpisodeProxy();

    /// Get list of all registered vehicles
    std::vector<ActorId> GetRegisteredVehiclesIDs();
  };

} // namespace traffic_manager
} // namespace carla

#endif /* __TRAFFICMANAGERLOCAL__ */

